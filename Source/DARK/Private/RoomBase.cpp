#include "RoomBase.h"
#include "GridManager.h"
#include "PuzzleDoor.h"

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

    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors, true);

    for (AActor* Actor : AttachedActors)
    {
        if (APuzzleDoor* Door = Cast<APuzzleDoor>(Actor))
        {
            Door->GridManager = InGridManager;
            UE_LOG(LogTemp, Log, TEXT("Injected GridManager into door: %s"), *Door->GetName());
        }
    }
}