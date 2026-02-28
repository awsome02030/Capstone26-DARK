// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDatabase.h"
#include "InventoryWidget.generated.h"

class UListView;

UCLASS()
class DARK_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RefreshInventory(const TArray<FItemData>& Inventory);

private:
	UPROPERTY(meta = (BindWidget))
	UListView* ItemList;
};
