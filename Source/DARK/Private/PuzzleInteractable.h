#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ItemDatabase.h"
#include "PuzzleDoor.h"
#include "PuzzleInteractable.generated.h"

class APuzzleInteractable;

UCLASS()
class APuzzleInteractable : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	TArray<FName> actorIDList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	FName actorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	FName roomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	TArray<FItemData> requredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	TArray<AActor*> TiedClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	int completeNeeded;

	/*
	1 = Keypad / thing that opens a door
	2 = Socket / thing that unlocks another interactable
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Puzzle")
	int interactableType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	EDoorDirection SpawnDirection;

	bool used = false;

	UFUNCTION(BlueprintCallable)
	void OnPuzzleComplete();

	UFUNCTION(BlueprintCallable)
	void DecreaseNeeded();
};