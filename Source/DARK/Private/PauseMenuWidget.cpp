// Fill out your copyright notice in the Description page of Project Settings.

#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DARKCharacter.h"

void UPauseMenuWidget::OnResumeClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADARKCharacter* Character = Cast<ADARKCharacter>(PC->GetPawn()))
        {
            Character->TogglePauseMenu();
        }
    }
}

void UPauseMenuWidget::OnRespawnClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADARKCharacter* Character = Cast<ADARKCharacter>(PC->GetPawn()))
        {
            Character->TogglePauseMenu();
            Character->Respawn();         
        }
    }
}

void UPauseMenuWidget::OnQuitClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}

