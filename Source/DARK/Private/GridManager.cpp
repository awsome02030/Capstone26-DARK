#include "GridManager.h"
#include "RoomBase.h"
#include "RoomSelectWidget.h"
#include "PuzzleDoor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "DARKCharacter.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    SpawnAnchorRoom();
}

FVector AGridManager::GridToWorld(const FIntPoint& GridPos) const
{
    return FVector(GridPos.X * GridSpacingX, GridPos.Y * GridSpacingY, 0.f);
}

FIntPoint AGridManager::GetNextCell(const FIntPoint& From, EDoorDirection Dir) const
{
    switch (Dir)
    {
    case EDoorDirection::North: return FIntPoint(From.X, From.Y + 1);
    case EDoorDirection::South: return FIntPoint(From.X, From.Y - 1);
    case EDoorDirection::East:  return FIntPoint(From.X + 1, From.Y);
    case EDoorDirection::West:  return FIntPoint(From.X - 1, From.Y);
    }
    return From;
}

FRotator AGridManager::HallwayRotationForDirection(EDoorDirection Dir) const
{
    switch (Dir)
    {
    case EDoorDirection::North: return FRotator(0, 0, 0);
    case EDoorDirection::South: return FRotator(0, 180, 0);
    case EDoorDirection::East:  return FRotator(0, 90, 0);
    case EDoorDirection::West:  return FRotator(0, 270, 0);
    }
    return FRotator::ZeroRotator;
}

EDoorDirection AGridManager::RemapDoorDirection(EDoorDirection LocalDir, EDoorDirection SpawnDir) const
{

    static const EDoorDirection Table[4][4] =
    {
         { EDoorDirection::North, EDoorDirection::South, EDoorDirection::East,  EDoorDirection::West  },
         { EDoorDirection::South, EDoorDirection::North, EDoorDirection::West,  EDoorDirection::East  },
         { EDoorDirection::East,  EDoorDirection::West,  EDoorDirection::South, EDoorDirection::North },
         { EDoorDirection::West,  EDoorDirection::East,  EDoorDirection::North, EDoorDirection::South },
    };

    auto ToIndex = [](EDoorDirection D) -> int32
        {
            switch (D)
            {
            case EDoorDirection::North: return 0;
            case EDoorDirection::South: return 1;
            case EDoorDirection::East:  return 2;
            case EDoorDirection::West:  return 3;
            }
            return 0;
        };

    return Table[ToIndex(SpawnDir)][ToIndex(LocalDir)];
}

const TArray<FRoomData>& AGridManager::GetCurrentRoomPool() const
{
    switch (ResetCount)
    {
    case 0: return GameStartPool;
    case 1: return Reset1Pool;
    case 2: return Reset2Pool;
    case 3: return Reset3Pool;
    default: return Reset3Pool;
    }
}

void AGridManager::SpawnAnchorRoom()
{
    if (!AnchorRoomBP)
    {
        UE_LOG(LogTemp, Error, TEXT("AnchorRoomBP is not set"));
        return;
    }

    ARoomBase* AnchorRoom = GetWorld()->SpawnActor<ARoomBase>(
        AnchorRoomBP,
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );

    if (AnchorRoom)
    {
        SpawnedRooms.Add(FIntPoint(0, 0), AnchorRoom);

        AGridManager* Self = this;
        ARoomBase* RoomRef = AnchorRoom;
        GetWorldTimerManager().SetTimerForNextTick([Self, RoomRef]()
            {
                if (IsValid(RoomRef)) RoomRef->SetGridManager(Self);
            });
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn AnchorRoom"));
    }
}

void AGridManager::RegisterExitDoor(AActor* Door, EDoorDirection Direction)
{
    CurrentExitDoor = Door;
    CurrentExitDirection = Direction;

    APuzzleDoor* PuzzleDoor = Cast<APuzzleDoor>(Door);

    if (PuzzleDoor && !PuzzleDoor->roomID.IsNone())
    {
        for (auto& Pair : SpawnedRooms)
        {
            if (!IsValid(Pair.Value)) continue;
            if (Pair.Value->RoomID == PuzzleDoor->roomID)
            {
                CurrentExitRoomCell = Pair.Key;
                UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: matched by roomID=%s at cell X=%d Y=%d direction=%d"),
                    *PuzzleDoor->roomID.ToString(), CurrentExitRoomCell.X, CurrentExitRoomCell.Y, (int32)Direction);
                return;
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: no roomID match, falling back to distance"));
    }

    float BestDist = MAX_FLT;
    for (auto& Pair : SpawnedRooms)
    {
        if (!IsValid(Pair.Value)) continue;

        FIntPoint PotentialNext = GetNextCell(Pair.Key, Direction);
        if (SpawnedRooms.Contains(PotentialNext)) continue;

        float Dist = FVector::Dist(Door->GetActorLocation(), Pair.Value->GetActorLocation());
        if (Dist < BestDist)
        {
            BestDist = Dist;
            CurrentExitRoomCell = Pair.Key;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: fallback matched cell X=%d Y=%d direction=%d"),
        CurrentExitRoomCell.X, CurrentExitRoomCell.Y, (int32)Direction);
}

bool AGridManager::IsCellFreeInDirection(AActor* Door, EDoorDirection Direction)
{
    APuzzleDoor* PuzzleDoor = Cast<APuzzleDoor>(Door);
    if (!PuzzleDoor) return false;

    for (auto& Pair : SpawnedRooms)
    {
        if (!IsValid(Pair.Value)) continue;
        if (Pair.Value->RoomID == PuzzleDoor->roomID)
        {
            FIntPoint NextCell = GetNextCell(Pair.Key, Direction);
            return !SpawnedRooms.Contains(NextCell);
        }
    }
    return false;
}

void AGridManager::ShowRoomSelectWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("ShowRoomSelectWidget CALLED"));

    const TArray<FRoomData>& ActivePool = GetCurrentRoomPool();

    if (ActivePool.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("Active room pool must have at least 3 entries, currently has %d"), ActivePool.Num());
        return;
    }

    FIntPoint NextCell = GetNextCell(CurrentExitRoomCell, CurrentExitDirection);

    if (SpawnedRooms.Contains(NextCell))
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCell already occupied at X=%d Y=%d, ExitCell X=%d Y=%d, Direction=%d"),
            NextCell.X, NextCell.Y, CurrentExitRoomCell.X, CurrentExitRoomCell.Y, (int32)CurrentExitDirection);
        return;
    }

    PendingRoomChoices.Empty();
    TArray<int32> UsedIndices;

    while (PendingRoomChoices.Num() < 3)
    {
        int32 Idx = FMath::RandRange(0, ActivePool.Num() - 1);
        if (!UsedIndices.Contains(Idx))
        {
            UsedIndices.Add(Idx);
            PendingRoomChoices.Add(ActivePool[Idx]);
        }
    }

    if (ResetCount >= 1)
    {
        PendingRoomChoices[FMath::RandRange(0, 2)] = HauntRoom;
    }

    if (ResetCount >= 2)
    {
        PendingRoomChoices[FMath::RandRange(0, 2)] = EscapePodRoom;
    }

    ADARKCharacter* Character = Cast<ADARKCharacter>(
        UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

    if (Character)
    {
        Character->ShowRoomSelectWidget(PendingRoomChoices);
    }
}

void AGridManager::OnRoomChosen(int32 ChosenIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("OnRoomChosen CALLED with index: %d"), ChosenIndex);

    if (!PendingRoomChoices.IsValidIndex(ChosenIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("OnRoomChosen: ChosenIndex %d is invalid! PendingRoomChoices count: %d"),
            ChosenIndex, PendingRoomChoices.Num());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnRoomChosen: Spawning room type: %s"),
        *PendingRoomChoices[ChosenIndex].RoomName);

    SpawnChosenRoom(PendingRoomChoices[ChosenIndex]);
}

void AGridManager::SpawnChosenRoom(const FRoomData& RoomData)
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnChosenRoom CALLED"));

    if (!RoomData.RoomClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnChosenRoom: RoomClass is null!"));
        return;
    }

    FIntPoint NextCell = GetNextCell(CurrentExitRoomCell, CurrentExitDirection);
    FVector SpawnLocation = GridToWorld(NextCell);

    FVector Offset = FVector::ZeroVector;

    switch (CurrentExitDirection)
    {
    case EDoorDirection::North: Offset = FVector(0, HallwayLengthOffset, 0); break;
    case EDoorDirection::South: Offset = FVector(0, -HallwayLengthOffset, 0); break;
    case EDoorDirection::East:  Offset = FVector(HallwayLengthOffset, 0, 0); break;
    case EDoorDirection::West:  Offset = FVector(-HallwayLengthOffset, 0, 0); break;
    }

    SpawnLocation += Offset;
    SpawnLocation.Z += RoomZOffset;

    FRotator SpawnRotation = HallwayRotationForDirection(CurrentExitDirection);

    UE_LOG(LogTemp, Warning, TEXT("SpawnChosenRoom: Spawning at %s rotation %s"),
        *SpawnLocation.ToString(), *SpawnRotation.ToString());

    ARoomBase* Room = GetWorld()->SpawnActor<ARoomBase>(
        RoomData.RoomClass,
        SpawnLocation,
        SpawnRotation
    );

    if (Room)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnChosenRoom: Room spawned successfully at %s"),
            *Room->GetActorLocation().ToString());

        FName NewRoomID = FName(*FString::Printf(TEXT("Room_%d_%d"), NextCell.X, NextCell.Y));
        Room->RoomID = NewRoomID;

        SpawnedRooms.Add(NextCell, Room);
        SpawnHallway(CurrentExitRoomCell, NextCell, CurrentExitDirection);

        AGridManager* Self = this;
        ARoomBase* RoomRef = Room;
        FName RoomIDCopy = NewRoomID;
        EDoorDirection SpawnDirCopy = CurrentExitDirection;
        GetWorldTimerManager().SetTimerForNextTick([Self, RoomRef, RoomIDCopy, SpawnDirCopy]()
            {
                if (IsValid(RoomRef))
                {
                    RoomRef->SetGridManager(Self);

                    TArray<AActor*> Attached;
                    RoomRef->GetAttachedActors(Attached, true);
                    for (AActor* Actor : Attached)
                    {
                        APuzzleDoor* Door = Cast<APuzzleDoor>(Actor);
                        if (Door)
                        {
                            Door->roomID = RoomIDCopy;
                            Door->DoorDirection = Self->RemapDoorDirection(Door->DoorDirection, SpawnDirCopy);
                            UE_LOG(LogTemp, Warning, TEXT("Stamped door %s roomID=%s worldDir=%d"),
                                *Door->GetName(), *RoomIDCopy.ToString(), (int32)Door->DoorDirection);
                        }
                    }
                }
            });
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnChosenRoom: SpawnActor returned null!"));
    }
}

void AGridManager::SpawnHallway(const FIntPoint& FromCell, const FIntPoint& ToCell, EDoorDirection Dir)
{
    if (HallwayPool.Num() == 0) return;

    TSubclassOf<AActor> HallwayClass = HallwayPool[0];
    if (!HallwayClass) return;

    FVector FromWorld = GridToWorld(FromCell);
    FVector ToWorld = GridToWorld(ToCell);

    FVector Mid = (FromWorld + ToWorld) * 0.5f;
    Mid.Z += RoomZOffset;
    FRotator Rot = HallwayRotationForDirection(Dir);

    AActor* Hallway = GetWorld()->SpawnActor<AActor>(HallwayClass, Mid, Rot);

    if (Hallway)
    {
        SpawnedHallways.Add(FromCell, Hallway);
    }
}

void AGridManager::ClearGrid()
{
    for (auto& Pair : SpawnedRooms)
        if (IsValid(Pair.Value))
            Pair.Value->Destroy();

    SpawnedRooms.Empty();

    for (auto& Pair : SpawnedHallways)
        if (IsValid(Pair.Value))
            Pair.Value->Destroy();

    SpawnedHallways.Empty();
}

void AGridManager::ResetGrid()
{
    ResetCount++;
    ResetCount = FMath::Clamp(ResetCount, 0, 3);

    ClearGrid();
    SpawnAnchorRoom();

    CurrentExitDoor = nullptr;
    CurrentExitRoomCell = FIntPoint(0, 0);
}