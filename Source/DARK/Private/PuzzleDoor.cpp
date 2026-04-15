#include "PuzzleDoor.h"
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

    UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: GridManager found: %s"), *GridManager->GetName());
    UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: Registering door direction: %d"), (int32)DoorDirection);

    GridManager->RegisterExitDoor(this, DoorDirection);
    UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: RegisterExitDoor complete"));
}