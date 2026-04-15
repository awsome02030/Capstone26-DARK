#include "GridManager.h"
#include "RoomBase.h"
#include "RoomSelectWidget.h"
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

    float BestDist = MAX_FLT;

    for (auto& Pair : SpawnedRooms)
    {
        if (!IsValid(Pair.Value)) continue;
        float Dist = FVector::Dist(Door->GetActorLocation(), Pair.Value->GetActorLocation());
        if (Dist < BestDist)
        {
            BestDist = Dist;
            CurrentExitRoomCell = Pair.Key;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: closest room cell X=%d Y=%d"),
        CurrentExitRoomCell.X, CurrentExitRoomCell.Y);
}

void AGridManager::ShowRoomSelectWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("ShowRoomSelectWidget CALLED"));

    if (RoomPool.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("RoomPool must have at least 3 entries, currently has %d"), RoomPool.Num());
        return;
    }

    FIntPoint NextCell = GetNextCell(CurrentExitRoomCell, CurrentExitDirection);
    UE_LOG(LogTemp, Warning, TEXT("NextCell: X=%d Y=%d"), NextCell.X, NextCell.Y);

    if (SpawnedRooms.Contains(NextCell))
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowRoomSelectWidget: NextCell already occupied, aborting"));
        return;
    }

    PendingRoomChoices.Empty();
    TArray<int32> UsedIndices;

    while (PendingRoomChoices.Num() < 3)
    {
        int32 Idx = FMath::RandRange(0, RoomPool.Num() - 1);

        if (!UsedIndices.Contains(Idx))
        {
            UsedIndices.Add(Idx);
            PendingRoomChoices.Add(RoomPool[Idx]);
        }
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

        SpawnedRooms.Add(NextCell, Room);
        SpawnHallway(CurrentExitRoomCell, NextCell, CurrentExitDirection);

        AGridManager* Self = this;
        ARoomBase* RoomRef = Room;
        GetWorldTimerManager().SetTimerForNextTick([Self, RoomRef]()
            {
                if (IsValid(RoomRef)) RoomRef->SetGridManager(Self);
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
    ClearGrid();
    SpawnAnchorRoom();

    CurrentExitDoor = nullptr;
    CurrentExitRoomCell = FIntPoint(0, 0);
}