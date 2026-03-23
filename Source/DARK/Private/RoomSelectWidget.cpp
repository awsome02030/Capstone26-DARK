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

void URoomSelectWidget::SetupRoomButtons(const TArray<FRoomData>& Rooms)
{
    if (RoomText1 && Rooms.Num() > 0) RoomText1->SetText(FText::FromString(Rooms[0].RoomName));
    if (RoomText2 && Rooms.Num() > 1) RoomText2->SetText(FText::FromString(Rooms[1].RoomName));
    if (RoomText3 && Rooms.Num() > 2) RoomText3->SetText(FText::FromString(Rooms[2].RoomName));
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