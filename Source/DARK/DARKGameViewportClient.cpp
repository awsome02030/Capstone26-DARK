#include "DARKGameViewportClient.h"

#include "DARK.h"
#include "DARKPlayerController.h"
#include "Engine/Console.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealClient.h"

EMouseCursor::Type UDARKGameViewportClient::GetCursor(
    FViewport* InViewport, int32 X, int32 Y)
{
    if (!InViewport) return EMouseCursor::Default;

    if (!bLoggedFirstCursorQuery)
    {
        UE_LOG(LogDARK, Display, TEXT("DARK cursor viewport: active."));
        bLoggedFirstCursorQuery = true;
    }

    // Retain the original answer for normal gameplay, other applications,
    // the development console, and menus outside the managed cursor session.
    const EMouseCursor::Type OriginalCursor =
        Super::GetCursor(InViewport, X, Y);

    UGameInstance* Instance = GetGameInstance();
    const ADARKPlayerController* Controller = Instance
        ? Cast<ADARKPlayerController>(Instance->GetFirstLocalPlayerController())
        : nullptr;

    const bool bManagedSession = Controller &&
        Controller->IsLocalPlayerController() &&
        Controller->IsCursorSessionActive();
    const bool bApplicationActive = FSlateApplication::IsInitialized() &&
        FSlateApplication::Get().IsActive();
    const bool bConsoleActive = ViewportConsole && ViewportConsole->ConsoleActive();

    if (!bManagedSession || !bApplicationActive || bConsoleActive)
    {
        return OriginalCursor;
    }

    // UI-only input releases viewport capture and focuses a UMG widget. The
    // base viewport can then answer Default without querying the controller.
    // Always answer None during this game's managed cursor session instead.
    if (OriginalCursor != EMouseCursor::None && !bLoggedNativeCursorFallback)
    {
        UE_LOG(LogDARK, Display,
            TEXT("DARK cursor viewport: suppressed native cursor %d during managed menu (capture=%d, focus=%d)."),
            static_cast<int32>(OriginalCursor),
            InViewport->HasMouseCapture() ? 1 : 0,
            InViewport->HasFocus() ? 1 : 0);
        bLoggedNativeCursorFallback = true;
    }

    return EMouseCursor::None;
}
