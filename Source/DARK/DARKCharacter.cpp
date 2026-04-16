// Copyright Epic Games, Inc. All Rights Reserved.

#include "DARKCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Item.h"
#include "InventoryWidget.h"
#include "InventoryEntry.h"
#include "ItemDatabase.h"
#include "OxygenTank.h"
#include "DARKPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include <EnhancedInputSubsystems.h>
#include "DARK.h"
#include "PuzzleInteractable.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "GridManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "RoomSelectWidget.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "../../../../../../UE_5.6/Engine/Plugins/VirtualProduction/TextureShare/Source/TextureShareCore/Private/Module/TextureShareCoreLogDefines.h"

bool bUIReady = false;

ADARKCharacter::ADARKCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ADARKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent CALLED. PlayerInputComponent=%s"),
		*GetNameSafe(PlayerInputComponent));

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADARKCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADARKCharacter::DoJumpEnd);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADARKCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADARKCharacter::LookInput);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADARKCharacter::Interact);
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ADARKCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(DeviceAction, ETriggerEvent::Started, this, &ADARKCharacter::ToggleDevice);
		EnhancedInputComponent->BindAction(DebugKillAction, ETriggerEvent::Started, this, &ADARKCharacter::Die);
		EnhancedInputComponent->BindAction(GravAction, ETriggerEvent::Started, this, &ADARKCharacter::GravChange);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ADARKCharacter::TogglePauseMenu);
	}
	else
	{
		UE_LOG(LogDARK, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void ADARKCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	Health = MaxHealth;

	GridManagerRef = Cast<AGridManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass())
	);

	if (ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(Mapping, 0);
			}
		}

		InteractWidget = CreateWidget<UUserWidget>(PC, InteractWidgetClass);
		InteractWidget->AddToPlayerScreen();
		InteractWidget->SetVisibility(ESlateVisibility::Collapsed);

		InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
		InventoryWidget->AddToPlayerScreen();
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			HUDWidget->AddToPlayerScreen();
			HUDWidget->SetVisibility(ESlateVisibility::Visible);
		}



		// Added
		if (DeviceWidgetClass)
		{
			DeviceWidget = CreateWidget<UUserWidget>(PC, DeviceWidgetClass);
		}
		// End



		bUIReady = true;

		bUIReady = true;

		if (LowOxygenSound)
		{
			LowOxygenAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
			if (LowOxygenAudioComponent)
			{
				LowOxygenAudioComponent->bAutoActivate = false;
				LowOxygenAudioComponent->SetSound(LowOxygenSound);
				LowOxygenAudioComponent->SetupAttachment(RootComponent);
				LowOxygenAudioComponent->RegisterComponent();
			}
		}
	}

	SpawnAndAttachHandheld();
	if (HandheldActor)
	{
		HandheldActor->SetActorHiddenInGame(true);
	}

	GetWorld()->GetTimerManager().SetTimer(
		OxygenTimerHandle,
		this,
		&ADARKCharacter::OxygenCountdown,
		2,
		true
	);
}

void ADARKCharacter::ShowRoomSelectWidget(const TArray<FRoomData>& Choices)
{
	ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowRoomSelectWidget: No PlayerController found!"));
		return;
	}

	if (!RoomSelectWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowRoomSelectWidget: RoomSelectWidgetClass not set on character!"));
		return;
	}

	RoomSelectWidget = CreateWidget<URoomSelectWidget>(PC, RoomSelectWidgetClass);
	if (!RoomSelectWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowRoomSelectWidget: Failed to create widget!"));
		return;
	}

	RoomSelectWidget->OnRoomSelected.AddDynamic(this, &ADARKCharacter::OnRoomChosen);
	RoomSelectWidget->AddToPlayerScreen(10);
	RoomSelectWidget->SetupRoomButtons(Choices);

	PC->bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);

	UE_LOG(LogTemp, Warning, TEXT("ShowRoomSelectWidget: Widget created and added to player screen"));
}

void ADARKCharacter::OnRoomChosen(int32 ChosenIndex)
{
	if (GridManagerRef)
		GridManagerRef->OnRoomChosen(ChosenIndex);

	ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	if (RoomSelectWidget)
	{
		RoomSelectWidget->RemoveFromParent();
		RoomSelectWidget = nullptr;
	}
}

void ADARKCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ADARKCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void ADARKCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		float Sensitivity = 1.0f;
		AddControllerYawInput(Yaw * Sensitivity);
		AddControllerPitchInput(Pitch * Sensitivity);
	}
}

void ADARKCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADARKCharacter::DoJumpStart()
{
	Jump();
}

void ADARKCharacter::DoJumpEnd()
{
	StopJumping();
}

void ADARKCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUIReady)
	{
		InteractCheck();
	}

	if (bDeviceAnimating && HandheldActor)
	{
		DeviceAnimTime += DeltaTime;

		const float Alpha = FMath::Clamp(DeviceAnimTime / DeviceAnimDuration, 0.f, 1.f);

		const float NewZ = bDeviceOpening
			? FMath::Lerp(DeviceStartZ, DeviceTargetZ, Alpha)
			: FMath::Lerp(DeviceTargetZ, DeviceStartZ, Alpha);

		FVector Loc = HandheldOffset;
		Loc.Z = NewZ;
		HandheldActor->SetActorRelativeLocation(Loc);

		if (Alpha >= 1.f)
		{
			bDeviceAnimating = false;

			if (!bDeviceOpening)
			{
				HandheldActor->SetActorHiddenInGame(true);
			}
		}
	}
}

void ADARKCharacter::InteractCheck()
{


	// Modded
	if (DeviceWidget && DeviceWidget->IsVisible())
	{
		if (IsValid(InteractWidget))
		{
			InteractWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		InteractHitResult = FHitResult();
		return;
	}
	// End mod



	if (!IsValid(InteractWidget)) return;

	FHitResult Hit;
	const float TraceDistance = 250.f;

	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + FirstPersonCameraComponent->GetForwardVector() * TraceDistance;

	InteractVectorEnd = End;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params);

	if (bHit && Hit.GetActor() && (Hit.GetActor()->IsA<AItem>() || Hit.GetActor()->IsA<APuzzleInteractable>() || Hit.GetActor()->IsA<AOxygenTank>()))
	{
		InteractWidget->SetVisibility(ESlateVisibility::Visible);
		InteractHitResult = Hit;
	}
	else
	{
		InteractWidget->SetVisibility(ESlateVisibility::Collapsed);
		InteractHitResult = FHitResult();
	}
}

void ADARKCharacter::Interact()
{


	// Modded
	if (DeviceWidget && DeviceWidget->IsVisible())
	{
		return;
	}
	// End Mod



	if (AItem* Item = Cast<AItem>(InteractHitResult.GetActor()))
	{
		if (Inventory.Num() <= 5) {
			FItemData* Data = ItemDatabase->Items.FindByPredicate([&](const FItemData& ItemData) {
				return ItemData.Class == InteractHitResult.GetActor()->GetClass();
				});

			Inventory.Emplace(*Data);
			InteractHitResult.GetActor()->Destroy();
		}
	}
	else if (APuzzleInteractable* Check = Cast<APuzzleInteractable>(InteractHitResult.GetActor())) {
		for (int i = Check->requredItems.Num() - 1; i >= 0; i--) {
			FItemData& NeededItem = Check->requredItems[i];

			if (Inventory.Contains(NeededItem)) {
				RemoveItem(NeededItem);
				Check->requredItems.RemoveAt(i);
			}
		}

		if (Check->interactableType == 2) {
			Check->DecreaseNeeded();
		}
		else if (Check->requredItems.Num() == 0 && Check->completeNeeded == 0 && Check->interactableType == 1) {
			Check->OnPuzzleComplete();
		}
	}
	else if (AOxygenTank* Tank = Cast<AOxygenTank>(InteractHitResult.GetActor())) {
		Tank->Destroy();

		if (Oxygen + Tank->Oxygen > 100) {
			Oxygen = 100;
		}
		else {
			Oxygen += Tank->Oxygen;
		}

		UpdateLowOxygenAudio();
	}
}

bool ADARKCharacter::PerformInteractTrace(FHitResult& OutHit, float TraceDistance) const
{
	if (!FirstPersonCameraComponent) return false;

	FVector ViewVector = FirstPersonCameraComponent->GetComponentLocation();
	FRotator ViewRotation = FirstPersonCameraComponent->GetComponentRotation();
	FVector End = ViewVector + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), ViewVector, End, FColor::Red, false, 1.1f, 0, 1.f);

	if (OutHit.GetActor())
	{
		DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.f, 12, FColor::Green, false, 1.0f);
	}

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		ViewVector,
		End,
		ECC_GameTraceChannel1,
		Params
	);
}

void ADARKCharacter::ToggleInventory()
{
	ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController());
	if (!PC) return;


	// Modded - prevent inventory from opening when device is triggered
	if (DeviceWidget && DeviceWidget->IsVisible())
	{
		return;
	}
	// End mod



	if (!InventoryWidget->IsVisible())
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		InventoryWidget->RefreshInventory(Inventory);

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetCinematicMode(true, true, true);
		PC->bShowMouseCursor = true;

		GetCharacterMovement()->StopMovementImmediately();
		GetController()->SetIgnoreMoveInput(true);
		GetController()->SetIgnoreLookInput(true);
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

		PC->SetInputMode(FInputModeGameOnly());
		PC->SetCinematicMode(false, false, false);
		PC->bShowMouseCursor = false;

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		GetController()->SetIgnoreMoveInput(false);
		GetController()->SetIgnoreLookInput(false);

		if (ADARKPlayerController* PlayerController = Cast<ADARKPlayerController>(GetController()))
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			GetController()->SetIgnoreMoveInput(false);
			GetController()->SetIgnoreLookInput(false);
		}
	}
}

void ADARKCharacter::TogglePauseMenu()
{
	ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController());
	if (!PC) return;


	// Modded - close device when paused
	if (DeviceWidget && DeviceWidget->IsVisible())
	{
		GetWorld()->GetTimerManager().ClearTimer(DeviceWidgetDelayHandle);

		if (DeviceWidget && DeviceWidget->IsInViewport())
		{
			DeviceWidget->RemoveFromParent();
		}
		if (HandheldActor)
		{
			HandheldActor->SetActorHiddenInGame(true);
		}
	}

	bDeviceAnimating = false;
	bDeviceOpening = false;
	// End Mod


	if (!PauseMenuWidget)
	{
		if (!PauseMenuClass)
		{
			UE_LOG(LogTemp, Error, TEXT("PauseMenuClass not assigned!"));
			return;
		}

		PauseMenuWidget = CreateWidget<UUserWidget>(PC, PauseMenuClass);
		PauseMenuWidget->AddToPlayerScreen();
		PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	bIsPaused = !bIsPaused;
	PauseMenuWidget->SetVisibility(bIsPaused ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bIsPaused)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;

		GetCharacterMovement()->StopMovementImmediately();
		GetController()->SetIgnoreMoveInput(true);
		GetController()->SetIgnoreLookInput(true);
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		GetController()->SetIgnoreMoveInput(false);
		GetController()->SetIgnoreLookInput(false);
	}
}

void ADARKCharacter::RemoveItem(const FItemData& Data)
{
	Inventory.RemoveAll([&](const FItemData& Item)
		{
			return Item.ItemName == Data.ItemName;
		});

	if (InventoryWidget)
	{
		InventoryWidget->RefreshInventory(Inventory);
	}
}

void ADARKCharacter::SpawnAndAttachHandheld()
{
	if (HandheldActor) return;

	if (!HandheldClass)
	{
		UE_LOG(LogTemp, Error, TEXT("HandheldClass is NULL (check Details panel / BP class assignment)."));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	HandheldActor = GetWorld()->SpawnActor<AActor>(HandheldClass, FTransform::Identity, Params);

	if (!HandheldActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor FAILED for HandheldClass=%s"), *GetNameSafe(HandheldClass));
		return;
	}

	HandheldActor->AttachToComponent(
		FirstPersonCameraComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	HandheldActor->SetActorRelativeLocation(HandheldOffset);
	HandheldActor->SetActorRelativeRotation(HandheldRotation);
	HandheldActor->SetActorEnableCollision(false);

	FVector Loc = HandheldOffset;
	Loc.Z = DeviceStartZ;
	HandheldActor->SetActorRelativeLocation(Loc);

	UE_LOG(LogTemp, Warning, TEXT("SpawnAndAttachHandheld. Class=%s Spawned=%s"),
		*GetNameSafe(HandheldClass),
		*GetNameSafe(HandheldActor));
}

void ADARKCharacter::ToggleDevice()
{
	UE_LOG(LogTemp, Warning, TEXT("ToggleDevice CALLED. HandheldActor=%s Hidden=%d"),
		*GetNameSafe(HandheldActor),
		HandheldActor ? HandheldActor->IsHidden() : -1);

	SpawnAndAttachHandheld();

	if (!HandheldActor)
	{
		UE_LOG(LogTemp, Error, TEXT("HandheldActor is NULL after spawn attempt."));
		return;
	}


	// Modded - cannot open widget when other widgets are open?
	if (InventoryWidget && InventoryWidget->IsVisible())
	{
		return;
	}

	if (PauseMenuWidget && PauseMenuWidget->IsVisible())
	{
		return;
	}
	// End Mod



	if (bDeviceAnimating) return;

	bDeviceOpening = HandheldActor->IsHidden();
	bDeviceAnimating = true;
	DeviceAnimTime = 0.f;

	ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController());
	if (PC)
	{


		// Modded
		if (bDeviceOpening)
		{
			GetWorld()->GetTimerManager().ClearTimer(DeviceWidgetDelayHandle);

			GetWorld()->GetTimerManager().SetTimer(
				DeviceWidgetDelayHandle,
				this,
				&ADARKCharacter::ShowDeviceWidgetDelayed,
				DeviceAnimDuration,
				false
			);

			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			if (DeviceWidget)
			{
				InputMode.SetWidgetToFocus(DeviceWidget->TakeWidget());
			}

			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;

			GetController()->SetIgnoreLookInput(true);
			GetController()->SetIgnoreMoveInput(true);
			GetCharacterMovement()->StopMovementImmediately();
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(DeviceWidgetDelayHandle);

			if (DeviceWidget && DeviceWidget->IsInViewport())
			{
				DeviceWidget->RemoveFromParent();
			}

			PC->bShowMouseCursor = false;
			PC->SetInputMode(FInputModeGameOnly());

			GetController()->SetIgnoreLookInput(false);
			GetController()->SetIgnoreMoveInput(false);
		}
		// End Mod


	}

	if (bDeviceOpening)
	{
		HandheldActor->SetActorHiddenInGame(false);

		FVector Loc = HandheldOffset;
		Loc.Z = DeviceStartZ;
		HandheldActor->SetActorRelativeLocation(Loc);
	}
	else
	{
		FVector Loc = HandheldOffset;
		Loc.Z = DeviceTargetZ;
		HandheldActor->SetActorRelativeLocation(Loc);
	}
}

void ADARKCharacter::OxygenCountdown()
{
	int DrainAmount = FMath::RoundToInt(1.0f * OxygenDrainMultiplier);

	Oxygen = FMath::Clamp(Oxygen - DrainAmount, 0, 100);

	UpdateLowOxygenAudio();

	if (Oxygen == 0) {
		TakeDamagePlayer(MaxHealth);
	}
}

void ADARKCharacter::TakeDamagePlayer(float DamageAmount)
{
	Health -= DamageAmount;

	UE_LOG(LogTemp, Warning,
		TEXT("Player damage: %f, Current Health: %f"),
		DamageAmount,
		Health);

	if (Health <= 0.f)
	{
		Die();
	}
}

void ADARKCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Player Died"));

	GetCharacterMovement()->DisableMovement();
	GetController()->SetIgnoreMoveInput(true);
	GetController()->SetIgnoreLookInput(true);

	SetActorHiddenInGame(true);

	if (LowOxygenAudioComponent && LowOxygenAudioComponent->IsPlaying())
	{
		LowOxygenAudioComponent->Stop();
	}



	// Modded - CLose widget when die
	GetWorld()->GetTimerManager().ClearTimer(DeviceWidgetDelayHandle);

	if (DeviceWidget && DeviceWidget->IsInViewport())
	{
		DeviceWidget->RemoveFromParent();
	}
	if (HandheldActor)
	{
		HandheldActor->SetActorHiddenInGame(true);
	}
	// End Mod

	bDeviceAnimating = false;
	bDeviceOpening = false;

	if (ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	FTimerHandle RespawnTimer;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ADARKCharacter::Respawn,
		2.0f,
		false
	);
}

void ADARKCharacter::Respawn()
{
	Health = MaxHealth;
	Oxygen = 100;
	UpdateLowOxygenAudio();
	bIsPaused = false;

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (GridManagerRef)
		GridManagerRef->ResetGrid();

	SetActorLocation(SpawnLocation);
	SetActorHiddenInGame(false);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetController()->SetIgnoreMoveInput(false);
	GetController()->SetIgnoreLookInput(false);


	// Modded - reset when respawn
	GetWorld()->GetTimerManager().ClearTimer(DeviceWidgetDelayHandle);

	if (DeviceWidget && DeviceWidget->IsInViewport())
	{
		DeviceWidget->RemoveFromParent();
	}
	if (HandheldActor)
	{
		HandheldActor->SetActorHiddenInGame(true);
	}

	SetActorLocation(SpawnLocation);
	SetActorHiddenInGame(false);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetController()->SetIgnoreMoveInput(false);
	GetController()->SetIgnoreLookInput(false);

	if (ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetCinematicMode(false, false, false);
	}

	bDeviceAnimating = false;
	bDeviceOpening = false;
	OnPlayerRespawned();
	// ENd Mod



}

void ADARKCharacter::GravChange()
{
	UCharacterMovementComponent* move = GetCharacterMovement();

	if (move) {
		if (grav == 0) {
			move->GravityScale = 1.0;
			move->JumpZVelocity = 420.0;
			move->MaxWalkSpeed = 480.0;
			move->MaxAcceleration = 2048.0;
			move->BrakingDecelerationWalking = 2048.0;

			grav = 1;
		}
		else {
			move->GravityScale = 0.067;
			move->JumpZVelocity = 120.0;
			move->MaxWalkSpeed = 240.0;
			move->MaxAcceleration = 250.0;
			move->BrakingDecelerationWalking = 0.0;

			grav = 0;
		}
	}
}

void ADARKCharacter::UpdateLowOxygenAudio()
{
	if (!LowOxygenAudioComponent || !LowOxygenSound)
	{
		return;
	}

	bool bShouldPlay = (Oxygen <= LowOxygenThreshold && Oxygen > 0);

	if (bShouldPlay)
	{
		if (!LowOxygenAudioComponent->IsPlaying())
		{
			LowOxygenAudioComponent->Play();
		}
	}
	else
	{
		if (LowOxygenAudioComponent->IsPlaying())
		{
			LowOxygenAudioComponent->Stop();
		}
	}
}

void ADARKCharacter::ReduceOxygen(int Amount)
{
	Oxygen = FMath::Clamp(Oxygen - Amount, 0, 100);

	UpdateLowOxygenAudio();

	if (Oxygen == 0)
	{
		TakeDamagePlayer(MaxHealth);
	}
}

void ADARKCharacter::ShowDeviceWidgetDelayed()
{
	if (!DeviceWidget) return;

	if (!DeviceWidget->IsInViewport())
	{
		DeviceWidget->AddToPlayerScreen(50);
	}

	DeviceWidget->SetVisibility(ESlateVisibility::Visible);
}