#include "CursorPresentationWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformTime.h"
#include "Input/Reply.h"

void UCursorPresentationWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true);

    if (MousePointerTransform)
    {
        FWidgetAnimationDynamicEvent Finished;
        Finished.BindDynamic(this, &UCursorPresentationWidget::HandleTransformFinished);
        BindToAnimationFinished(MousePointerTransform, Finished);
    }
}

void UCursorPresentationWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    SetCursor(EMouseCursor::None);
    if (CursorVisualRoot)
    {
        CursorVisualRoot->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    SetInteractionBlocked(false);
    ApplyVisualOffset(FVector2D::ZeroVector);
    UpdateVisibility();
}

void UCursorPresentationWidget::BeginOpening()
{
    if (Phase != ECursorPresentationPhase::Gameplay) return;

    ApplyVisualOffset(FVector2D::ZeroVector);
    SetInteractionBlocked(true);
    SetPhase(ECursorPresentationPhase::Opening);

    // Startup retains ownership until its existing completion event releases this gate.
    bWaitingForIntro = !bIntroReady;
    if (bIntroReady) PlayOpeningMorph();
}

void UCursorPresentationWidget::SetIntroReady(bool bReady)
{
    bIntroReady = bReady;
    if (bIntroReady && bWaitingForIntro && Phase == ECursorPresentationPhase::Opening)
        PlayOpeningMorph();
}

void UCursorPresentationWidget::PlayOpeningMorph()
{
    bWaitingForIntro = false;
    PrepareCursorMorph();
    if (MousePointerTransform)
    {
        PlayAnimation(MousePointerTransform, 0.f, 1,
            EUMGSequencePlayMode::Forward, 1.f, false);
    }
    else
    {
        HandleTransformFinished();
    }
}

void UCursorPresentationWidget::BeginReturn()
{
    if (Phase != ECursorPresentationPhase::Pointer) return;

    // Interpolate the drawn visual, not the operating-system cursor.
    ReturnStartOffset = VisualOffset;
    ReturnStartedAt = FPlatformTime::Seconds();
    SetInteractionBlocked(true);
    SetPhase(ECursorPresentationPhase::Recentering);

    if (RecenterDuration <= 0.f || ReturnStartOffset.IsNearlyZero())
    {
        BeginClosing();
    }
}

void UCursorPresentationWidget::NativeTick(
    const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (Phase == ECursorPresentationPhase::Pointer)
    {
        const FVector2D Size = MyGeometry.GetLocalSize();
        if (FSlateApplication::IsInitialized() && Size.X > 0.f && Size.Y > 0.f)
        {
            const FVector2D LocalMouse = MyGeometry.AbsoluteToLocal(
                FSlateApplication::Get().GetCursorPos());
            ApplyVisualOffset(LocalMouse - Size * 0.5f);
        }
    }
    else if (Phase == ECursorPresentationPhase::Recentering)
    {
        // Real elapsed time keeps the return moving while the world is paused.
        const float Alpha = RecenterDuration > 0.f
            ? FMath::Clamp(static_cast<float>(
                (FPlatformTime::Seconds() - ReturnStartedAt) / RecenterDuration), 0.f, 1.f)
            : 1.f;
        const float Eased = Alpha * Alpha * (3.f - 2.f * Alpha);
        ApplyVisualOffset(FMath::Lerp(ReturnStartOffset, FVector2D::ZeroVector, Eased));
        if (Alpha >= 1.f) BeginClosing();
    }
}

void UCursorPresentationWidget::BeginClosing()
{
    ApplyVisualOffset(FVector2D::ZeroVector);
    SetPhase(ECursorPresentationPhase::Closing);
    if (MousePointerTransform)
    {
        PlayAnimationReverse(MousePointerTransform, 1.f, false);
    }
    else
    {
        HandleTransformFinished();
    }
}

void UCursorPresentationWidget::HandleTransformFinished()
{
    if (bResetting) return;

    if (Phase == ECursorPresentationPhase::Opening)
    {
        // Retain the final pose. The controller decides when menu input is ready.
        SetPhase(ECursorPresentationPhase::Pointer);
    }
    else if (Phase == ECursorPresentationPhase::Closing)
    {
        ApplyVisualOffset(FVector2D::ZeroVector);
        SetPhase(ECursorPresentationPhase::Gameplay);
    }
}

void UCursorPresentationWidget::ResetToGameplay()
{
    bResetting = true;
    bWaitingForIntro = false;
    Phase = ECursorPresentationPhase::Gameplay;
    ApplyAnimationStartPose(MousePointerTransform);
    ApplyVisualOffset(FVector2D::ZeroVector);
    SetInteractionBlocked(false);
    UpdateVisibility();
    bResetting = false;
}

void UCursorPresentationWidget::ApplyAnimationStartPose(UWidgetAnimation* Animation)
{
    if (!Animation) return;
    TGuardValue<bool> Guard(bResetting, true);
    StopAnimation(Animation);
    // Evaluate a paused instance at the start, rather than relying on a stopped instance.
    PlayAnimation(Animation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
    PauseAnimation(Animation);
    SetAnimationCurrentTime(Animation, 0.f);
    FlushAnimations();
}

void UCursorPresentationWidget::SetPhase(ECursorPresentationPhase NewPhase)
{
    Phase = NewPhase;
    UpdateVisibility();
    OnPhaseChanged.Broadcast(Phase);
}

void UCursorPresentationWidget::ApplyVisualOffset(const FVector2D& Offset)
{
    VisualOffset = Offset;
    if (CursorVisualRoot) CursorVisualRoot->SetRenderTranslation(Offset);
}

void UCursorPresentationWidget::SetInteractionBlocked(bool bBlocked)
{
    bInteractionBlocked = bBlocked;
    if (TransitionBlocker)
    {
        TransitionBlocker->SetVisibility(bBlocked
            ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UCursorPresentationWidget::SetGameplayReticleVisible(bool bVisible)
{
    bGameplayReticleVisible = bVisible;
    UpdateVisibility();
}

void UCursorPresentationWidget::SetPresentationSuppressed(bool bSuppressed)
{
    bPresentationSuppressed = bSuppressed;
    UpdateVisibility();
}

void UCursorPresentationWidget::UpdateVisibility()
{
    if (CursorVisualRoot)
    {
        const bool bShow = !bPresentationSuppressed &&
            (Phase != ECursorPresentationPhase::Gameplay || bGameplayReticleVisible);
        // The animation owns child opacities; policy owns only this outer opacity.
        CursorVisualRoot->SetRenderOpacity(bShow ? 1.f : 0.f);
    }
}

FReply UCursorPresentationWidget::NativeOnPreviewKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    return bInteractionBlocked ? FReply::Handled()
        : Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UCursorPresentationWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return bInteractionBlocked ? FReply::Handled()
        : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCursorPresentationWidget::NativeOnMouseButtonUp(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return bInteractionBlocked ? FReply::Handled()
        : Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UCursorPresentationWidget::NativeOnMouseWheel(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return bInteractionBlocked ? FReply::Handled()
        : Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}
