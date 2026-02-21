// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryEntry.h"
#include "ItemDatabase.h"
#include "Components/TextBlock.h"
#include "DARKCharacter.h"
#include "Components/Button.h"
#include "InventoryWidget.h"
#include "Item.h"

void UInventoryEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UItemUIObject* UItem = Cast<UItemUIObject>(ListItemObject);
	ItemName->SetText(FText::FromString(UItem->ItemData.ItemName.ToString()));

	DropButton->OnClicked.Clear();
	DropButton->OnClicked.AddDynamic(this, &UInventoryEntry::DropItem);
}

void UInventoryEntry::DropItem()
{
	ADARKCharacter* PlayerCharacter = Cast<ADARKCharacter>(GetOwningPlayer()->GetPawn());
	FItemData ItemData = GetListItem<UItemUIObject>()->ItemData;

	GetWorld()->SpawnActor<AItem>(ItemData.Class, PlayerCharacter->InteractVectorEnd, FRotator());
	PlayerCharacter->Inventory.Remove(ItemData);
	PlayerCharacter->InventoryWidget->RefreshInventory(PlayerCharacter->Inventory);
	DropButton->OnClicked.Clear();
}