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
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
    void OpenDoor();

    void OpenDoor_Implementation();

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Door")
    AGridManager* GridManager;
};