#include "GridManager.h"
#include "RoomSelectWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "DARKCharacter.h"
#include "Components/SceneComponent.h"

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

void AGridManager::RegisterExitDoor(AActor* Door)
{
    CurrentExitDoor = Door;
    UE_LOG(LogTemp, Warning, TEXT("RegisterExitDoor: %s"), *GetNameSafe(Door));
}

void AGridManager::ShowRoomSelectWidget()
{
    UE_LOG(LogTemp, Log, TEXT("ShowRoomSelectWidget called"));
    if (RoomPool.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("RoomPool must have at least 3 entries"));
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
        UE_LOG(LogTemp, Error, TEXT("ShowRoomSelectWidget: No DARKCharacter found!"));
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
        UE_LOG(LogTemp, Error, TEXT("Chosen room has no RoomClass assigned"));
        return;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (CurrentExitDoor)
    {
        SpawnLocation = CurrentExitDoor->GetActorLocation() + FVector(0.f, 1000.f, 0.f);
        SpawnRotation = CurrentExitDoor->GetActorRotation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No exit door registered, spawning at origin"));
    }

    AActor* Room = GetWorld()->SpawnActor<AActor>(
        RoomData.RoomClass, SpawnLocation, SpawnRotation);

    if (Room)
    {
        SpawnedRooms.Add(FIntPoint(0, -1), Room);
        UE_LOG(LogTemp, Log, TEXT("Spawned room: %s at %s"), *RoomData.RoomName, *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn chosen room"));
    }
}

void AGridManager::ClearGrid()
{
    for (auto& Pair : SpawnedRooms)
        if (IsValid(Pair.Value))
            Pair.Value->Destroy();
    SpawnedRooms.Empty();
}

void AGridManager::ResetGrid()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting Grid"));
    ClearGrid();
    SpawnAnchorRoom();
    CurrentExitDoor = nullptr;
}