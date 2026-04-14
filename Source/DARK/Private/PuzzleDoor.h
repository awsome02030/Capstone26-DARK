// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GridManager.h"
#include "PuzzleDoor.generated.h"

UCLASS()
class APuzzleDoor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	FName actorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	FName roomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	EDoorDirection DoorDirection = EDoorDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Advanced")
	AGridManager* GridManager;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	void OpenDoor();

	void OpenDoor_Implementation();
};