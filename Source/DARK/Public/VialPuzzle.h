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

protected:
	UPROPERTY(EditAnywhere, Category = "Puzzle|Order")
	TArray<EVialColor> RequiredOrder;

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	TArray<ABreakableVial*> Vials;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Reward")
	TSubclassOf<AItem> ItemToSpawn;

	UPROPERTY(EditAnywhere, Category = "Puzzle|Reward")
	float SpawnDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	bool bResetOnFail = true;

	UPROPERTY(EditAnywhere, Category = "Puzzle", meta = (EditCondition = "bResetOnFail"))
	float ResetDelay = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	int32 MaxFailedAttempts = 3;

	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleVialBroken(ABreakableVial* BrokenVial);

	void FailPuzzle();
	void ResetPuzzle();
	void SpawnReward();

	int32 CurrentStep = 0;
	int32 FailedAttempts = 0;
	bool bSolved = false;

	FTimerHandle ResetTimerHandle;
};