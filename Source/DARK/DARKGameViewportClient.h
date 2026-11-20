#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "DARKGameViewportClient.generated.h"

// Enforce the managed menu cursor policy at the viewport cursor-query boundary.
UCLASS(Transient, Config = Engine)
class DARK_API UDARKGameViewportClient : public UGameViewportClient
{
    GENERATED_BODY()

public:
    virtual EMouseCursor::Type GetCursor(
        FViewport* InViewport, int32 X, int32 Y) override;

private:
    // Diagnostic messages are emitted at most once per viewport instance.
    bool bLoggedFirstCursorQuery = false;
    bool bLoggedNativeCursorFallback = false;
};
