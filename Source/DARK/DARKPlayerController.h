// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CursorPresentationWidget.h"
#include "DARKPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UWidget;

USTRUCT()
struct FManagedCursorMenu
{
    GENERATED_BODY()

    UPROPERTY() TObjectPtr<UUserWidget> Menu = nullptr;
    UPROPERTY() TObjectPtr<UWidget> FocusWidget = nullptr;
    bool bPauseWorld = false;
    bool bReady = true;
    bool bWasEnabled = true;
};

UCLASS(abstract)
class DARK_API ADARKPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ADARKPlayerController();

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor")
    UCursorPresentationWidget* InitializeCursorPresentation();

    UFUNCTION(BlueprintPure, Category = "UI|Cursor")
    UCursorPresentationWidget* GetCursorPresentation() const { return CursorPresentation; }

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor",
        meta = (AdvancedDisplay = "bReady,FocusWidget"))
    void RegisterCursorMenu(UUserWidget* Menu, bool bPauseWorld = false,
        bool bReady = true, UWidget* FocusWidget = nullptr);

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor")
    void SetCursorMenuReady(UUserWidget* Menu, bool bReady);

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor")
    void UnregisterCursorMenu(UUserWidget* Menu);

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor")
    void ResetCursorSession();

    UFUNCTION(BlueprintCallable, Category = "UI|Cursor")
    void SetCursorPresentationSuppressed(bool bSuppressed);

    UFUNCTION(BlueprintPure, Category = "UI|Cursor")
    bool IsCursorSessionActive() const { return bCursorSessionActive; }

    UFUNCTION(BlueprintPure, Category = "UI|Cursor")
    bool IsGameplayCursorAllowed() const
    {
        return !bCursorSessionActive && !bCursorSuppressed;
    }

protected:
    UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
    TArray<UInputMappingContext*> DefaultMappingContexts;

    UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
    TArray<UInputMappingContext*> MobileExcludedMappingContexts;

    UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
    TSubclassOf<UUserWidget> MobileControlsWidgetClass;

    UPROPERTY() TObjectPtr<UUserWidget> MobileControlsWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY() UUserWidget* HUDWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Cursor")
    TSubclassOf<UCursorPresentationWidget> CursorPresentationClass;

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    UFUNCTION()
    void HandleCursorPhaseChanged(ECursorPresentationPhase NewPhase);

    void ReconcileCursorState();
    void ActivateTopMenu();
    void FocusTransitionShield();
    void UpdateOwnedPause();
    void NotifyCursorPolicyChanged();
    void CenterRealMouse();
    void FinishCursorSession();
    void ConfigureInvisibleCursor(UUserWidget* Menu);

    UPROPERTY(Transient) TObjectPtr<UCursorPresentationWidget> CursorPresentation;
    UPROPERTY(Transient) TArray<FManagedCursorMenu> CursorMenus;

    bool bCursorSessionActive = false;
    bool bCursorSuppressed = false;
    bool bOwnsWorldPause = false;
    bool bCenterOnActivation = false;
    bool bResettingCursor = false;
    TEnumAsByte<EMouseCursor::Type> SavedDefaultCursor = EMouseCursor::Default;
    TEnumAsByte<EMouseCursor::Type> SavedCurrentCursor = EMouseCursor::Default;
};
