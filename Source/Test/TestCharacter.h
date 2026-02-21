// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ItemDatabase.h"
#include "InventoryWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "TestCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInventoryWidget;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ATestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* Mapping;

	/** Jump Input Action */
	UPROPERTY(EditDefaultsOnly)
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditDefaultsOnly)
	class UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* InteractAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* InventoryAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* DeviceAction;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InteractWidgetClass;
	UPROPERTY()
	UUserWidget* InteractWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	UItemDatabase* ItemDatabase;

	UPROPERTY(EditDefaultsOnly, Category = "Device")
	TSubclassOf<AActor> HandheldClass;

	UPROPERTY()
	AActor* HandheldActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Device")
	FVector HandheldOffset = FVector(30.f, 12.f, -15.f);

	UPROPERTY(EditAnywhere, Category = "Device")
	FRotator HandheldRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Device")
	float DeviceAnimDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Device")
	float DeviceStartZ = -50.f;

	UPROPERTY(EditAnywhere, Category = "Device")
	float DeviceTargetZ = 0.f;

	float DeviceAnimTime = 0.f;
	bool bDeviceAnimating = false;
	bool bDeviceOpening = false;

	void SpawnAndAttachHandheld();

	void ToggleDevice();

public:
	ATestCharacter();

	virtual void Tick(float DeltaTime) override;

	FVector InteractVectorEnd;
	TArray<FItemData> Inventory;

	UPROPERTY()
	UInventoryWidget* InventoryWidget;

protected:
	void BeginPlay();

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	bool PerformInteractTrace(FHitResult& OutHit, float TraceDistance = 1000.f) const;

	void Interact();
	void InteractCheck();
	void ToggleInventory();
	void RemoveItem(const FItemData& data);

	FHitResult InteractHitResult;

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

