#include "PuzzleInteractable.h"
#include "PuzzleDoor.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

void APuzzleInteractable::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Actors;

	if (interactableType == 1)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleDoor::StaticClass(), Actors);
	}
	else if (interactableType == 2)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleInteractable::StaticClass(), Actors);
	}

	for (FName ID : actorIDList)
	{
		for (AActor* Actor : Actors)
		{
			if (interactableType == 1)
			{
				APuzzleDoor* puzzle = Cast<APuzzleDoor>(Actor);
				if (puzzle && puzzle->roomID == roomID && puzzle->actorID == ID)
				{
					TiedClass.Add(puzzle);
				}
			}
			else if (interactableType == 2)
			{
				APuzzleInteractable* puzzle = Cast<APuzzleInteractable>(Actor);
				if (puzzle && puzzle->roomID == roomID && puzzle->actorID == ID)
				{
					TiedClass.Add(puzzle);
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

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPuzzleComplete: GridManager NOT FOUND"));
		return;
	}

	for (AActor* Actor : TiedClass)
	{
		if (!IsValid(Actor)) continue;
		APuzzleDoor* Door = Cast<APuzzleDoor>(Actor);
		if (Door)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnPuzzleComplete: Calling OpenDoor on %s"), *Door->GetName());
			Door->OpenDoor();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnPuzzleComplete: Actor %s is not a PuzzleDoor"), *Actor->GetName());
		}
	}

	used = true;
	GridManager->ShowRoomSelectWidget();
}

void APuzzleInteractable::DecreaseNeeded()
{
	for (AActor* Actor : TiedClass)
	{
		APuzzleInteractable* interactable = Cast<APuzzleInteractable>(Actor);

		if (interactable)
		{
			interactable->completeNeeded -= 1;

			used = true;
			TiedClass.Remove(interactable);
		}


	}
}