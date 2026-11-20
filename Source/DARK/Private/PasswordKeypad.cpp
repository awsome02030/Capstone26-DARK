#include "PasswordKeypad.h"

#include "DARKCharacter.h"
#include "DARKPlayerController.h"
#include "Blueprint/UserWidget.h"

void APasswordKeypad::Interact(ADARKCharacter* PlayerCharacter)
{
    if (!IsValid(PlayerCharacter))
    {
        return;
    }

    // Keep the reference to an already-open widget instead of spawning a copy.
    if (IsValid(PasswordWidget) && PasswordWidget->IsInViewport())
    {
        return;
    }

    // Clear a stale instance, for example after the controller removed menus
    // during a respawn. Unregistering an absent registration is harmless.
    ClosePasswordWidget();
    InteractingPlayer = PlayerCharacter;
    OpenPasswordWidget();
}

void APasswordKeypad::OpenPasswordWidget()
{
    if (!PasswordWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: PasswordWidgetClass is not set."));
        InteractingPlayer = nullptr;
        return;
    }

    if (!IsValid(InteractingPlayer))
    {
        InteractingPlayer = nullptr;
        return;
    }

    ADARKPlayerController* PC = Cast<ADARKPlayerController>(
        InteractingPlayer->GetController());

    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: Could not find DARKPlayerController."));
        InteractingPlayer = nullptr;
        return;
    }

    // Do not display an unmanaged menu if cursor presentation cannot initialize.
    if (!PC->InitializeCursorPresentation())
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: Could not initialize cursor presentation."));
        InteractingPlayer = nullptr;
        return;
    }

    PasswordWidget = CreateWidget<UUserWidget>(PC, PasswordWidgetClass);

    if (!IsValid(PasswordWidget))
    {
        UE_LOG(LogTemp, Error, TEXT("PasswordKeypad: Failed to create PasswordWidget."));
        InteractingPlayer = nullptr;
        return;
    }

    PasswordWidget->SetIsFocusable(true);
    PasswordWidget->AddToViewport();

    // Keep the world running, matching the original keypad behavior. The
    // controller owns the morph, input mode, cursor visibility and input gate.
    PC->RegisterCursorMenu(PasswordWidget, false);
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
    UUserWidget* WidgetToClose = PasswordWidget;

    // Clear actor references first so a repeated close or a Destruct callback
    // cannot try to close the same widget recursively.
    PasswordWidget = nullptr;
    InteractingPlayer = nullptr;

    if (!IsValid(WidgetToClose))
    {
        return;
    }

    // Use the widget's owner even if the character is no longer possessed.
    ADARKPlayerController* PC = Cast<ADARKPlayerController>(
        WidgetToClose->GetOwningPlayer());

    if (IsValid(PC))
    {
        PC->UnregisterCursorMenu(WidgetToClose);
    }

    WidgetToClose->RemoveFromParent();
}
