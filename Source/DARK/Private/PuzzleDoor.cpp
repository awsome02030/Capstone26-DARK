// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleDoor.h"
#include "Kismet/GameplayStatics.h"

void APuzzleDoor::OpenDoor_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("OpenDoor_Implementation CALLED"));

    if (!GridManager)
    {
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
        if (!GridManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: GridManager not found!"));
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("APuzzleDoor: Calling ShowRoomSelectWidget"));
    GridManager->ShowRoomSelectWidget();
}
