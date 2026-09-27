#include "RoomSelectWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

bool URoomSelectWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (RoomButton1) RoomButton1->OnClicked.AddDynamic(this, &URoomSelectWidget::HandleButton1Clicked);
    if (RoomButton2) RoomButton2->OnClicked.AddDynamic(this, &URoomSelectWidget::HandleButton2Clicked);
    if (RoomButton3) RoomButton3->OnClicked.AddDynamic(this, &URoomSelectWidget::HandleButton3Clicked);

    return true;
}

void URoomSelectWidget::SetSlotVisible(UButton* Button, UTextBlock* Text, bool bVisible)
{
    const ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

    if (Button) Button->SetVisibility(NewVisibility);
    if (Text) Text->SetVisibility(NewVisibility);
}

void URoomSelectWidget::SetupRoomButtons(const TArray<FRoomData>& Rooms)
{
    if (RoomText1) RoomText1->SetText(Rooms.IsValidIndex(0) ? FText::FromString(Rooms[0].RoomName) : FText::GetEmpty());
    SetSlotVisible(RoomButton1, RoomText1, Rooms.IsValidIndex(0));

    if (RoomText2) RoomText2->SetText(Rooms.IsValidIndex(1) ? FText::FromString(Rooms[1].RoomName) : FText::GetEmpty());
    SetSlotVisible(RoomButton2, RoomText2, Rooms.IsValidIndex(1));

    if (RoomText3) RoomText3->SetText(Rooms.IsValidIndex(2) ? FText::FromString(Rooms[2].RoomName) : FText::GetEmpty());
    SetSlotVisible(RoomButton3, RoomText3, Rooms.IsValidIndex(2));
}

void URoomSelectWidget::HandleButton1Clicked()
{
    OnRoomSelected.Broadcast(0);
    RemoveFromParent();
}

void URoomSelectWidget::HandleButton2Clicked()
{
    OnRoomSelected.Broadcast(1);
    RemoveFromParent();
}

void URoomSelectWidget::HandleButton3Clicked()
{
    OnRoomSelected.Broadcast(2);
    RemoveFromParent();
}