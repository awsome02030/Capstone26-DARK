#include "PuzzleDoor.h"
#include "RoomBase.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

void APuzzleDoor::OpenDoor_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("OpenDoor_Implementation CALLED"));
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

    TArray<AActor*> Rooms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomBase::StaticClass(), Rooms);
    for (AActor* Actor : Rooms)
    {
        ARoomBase* Room = Cast<ARoomBase>(Actor);
        if (Room && Room->RoomID == roomID)
        {
            if (Room->bDoorOpened)
            {
                UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: Room already has an open door, ignoring"));
                return;
            }
            Room->bDoorOpened = true;
            break;
        }
    }

    GridManager->RegisterExitDoor(this, DoorDirection);
}