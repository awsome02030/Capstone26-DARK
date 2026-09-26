// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableVial.h"
#include "VialPuzzle.generated.h"

class AItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleFailed);

UCLASS()
class AVialPuzzle : public AActor
{
	GENERATED_BODY()

public:
	AVialPuzzle();

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnPuzzleSolved OnPuzzleSolved;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnPuzzleFailed OnPuzzleFailed;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Order")
	TArray<EVialColor> RequiredOrder;


	UPROPERTY(BlueprintReadWrite)
	bool bSolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	TArray<ABreakableVial*> Vials;

protected:

	UPROPERTY(EditAnywhere, Category = "Puzzle|Reward")
	TSubclassOf<AItem> ItemToSpawn;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Reward")
	float SpawnDistance = 150.0f;

	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleVialBroken(ABreakableVial* BrokenVial);

	void FailPuzzle();
	void SpawnReward();

	int32 CurrentStep = 0;
};