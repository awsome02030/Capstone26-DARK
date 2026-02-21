// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleInteractable.h"

void APuzzleInteractable::OnPuzzleComplete()
{
	for (APuzzleDoor* Door : TiedClass) {
		Door->Destroy();
	}

	Destroy();
}