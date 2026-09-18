// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableVial.generated.h"

class UStaticMeshComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EVialColor : uint8
{
	None	UMETA(DisplayName = "None"),
	Red		UMETA(DisplayName = "Red"),
	Blue	UMETA(DisplayName = "Blue"),
	Yellow	UMETA(DisplayName = "Yellow")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVialBroken, ABreakableVial*, BrokenVial);

UCLASS()
class ABreakableVial : public AActor
{
	GENERATED_BODY()

public:
	ABreakableVial();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vial")
	EVialColor VialColor = EVialColor::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VialMesh;

	UPROPERTY(EditAnywhere, Category = "Vial|Visuals")
	UStaticMesh* IntactMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Vial|Visuals")
	UStaticMesh* BrokenMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Vial|Effects")
	USoundBase* BreakSound;

	UPROPERTY(BlueprintReadOnly, Category = "Vial")
	bool bIsBroken = false;

	UPROPERTY(BlueprintAssignable, Category = "Vial")
	FOnVialBroken OnVialBroken;

	UFUNCTION(BlueprintCallable, Category = "Vial")
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Vial")
	void Break();

	UFUNCTION(BlueprintCallable, Category = "Vial")
	void ResetVial();

protected:
	virtual void BeginPlay() override;
};