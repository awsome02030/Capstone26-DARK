// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Item.h"
#include "TestPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include <EnhancedInputSubsystems.h>
#include "Test.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "../../../../../../UE_5.6/Engine/Plugins/VirtualProduction/TextureShare/Source/TextureShareCore/Private/Module/TextureShareCoreLogDefines.h"

bool bUIReady = false;

ATestCharacter::ATestCharacter()
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

void ATestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATestCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATestCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestCharacter::LookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATestCharacter::Interact);

		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ATestCharacter::ToggleInventory);
	}
	else
	{
		UE_LOG(LogTest, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATestCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ATestPlayerController* PC = Cast<ATestPlayerController>(GetController()))
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
}

void ATestCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ATestCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ATestCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		float Sensitivity = 4.f;

		AddControllerYawInput(Yaw * Sensitivity);
		AddControllerPitchInput(Pitch);
	}
}

void ATestCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ATestCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ATestCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ATestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	 if (bUIReady)
     {
		InteractCheck();
     }
}

void ATestCharacter::InteractCheck()
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
    if (bHit && Hit.GetActor() && Hit.GetActor()->IsA<AItem>())
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

void ATestCharacter::Interact()
{
    if (AItem* Item = Cast<AItem>(InteractHitResult.GetActor()))
    {
		FItemData* Data = ItemDatabase->Items.FindByPredicate([&](const FItemData& ItemData) {
			return ItemData.Class == InteractHitResult.GetActor()->GetClass();
		});

		Inventory.Emplace(*Data);
		InteractHitResult.GetActor()->Destroy();
    }
}

bool ATestCharacter::PerformInteractTrace(FHitResult& OutHit, float TraceDistance) const
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

void ATestCharacter::ToggleInventory()
{
    ATestPlayerController* PC = Cast<ATestPlayerController>(GetController());
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

        if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
        {
            PlayerController->bShowMouseCursor = false;
            PlayerController->SetInputMode(FInputModeGameOnly());
            GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
            GetController()->SetIgnoreMoveInput(false);
            GetController()->SetIgnoreLookInput(false);
        }
    }
}