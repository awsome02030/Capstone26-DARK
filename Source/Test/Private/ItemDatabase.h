// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDatabase.generated.h"

class AItem;

USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	FName ItemName;

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<AItem> Class;

	FORCEINLINE bool operator==(const FItemData& Other) const
	{
		return ItemName == Other.ItemName;
	}
};

UCLASS()
class UItemUIObject : public UObject
{
	GENERATED_BODY()

public:
	FItemData ItemData;


};

UCLASS()
class UItemDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FItemData> Items;
	
};
