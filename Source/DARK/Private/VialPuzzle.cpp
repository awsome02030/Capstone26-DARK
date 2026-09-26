// Fill out your copyright notice in the Description page of Project Settings.

#include "VialPuzzle.h"
#include "Item.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include <DARKCharacter.h>

AVialPuzzle::AVialPuzzle()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVialPuzzle::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABreakableVial::StaticClass(), Actors);


	for (AActor* Vial : Actors)
	{
		ABreakableVial* Vial2 = Cast<ABreakableVial>(Vial);

		if (Vial2)
		{
			Vial2->OnVialBroken.AddDynamic(
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

	if (CurrentStep >= RequiredOrder.Num() && !bSolved)
	{
		FailPuzzle();
	}

	EVialColor ExpectedColor = RequiredOrder[CurrentStep];

	if (BrokenVial->VialColor == ExpectedColor)
	{
		CurrentStep++;

			ADARKCharacter* player = Cast<ADARKCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			player->vialSolved = true;

			GEngine->AddOnScreenDebugMessage(
				-1, 
				5.0f,
				FColor::Red,
				TEXT("Correct Vial")
			);

		if (CurrentStep >= RequiredOrder.Num())
		{
			bSolved = true;

			player->vialSolved = true;

			GEngine->AddOnScreenDebugMessage(
				-1, 
				5.0f,
				FColor::Red,
				TEXT("Puzzle Passed")
			);

			SpawnReward();

			OnPuzzleSolved.Broadcast();
		}
	}
}

void AVialPuzzle::FailPuzzle()
{
	OnPuzzleFailed.Broadcast();
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