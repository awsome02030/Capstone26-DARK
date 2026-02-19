#include "GridManager.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    SpawnRooms();
}

FVector AGridManager::GridToWorld(const FIntPoint& GridPos) const
{
    return FVector(
        GridPos.X * GridSpacingX,
        GridPos.Y * GridSpacingY,
        0.f
    );
}

void AGridManager::SpawnRooms()
{
    if (!AnchorRoomBP)
    {
        UE_LOG(LogTemp, Error, TEXT("AnchorRoomBP is not set"));
        return;
    }

    if (RoomBlueprints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("RoomBlueprints array is empty"));
        return;
    }

    FIntPoint AnchorGrid(0, 0);

    AActor* AnchorRoom = GetWorld()->SpawnActor<AActor>(
        AnchorRoomBP,
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );

    if (!AnchorRoom)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn AnchorRoom"));
        return;
    }

    SpawnedRooms.Add(AnchorGrid, AnchorRoom);

    for (int32 X = -1; X <= 1; X++)
    {
        for (int32 Y = -3; Y <= -1; Y++)
        {
            FIntPoint GridPos(X, Y);

            int32 RandomIndex = FMath::RandRange(0, RoomBlueprints.Num() - 1);
            TSubclassOf<AActor> RoomBP = RoomBlueprints[RandomIndex];

            FVector WorldPos = GridToWorld(GridPos);

            AActor* Room = GetWorld()->SpawnActor<AActor>(
                RoomBP,
                WorldPos,
                FRotator::ZeroRotator
            );

            if (Room)
            {
                SpawnedRooms.Add(GridPos, Room);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn room at grid (%d,%d)"), X, Y);
            }
        }
    }
}
