// Fill out your copyright notice in the Description page of Project Settings.


#include "Floating_obj.h"

AFloating_obj::AFloating_obj()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	Amplitude = 20.0f;
	Speed = 2.0f;

	RunningTime = 0.0f;

}

void AFloating_obj::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	
}

void AFloating_obj::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	float DeltaHeight = FMath::Sin(RunningTime * Speed) * Amplitude;

	FVector NewLocation = StartLocation;
	NewLocation.Z += DeltaHeight;

	SetActorLocation(NewLocation);

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += DeltaTime * 50.0f;
	SetActorRotation(NewRotation);
}

