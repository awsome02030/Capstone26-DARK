#include "PasswordKeypad.h"

#include "DARKCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void APasswordKeypad::Interact(ADARKCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return;
    }

    PasswordWidget = nullptr;
    InteractingPlayer = PlayerCharacter;

    OpenPasswordWidget();
}

void APasswordKeypad::OpenPasswordWidget()
{
    if (!PasswordWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: PasswordWidgetClass is not set."));
        return;
    }

    if (!InteractingPlayer)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(
        InteractingPlayer->GetController()
    );

    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: Could not find PlayerController."));
        return;
    }

    PasswordWidget = CreateWidget<UUserWidget>(
        PC,
        PasswordWidgetClass
    );

    if (!PasswordWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: Failed to create PasswordWidget."));
        return;
    }

    PasswordWidget->AddToViewport();

    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);

    FInputModeGameAndUI InputMode;

    InputMode.SetWidgetToFocus(
        PasswordWidget->TakeWidget()
    );

    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = true;
}

void APasswordKeypad::PasswordCompleted(bool bCorrect)
{
    if (!bCorrect)
    {
        return;
    }

    ClosePasswordWidget();

    OnPuzzleComplete();
}

void APasswordKeypad::ClosePasswordWidget()
{
    if (PasswordWidget)
    {
        PasswordWidget->RemoveFromParent();
        PasswordWidget = nullptr;
    }

    if (InteractingPlayer)
    {
        APlayerController* PC = Cast<APlayerController>(
            InteractingPlayer->GetController()
        );

        if (PC)
        {
            PC->SetIgnoreMoveInput(false);
            PC->SetIgnoreLookInput(false);
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }

    InteractingPlayer = nullptr;
}