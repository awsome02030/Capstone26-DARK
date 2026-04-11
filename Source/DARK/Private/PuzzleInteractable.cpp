#include "PuzzleInteractable.h"
#include "PuzzleDoor.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

void APuzzleInteractable::OnPuzzleComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("OnPuzzleComplete CALLED. TiedClass count=%d"), TiedClass.Num());

    AGridManager* GridManager = Cast<AGridManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

    for (AActor* Actor : TiedClass)
    {
        APuzzleDoor* Door = Cast<APuzzleDoor>(Actor);
        if (Door)
        {
            if (GridManager)
            {
                GridManager->RegisterExitDoor(Door, Door->DoorDirection);
            }
            Door->OpenDoor();
        }
    }

    if (GridManager)
    {
        GridManager->ShowRoomSelectWidget();
    }

    Destroy();
}