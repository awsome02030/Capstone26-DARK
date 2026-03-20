// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleInteractable.h"
#include "PuzzleDoor.h"

void APuzzleInteractable::OnPuzzleComplete()
{
	for (AActor* Actor : TiedClass) 
	{
		
		APuzzleDoor* Door = Cast<APuzzleDoor>(Actor);

		if (Door)
		{

			Door->OpenDoor();
		}
	}

	Destroy();
}