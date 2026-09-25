// Fill out your copyright notice in the Description page of Project Settings.

#include "BreakableVial.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ABreakableVial::ABreakableVial()
{
	PrimaryActorTick.bCanEverTick = false;

	VialMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VialMesh"));
	RootComponent = VialMesh;

	VialMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VialMesh->SetCollisionResponseToAllChannels(ECR_Block);
	VialMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}

void ABreakableVial::BeginPlay()
{
	Super::BeginPlay();

	ResetVial();
}

void ABreakableVial::Interact()
{
	Break();
}

void ABreakableVial::Break()
{
	if (bIsBroken)
	{
		return;
	}

	bIsBroken = true;

	if (BrokenMeshAsset && VialMesh)
	{
		VialMesh->SetStaticMesh(BrokenMeshAsset);
	}

	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BreakSound,
			GetActorLocation()
		);
	}

	OnVialBroken.Broadcast(this);
}

void ABreakableVial::ResetVial()
{
	bIsBroken = false;

	if (IntactMeshAsset && VialMesh)
	{
		VialMesh->SetStaticMesh(IntactMeshAsset);
	}
}