#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CursorPresentationWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UWidgetAnimation;

UENUM(BlueprintType)
enum class ECursorPresentationPhase : uint8
{
    Gameplay,
    Opening,
    Pointer,
    Recentering,
    Closing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FCursorPresentationPhaseChanged, ECursorPresentationPhase, Phase);

// Reparent the existing WBP_Crosshair to this class. Keep its images and animations.
UCLASS(Abstract)
class DARK_API UCursorPresentationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void BeginOpening();
    void BeginReturn();
    void ResetToGameplay();
    void SetInteractionBlocked(bool bBlocked);
    void SetPresentationSuppressed(bool bSuppressed);

    UFUNCTION(BlueprintCallable, Category = "Cursor")
    void SetGameplayReticleVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Cursor")
    void ApplyAnimationStartPose(UWidgetAnimation* Animation);

    UFUNCTION(BlueprintCallable, Category = "Cursor")
    void SetIntroReady(bool bReady);

    UFUNCTION(BlueprintPure, Category = "Cursor")
    ECursorPresentationPhase GetCursorPhase() const { return Phase; }

    UPROPERTY(BlueprintAssignable, Category = "Cursor")
    FCursorPresentationPhaseChanged OnPhaseChanged;

    // Called by the controller after its authoritative policy has changed.
    UFUNCTION(BlueprintImplementableEvent, Category = "Cursor")
    void OnCursorPolicyChanged();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnPreviewKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(
        const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseWheel(
        const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Cursor")
    void PrepareCursorMorph();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> CursorVisualRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> TransitionBlocker;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> MousePointerTransform;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cursor",
        meta = (ClampMin = "0.0"))
    float RecenterDuration = 0.18f;

private:
    UFUNCTION()
    void HandleTransformFinished();

    void BeginClosing();
    void PlayOpeningMorph();
    void SetPhase(ECursorPresentationPhase NewPhase);
    void ApplyVisualOffset(const FVector2D& Offset);
    void UpdateVisibility();

    ECursorPresentationPhase Phase = ECursorPresentationPhase::Gameplay;
    FVector2D VisualOffset = FVector2D::ZeroVector;
    FVector2D ReturnStartOffset = FVector2D::ZeroVector;
    double ReturnStartedAt = 0.0;
    bool bGameplayReticleVisible = true;
    bool bPresentationSuppressed = false;
    bool bInteractionBlocked = false;
    bool bResetting = false;
    bool bIntroReady = false;
    bool bWaitingForIntro = false;
};
