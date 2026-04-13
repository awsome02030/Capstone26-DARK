#include "RoomBase.h"
#include "GridManager.h"

ARoomBase::ARoomBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARoomBase::BeginPlay()
{
    Super::BeginPlay();
    if (GridManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Room connected to GridManager"));
    }
}

void ARoomBase::SetGridManager(AGridManager* InGridManager)
{
    GridManager = InGridManager;
}