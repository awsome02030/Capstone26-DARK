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
	//FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
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

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADARKCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADARKCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADARKCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADARKCharacter::LookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADARKCharacter::Interact);

		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ADARKCharacter::ToggleInventory);

		// Toggle device
		EnhancedInputComponent->BindAction(DeviceAction, ETriggerEvent::Started, this, &ADARKCharacter::ToggleDevice);
	}
	else
	{
		UE_LOG(LogDARK, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ADARKCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ADARKPlayerController* PC = Cast<ADARKPlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			UE_LOG(LogTemp, Warning, TEXT("PC: %s, Local? %d"), *GetNameSafe(PC), PC ? PC->IsLocalController() : 0);

			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(Mapping, 0);
			}
		}

			InteractWidget = CreateWidget<UUserWidget>(PC, InteractWidgetClass);
			InteractWidget->AddToPlayerScreen();
			InteractWidget->SetVisibility(ESlateVisibility::Collapsed);

			InventoryWidget = CreateWidget < UInventoryWidget>(PC, InventoryWidgetClass);
			InventoryWidget->AddToPlayerScreen();
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);


			bUIReady = true;
	}

	SpawnAndAttachHandheld();
	if (HandheldActor)
	{
		HandheldActor->SetActorHiddenInGame(true);
	}
}

void ADARKCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ADARKCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ADARKCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		float Sensitivity = 4.f;

		AddControllerYawInput(Yaw * Sensitivity);
		AddControllerPitchInput(Pitch);
	}
}

void ADARKCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADARKCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ADARKCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
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
    if (!IsValid(InteractWidget)) return;

    FHitResult Hit;
    const float TraceDistance = 250.f;

    FVector Start = FirstPersonCameraComponent->GetComponentLocation();
    FVector End = Start + FirstPersonCameraComponent->GetForwardVector() * TraceDistance;

	InteractVectorEnd = End;

	FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params);

    // DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.f);

    // Show widget if we hit an AItem
    if (bHit && Hit.GetActor() && (Hit.GetActor()->IsA<AItem>() || Hit.GetActor()->IsA<APuzzleInteractable>()))
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

		if (Check->requredItems.Num() == 0) {
			Check->OnPuzzleComplete();
		}
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

	if (bDeviceAnimating) return;

	bDeviceOpening = HandheldActor->IsHidden();
	bDeviceAnimating = true;
	DeviceAnimTime = 0.f;

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