#include "PuzzleInteractable.h"
#include "PuzzleDoor.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

void APuzzleInteractable::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> Actors;

    if (interactableType == 1) {
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleDoor::StaticClass(), Actors);
    }
    else if (interactableType == 2) {
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleInteractable::StaticClass(), Actors);
    }

    for (FName ID : actorIDList) {
        for (AActor* Actor : Actors) {
            if (interactableType == 1) {
                APuzzleDoor* puzzle = Cast<APuzzleDoor>(Actor);
                if ((puzzle->roomID == roomID)) {
                    if (puzzle->actorID == ID) {
                        UE_LOG(LogTemp, Warning, TEXT("Item added. Count=%d"), TiedClass.Num());
                        TiedClass.Add(puzzle);
                    }
                }
            }
            else if (interactableType == 2) {
                APuzzleInteractable* puzzle = Cast<APuzzleInteractable>(Actor);
                if ((puzzle->roomID == roomID)) {
                    if (puzzle->actorID == ID) {
                        TiedClass.Add(puzzle);
                    }
                }
            }
        }
    }
}

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

void APuzzleInteractable::DecreaseNeeded()
{
    for (AActor* Actor : TiedClass)
    {
        APuzzleInteractable* interactable = Cast<APuzzleInteractable>(Actor);

        interactable->completeNeeded -= 1;
        TiedClass.Remove(Actor);
    }
}