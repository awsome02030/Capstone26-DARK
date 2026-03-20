// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ItemDatabase.h"
#include "PuzzleDoor.h"
#include "PuzzleInteractable.generated.h"

class APuzzleInteractable;

/**
 * 
 */
UCLASS()
class APuzzleInteractable : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Puzzle")
	TArray<FItemData> requredItems;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	TArray<AActor*> TiedClass;
	UFUNCTION(BlueprintCallable)
	void OnPuzzleComplete();
};
