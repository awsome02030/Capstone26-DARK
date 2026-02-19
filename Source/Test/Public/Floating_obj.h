// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floating_obj.generated.h"

UCLASS()
class TEST_API AFloating_obj : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloating_obj();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	FVector StartLocation;

	float RunningTime;

	UPROPERTY(EditAnywhere)
	float Amplitude;

	UPROPERTY(EditAnywhere)
	float Speed;
};
