#include "PuzzleDoor.h"
#include "RoomBase.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

bool APuzzleDoor::CanOpen() const
{
    if (bIsLocked) return false;

    AGridManager* GM = GridManager;
    if (!GM)
    {
        GM = Cast<AGridManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    }

    if (!GM) return false;

    return GM->IsCellFreeInDirection(const_cast<APuzzleDoor*>(this), DoorDirection);
}

void APuzzleDoor::SetDoorLocked(bool bLocked)
{
    bIsLocked = bLocked;

    UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: %s locked=%d"), *GetName(), bIsLocked);
}

void APuzzleDoor::OpenDoor_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("OpenDoor_Implementation CALLED"));

    if (bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: door is locked, so we arent opening the door"));
        return;
    }

    if (!GridManager)
    {
        GridManager = Cast<AGridManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    }
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("APuzzleDoor: GridManager not found!"));
        return;
    }

    if (!GridManager->IsCellFreeInDirection(this, DoorDirection))
    {
        UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: Next cell occupied, door will not open"));
        return;
    }

    GridManager->RegisterExitDoor(this, DoorDirection);
}