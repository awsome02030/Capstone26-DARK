// Copyright Epic Games, Inc. All Rights Reserved.
#include "DARKPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "DARKCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/ICursor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DARK.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
    // Keep the platform cursor in sync at input/focus handoffs. Merely setting
    // the controller's cursor properties does not immediately refresh Slate.
    void RefreshInvisiblePlatformCursor()
    {
        if (!FSlateApplication::IsInitialized()) return;

        FSlateApplication& Slate = FSlateApplication::Get();
        if (!Slate.IsActive()) return;

        // Refresh the current widget/viewport cursor reply, including software
        // cursor state, before applying this menu session's native cursor policy.
        Slate.QueryCursor();

        const TSharedPtr<ICursor> PlatformCursor = Slate.GetPlatformCursor();
        if (PlatformCursor.IsValid())
        {
            PlatformCursor->SetType(EMouseCursor::None);
        }
    }
}

ADARKPlayerController::ADARKPlayerController()
{
    PlayerCameraManagerClass = ADARKCameraManager::StaticClass();
}

void ADARKPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
    {
        MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
        if (MobileControlsWidget)
        {
            MobileControlsWidget->AddToPlayerScreen(0);
        }
        else
        {
            UE_LOG(LogDARK, Error, TEXT("Could not spawn mobile controls widget."));
        }
    }

    if (IsLocalPlayerController() && HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUDWidget) HUDWidget->AddToViewport();
    }

    InitializeCursorPresentation();
}

void ADARKPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (IsLocalPlayerController())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
                Subsystem->AddMappingContext(CurrentContext, 0);

            if (!SVirtualJoystick::ShouldDisplayTouchInterface())
            {
                for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
                    Subsystem->AddMappingContext(CurrentContext, 0);
            }
        }
    }
}

UCursorPresentationWidget* ADARKPlayerController::InitializeCursorPresentation()
{
    if (IsValid(CursorPresentation)) return CursorPresentation;
    if (!IsLocalPlayerController()) return nullptr;
    if (!CursorPresentationClass)
    {
        UE_LOG(LogDARK, Error,
            TEXT("Assign WBP_Crosshair to CursorPresentationClass on the player controller."));
        return nullptr;
    }

    CursorPresentation = CreateWidget<UCursorPresentationWidget>(this, CursorPresentationClass);
    if (CursorPresentation)
    {
        CursorPresentation->OnPhaseChanged.AddUniqueDynamic(
            this, &ADARKPlayerController::HandleCursorPhaseChanged);
        CursorPresentation->AddToViewport(10000);
        ConfigureInvisibleCursor(CursorPresentation);
        CursorPresentation->SetPresentationSuppressed(bCursorSuppressed);
        NotifyCursorPolicyChanged();
    }
    return CursorPresentation;
}

void ADARKPlayerController::ConfigureInvisibleCursor(UUserWidget* Menu)
{
    if (!IsValid(Menu)) return;
    Menu->SetCursor(EMouseCursor::None);
    if (Menu->WidgetTree)
    {
        Menu->WidgetTree->ForEachWidget([this](UWidget* Child)
        {
            if (!Child) return;
            Child->SetCursor(EMouseCursor::None);
            if (UUserWidget* Nested = Cast<UUserWidget>(Child))
                ConfigureInvisibleCursor(Nested);
        });
    }
}

void ADARKPlayerController::RegisterCursorMenu(
    UUserWidget* Menu, bool bPauseWorld, bool bReady, UWidget* FocusWidget)
{
    if (bResettingCursor || !IsValid(Menu) || !InitializeCursorPresentation()) return;

    // Re-registering moves an existing menu to the top without double-counting it.
    const int32 Existing = CursorMenus.IndexOfByPredicate(
        [Menu](const FManagedCursorMenu& Entry) { return Entry.Menu == Menu; });
    FManagedCursorMenu Entry;
    if (Existing != INDEX_NONE)
    {
        Entry = CursorMenus[Existing];
        CursorMenus.RemoveAt(Existing);
    }
    else
    {
        Entry.Menu = Menu;
        Entry.bWasEnabled = Menu->GetIsEnabled();
    }
    Entry.FocusWidget = FocusWidget;
    Entry.bPauseWorld = bPauseWorld;
    Entry.bReady = bReady;
    CursorMenus.Add(Entry);
    ConfigureInvisibleCursor(Menu);

    if (!bCursorSessionActive)
    {
        bCursorSessionActive = true;
        SavedDefaultCursor = DefaultMouseCursor;
        SavedCurrentCursor = CurrentMouseCursor;
        DefaultMouseCursor = EMouseCursor::None;
        CurrentMouseCursor = EMouseCursor::None;
        // Clear a cached native arrow before releasing gameplay mouse capture.
        RefreshInvisiblePlatformCursor();
        bShowMouseCursor = true;
        bCenterOnActivation = true;

        if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
            ControlledCharacter->GetCharacterMovement()->StopMovementImmediately();

        FocusTransitionShield();
        NotifyCursorPolicyChanged();
    }

    ReconcileCursorState();
}

void ADARKPlayerController::SetCursorMenuReady(UUserWidget* Menu, bool bReady)
{
    for (FManagedCursorMenu& Entry : CursorMenus)
    {
        if (Entry.Menu == Menu)
        {
            Entry.bReady = bReady;
            ReconcileCursorState();
            return;
        }
    }
}

void ADARKPlayerController::UnregisterCursorMenu(UUserWidget* Menu)
{
    if (bResettingCursor || !Menu) return;
    const int32 Index = CursorMenus.IndexOfByPredicate(
        [Menu](const FManagedCursorMenu& Entry) { return Entry.Menu == Menu; });
    if (Index == INDEX_NONE) return;

    if (IsValid(CursorMenus[Index].Menu))
        CursorMenus[Index].Menu->SetIsEnabled(CursorMenus[Index].bWasEnabled);
    CursorMenus.RemoveAt(Index);
    ReconcileCursorState();
}

void ADARKPlayerController::ReconcileCursorState()
{
    if (bResettingCursor || !CursorPresentation || !bCursorSessionActive) return;
    UpdateOwnedPause();

    const ECursorPresentationPhase Phase = CursorPresentation->GetCursorPhase();
    if (CursorMenus.IsEmpty())
    {
        if (Phase == ECursorPresentationPhase::Pointer)
        {
            FocusTransitionShield();
            CursorPresentation->BeginReturn();
        }
        else if (Phase == ECursorPresentationPhase::Gameplay)
        {
            FinishCursorSession();
        }
        // An opening/return/closing already in progress is allowed to finish.
        return;
    }

    for (FManagedCursorMenu& Entry : CursorMenus)
        if (IsValid(Entry.Menu)) Entry.Menu->SetIsEnabled(false);

    if (Phase == ECursorPresentationPhase::Gameplay)
    {
        bCenterOnActivation = true;
        FocusTransitionShield();
        CursorPresentation->BeginOpening();
    }
    else if (Phase == ECursorPresentationPhase::Pointer)
    {
        ActivateTopMenu();
    }
    else
    {
        FocusTransitionShield();
    }
}

void ADARKPlayerController::HandleCursorPhaseChanged(ECursorPresentationPhase NewPhase)
{
    if (bResettingCursor) return;
    if (NewPhase == ECursorPresentationPhase::Pointer) bCenterOnActivation = true;
    ReconcileCursorState();
}

void ADARKPlayerController::FocusTransitionShield()
{
    if (!CursorPresentation) return;
    CursorPresentation->SetInteractionBlocked(true);
    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetWidgetToFocus(CursorPresentation->TakeWidget());
    SetInputMode(Mode);
    FlushPressedKeys();
    CursorPresentation->SetKeyboardFocus();
    RefreshInvisiblePlatformCursor();
}

void ADARKPlayerController::ActivateTopMenu()
{
    if (CursorMenus.IsEmpty() || !CursorPresentation) return;
    FManagedCursorMenu& Top = CursorMenus.Last();
    if (!IsValid(Top.Menu) || !Top.bReady || !Top.Menu->IsInViewport()
        || !Top.Menu->IsVisible())
    {
        FocusTransitionShield();
        return;
    }

    Top.Menu->SetIsEnabled(Top.bWasEnabled);
    ConfigureInvisibleCursor(Top.Menu);
    CursorPresentation->SetInteractionBlocked(false);

    UWidget* Focus = IsValid(Top.FocusWidget) ? Top.FocusWidget.Get() : Top.Menu.Get();
    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetWidgetToFocus(Focus->TakeWidget());
    SetInputMode(Mode);
    Focus->SetUserFocus(this);

    if (bCenterOnActivation)
    {
        // Center after changing capture/focus, exactly when pointer following begins.
        CenterRealMouse();
        bCenterOnActivation = false;
    }
    UpdateOwnedPause();
    RefreshInvisiblePlatformCursor();
}

void ADARKPlayerController::CenterRealMouse()
{
    int32 Width = 0;
    int32 Height = 0;
    GetViewportSize(Width, Height);
    if (Width > 0 && Height > 0) SetMouseLocation(Width / 2, Height / 2);
}

void ADARKPlayerController::UpdateOwnedPause()
{
    const bool bNeedsPause = CursorMenus.ContainsByPredicate(
        [](const FManagedCursorMenu& Entry) { return Entry.bPauseWorld; });
    if (bNeedsPause && !UGameplayStatics::IsGamePaused(this))
    {
        bOwnsWorldPause = UGameplayStatics::SetGamePaused(this, true);
    }
    else if (!bNeedsPause && bOwnsWorldPause && !CursorMenus.IsEmpty()
        && CursorPresentation
        && CursorPresentation->GetCursorPhase() == ECursorPresentationPhase::Pointer)
    {
        // Returning to another menu needs no crosshair transition.
        UGameplayStatics::SetGamePaused(this, false);
        bOwnsWorldPause = false;
    }
    // If this was the last menu, hold our pause until FinishCursorSession().
}

void ADARKPlayerController::FinishCursorSession()
{
    bCursorSessionActive = false;
    bCenterOnActivation = false;
    bShowMouseCursor = false;
    DefaultMouseCursor = SavedDefaultCursor;
    CurrentMouseCursor = SavedCurrentCursor;
    CenterRealMouse();
    SetInputMode(FInputModeGameOnly());
    FlushPressedKeys();
    if (CursorPresentation) CursorPresentation->SetInteractionBlocked(false);

    if (bOwnsWorldPause)
    {
        UGameplayStatics::SetGamePaused(this, false);
        bOwnsWorldPause = false;
    }
    NotifyCursorPolicyChanged();
}

void ADARKPlayerController::NotifyCursorPolicyChanged()
{
    if (CursorPresentation) CursorPresentation->OnCursorPolicyChanged();
}

void ADARKPlayerController::SetCursorPresentationSuppressed(bool bSuppressed)
{
    bCursorSuppressed = bSuppressed;
    if (CursorPresentation) CursorPresentation->SetPresentationSuppressed(bSuppressed);
    NotifyCursorPolicyChanged();
}

void ADARKPlayerController::ResetCursorSession()
{
    if (!bCursorSessionActive)
    {
        SavedDefaultCursor = DefaultMouseCursor;
        SavedCurrentCursor = CurrentMouseCursor;
    }
    bResettingCursor = true;
    for (FManagedCursorMenu& Entry : CursorMenus)
    {
        if (IsValid(Entry.Menu))
        {
            Entry.Menu->SetIsEnabled(Entry.bWasEnabled);
            Entry.Menu->SetVisibility(ESlateVisibility::Collapsed);
            Entry.Menu->RemoveFromParent();
        }
    }
    CursorMenus.Empty();
    if (CursorPresentation) CursorPresentation->ResetToGameplay();
    bResettingCursor = false;
    FinishCursorSession();
    // Movement/look locks belonging to death or cinematics are deliberately untouched.
}
