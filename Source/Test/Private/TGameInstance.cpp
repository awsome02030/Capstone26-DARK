// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameInstance.h"

const TArray<FItemData>& UTGameInstance::GetInventory() const
{
	return Inventory;
}

bool UTGameInstance::RemoveItem(const FItemData& ItemData)
{
	return Inventory.RemoveAll([&](const FItemData& ExistingItem) {
		return ExistingItem.ItemName == ItemData.ItemName;
		}) > 0;
}