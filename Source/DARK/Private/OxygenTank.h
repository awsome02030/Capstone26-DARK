// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "OxygenTank.generated.h"

/**
 * 
 */
UCLASS()
class AOxygenTank : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	int Oxygen;
};
