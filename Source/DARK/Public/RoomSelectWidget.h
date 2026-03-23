#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GridManager.h"        
#include "RoomSelectWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSelected, int32, ChosenIndex);

UCLASS()
class DARK_API URoomSelectWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    void SetupRoomButtons(const TArray<FRoomData>& Rooms);

    UPROPERTY(BlueprintAssignable, Category = "RoomSelect")
    FOnRoomSelected OnRoomSelected;

protected:
    virtual bool Initialize() override;

    UPROPERTY(meta = (BindWidget))
    UButton* RoomButton1;
    UPROPERTY(meta = (BindWidget))
    UButton* RoomButton2;
    UPROPERTY(meta = (BindWidget))
    UButton* RoomButton3;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RoomText1;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* RoomText2;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* RoomText3;

private:
    UFUNCTION() void HandleButton1Clicked();
    UFUNCTION() void HandleButton2Clicked();
    UFUNCTION() void HandleButton3Clicked();
};