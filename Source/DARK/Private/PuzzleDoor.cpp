#include "PuzzleDoor.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

void APuzzleDoor::OpenDoor_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("OpenDoor_Implementation CALLED"));

    UE_LOG(LogTemp, Warning, TEXT("GridManager ref: %s"),
        GridManager ? *GridManager->GetName() : TEXT("NULL"));

    if (!GridManager)
    {
        GridManager = Cast<AGridManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    }

    if (!GridManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: GridManager not found!"));
        return;
    }

    GridManager->RegisterExitDoor(this, DoorDirection);
}