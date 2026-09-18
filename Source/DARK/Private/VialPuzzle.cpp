// Fill out your copyright notice in the Description page of Project Settings.

#include "VialPuzzle.h"
#include "Item.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

AVialPuzzle::AVialPuzzle()
{
	PrimaryActorTick.bCanEverTick = false;

	RequiredOrder = {
		EVialColor::Red,
		EVialColor::Blue,
		EVialColor::Yellow
	};
}

void AVialPuzzle::BeginPlay()
{
	Super::BeginPlay();

	for (ABreakableVial* Vial : Vials)
	{
		if (Vial)
		{
			Vial->OnVialBroken.AddDynamic(
				this,
				&AVialPuzzle::HandleVialBroken
			);
		}
	}
}

void AVialPuzzle::HandleVialBroken(ABreakableVial* BrokenVial)
{
	if (bSolved || !BrokenVial)
	{
		return;
	}

	if (CurrentStep >= RequiredOrder.Num())
	{
		return;
	}

	EVialColor ExpectedColor = RequiredOrder[CurrentStep];

	if (BrokenVial->VialColor == ExpectedColor)
	{
		CurrentStep++;

		if (CurrentStep >= RequiredOrder.Num())
		{
			bSolved = true;

			SpawnReward();

			OnPuzzleSolved.Broadcast();
		}
	}
	else
	{
		FailPuzzle();
	}
}

void AVialPuzzle::FailPuzzle()
{
	FailedAttempts++;

	OnPuzzleFailed.Broadcast();

	if (bResetOnFail && FailedAttempts <= MaxFailedAttempts)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ResetTimerHandle,
			this,
			&AVialPuzzle::ResetPuzzle,
			ResetDelay,
			false
		);
	}
}

void AVialPuzzle::ResetPuzzle()
{
	CurrentStep = 0;

	for (ABreakableVial* Vial : Vials)
	{
		if (Vial)
		{
			Vial->ResetVial();
		}
	}
}

void AVialPuzzle::SpawnReward()
{
	if (!ItemToSpawn)
	{
		return;
	}

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (!Character)
	{
		return;
	}

	FVector SpawnLocation =
		Character->GetActorLocation() +
		Character->GetActorForwardVector() * SpawnDistance;

	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;

	GetWorld()->SpawnActor<AItem>(
		ItemToSpawn,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}