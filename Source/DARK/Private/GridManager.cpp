#include "GridManager.h"
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
    AActor* AnchorRoom = GetWorld()->SpawnActor<AActor>(
        AnchorRoomBP, FVector::ZeroVector, FRotator::ZeroRotator);
    if (AnchorRoom)
        SpawnedRooms.Add(FIntPoint(0, 0), AnchorRoom);
    else
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn AnchorRoom"));
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
    UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: %s | Dir: %d | Cell: %d,%d"),
        *GetNameSafe(Door), (int32)Direction,
        CurrentExitRoomCell.X, CurrentExitRoomCell.Y);
}

void AGridManager::ShowRoomSelectWidget()
{
    if (RoomPool.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("RoomPool must have at least 3 entries"));
        return;
    }

    FIntPoint NextCell = GetNextCell(CurrentExitRoomCell, CurrentExitDirection);

    if (SpawnedRooms.Contains(NextCell))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell %d,%d already occupied — skipping widget"),
            NextCell.X, NextCell.Y);
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
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("No DARKCharacter found"));
        return;
    }
    Character->ShowRoomSelectWidget(PendingRoomChoices);
}

void AGridManager::OnRoomChosen(int32 ChosenIndex)
{
    if (!PendingRoomChoices.IsValidIndex(ChosenIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ChosenIndex: %d"), ChosenIndex);
        return;
    }
    SpawnChosenRoom(PendingRoomChoices[ChosenIndex]);
}

void AGridManager::SpawnChosenRoom(const FRoomData& RoomData)
{
    if (!RoomData.RoomClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Chosen room has no RoomClass"));
        return;
    }

    FIntPoint NextCell = GetNextCell(CurrentExitRoomCell, CurrentExitDirection);
    FVector SpawnLocation = GridToWorld(NextCell);

    FVector DirectionVector = FVector::ZeroVector;
    switch (CurrentExitDirection)
    {
    case EDoorDirection::North: DirectionVector = FVector(0, HallwayLengthOffset, 0); break;
    case EDoorDirection::South: DirectionVector = FVector(0, -HallwayLengthOffset, 0); break;
    case EDoorDirection::East:  DirectionVector = FVector(HallwayLengthOffset, 0, 0); break;
    case EDoorDirection::West:  DirectionVector = FVector(-HallwayLengthOffset, 0, 0); break;
    }
    SpawnLocation += DirectionVector;

    FRotator SpawnRotation = HallwayRotationForDirection(CurrentExitDirection);

    AActor* Room = GetWorld()->SpawnActor<AActor>(RoomData.RoomClass, SpawnLocation, SpawnRotation);
    if (Room)
    {
        SpawnedRooms.Add(NextCell, Room);
        UE_LOG(LogTemp, Log, TEXT("Spawned '%s' at cell %d,%d (%s)"),
            *RoomData.RoomName, NextCell.X, NextCell.Y, *SpawnLocation.ToString());
        SpawnHallway(CurrentExitRoomCell, NextCell, CurrentExitDirection);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn chosen room"));
    }
}

void AGridManager::SpawnHallway(const FIntPoint& FromCell, const FIntPoint& ToCell, EDoorDirection Dir)
{
    if (HallwayPool.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hallways in HallwayPool — skipping"));
        return;
    }

    TSubclassOf<AActor> HallwayClass = HallwayPool[0];
    if (!HallwayClass) return;

    FVector FromWorld = GridToWorld(FromCell);
    FVector ToWorld = GridToWorld(ToCell);
    FVector MidPoint = (FromWorld + ToWorld) * 0.5f;
    FRotator HallwayRot = HallwayRotationForDirection(Dir);

    AActor* Hallway = GetWorld()->SpawnActor<AActor>(HallwayClass, MidPoint, HallwayRot);
    if (Hallway)
    {
        SpawnedHallways.Add(FromCell, Hallway);
        UE_LOG(LogTemp, Log, TEXT("Spawned hallway between %d,%d and %d,%d"),
            FromCell.X, FromCell.Y, ToCell.X, ToCell.Y);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn hallway"));
    }
}

void AGridManager::ClearGrid()
{
    for (auto& Pair : SpawnedRooms)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    SpawnedRooms.Empty();

    for (auto& Pair : SpawnedHallways)
        if (IsValid(Pair.Value)) Pair.Value->Destroy();
    SpawnedHallways.Empty();
}

void AGridManager::ResetGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting Grid"));
    ClearGrid();
    SpawnAnchorRoom();
    CurrentExitDoor = nullptr;
    CurrentExitRoomCell = FIntPoint(0, 0);
}