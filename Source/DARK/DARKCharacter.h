// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ItemDatabase.h"
#include "InventoryWidget.h"
#include "RoomSelectWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "GridManager.h"
#include "DARKCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInventoryWidget;
struct FInputActionValue;
class USoundBase;
class UAudioComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ADARKCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
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

	// Testing the Kill with "b"
	UPROPERTY(EditDefaultsOnly)
	UInputAction* DebugKillAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* GravAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* PauseAction;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InteractWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> DeviceWidgetClass;

	UPROPERTY()
	UUserWidget* DeviceWidget = nullptr;

	UPROPERTY()
	UUserWidget* HUDWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URoomSelectWidget> RoomSelectWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	UItemDatabase* ItemDatabase;

	UPROPERTY(EditDefaultsOnly, Category = "Device")
	TSubclassOf<AActor> HandheldClass;

	UPROPERTY()
	AActor* HandheldActor = nullptr;

	UPROPERTY()
	UUserWidget* InteractWidget = nullptr;

	UPROPERTY()
	UUserWidget* PauseMenuWidget = nullptr;

	UPROPERTY()
	URoomSelectWidget* RoomSelectWidget = nullptr;

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

	FTimerHandle OxygenTimerHandle;

	float DeviceAnimTime = 0.f;
	bool bDeviceAnimating = false;
	bool bDeviceOpening = false;
	bool bIsPaused = false;

	FTimerHandle DeviceWidgetDelayHandle;

	void ShowDeviceWidgetDelayed();

	void SpawnAndAttachHandheld();
	void ToggleDevice();

	//added audio for low oxygen sound like alarm
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* LowOxygenSound;

	UPROPERTY()
	UAudioComponent* LowOxygenAudioComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float LowOxygenThreshold = 20.f;



public:
	ADARKCharacter();

	virtual void Tick(float DeltaTime) override;

	FVector InteractVectorEnd;
	TArray<FItemData> Inventory;

	UPROPERTY()
	UInventoryWidget* InventoryWidget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int Oxygen = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float Health;

	void TakeDamagePlayer(float DamageAmount);

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerRespawned();

	void Respawn();
	void TogglePauseMenu();
	void GravChange();

	void ShowRoomSelectWidget(const TArray<FRoomData>& Choices);

	UFUNCTION()
	void OnRoomChosen(int32 ChosenIndex);

	FVector SpawnLocation;
	AGridManager* GridManagerRef = nullptr;

	void UpdateLowOxygenAudio();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ReduceOxygen(int Amount);

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	bool bIsInDeathZone = false;

	UPROPERTY(BlueprintReadWrite)
	float OxygenDrainMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	bool HasAudioTape = false;

protected:
	void BeginPlay();

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	bool PerformInteractTrace(FHitResult& OutHit, float TraceDistance = 1000.f) const;

	void Interact();
	void InteractCheck();
	void ToggleInventory();
	void RemoveItem(const FItemData& data);
	void OxygenCountdown();

	FHitResult InteractHitResult;
	int grav = 0;

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};