// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ItemDatabase.h"
#include "TGameInstance.generated.h"

UCLASS()
class DARK_API UTGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TArray<FItemData> Inventory;

public:
	UFUNCTION()
	const TArray<FItemData>& GetInventory() const;

	UFUNCTION()
	bool RemoveItem(const FItemData& ItemData);
};
