// Fill out your copyright notice in the Description page of Project Settings.


#include "Floating_obj.h"

// Sets default values
AFloating_obj::AFloating_obj()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Produce Static Mesh Component
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// Set Root Component
	RootComponent = Mesh;

	// Basic floating 
	Amplitude = 20.0f;
	Speed = 2.0f;

	RunningTime = 0.0f;

}

// BeginPlay
void AFloating_obj::BeginPlay()
{
	Super::BeginPlay();

	// Set Start Point
	StartLocation = GetActorLocation();
	
}

// Tick
void AFloating_obj::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// calculate floating with sin 
	float DeltaHeight = FMath::Sin(RunningTime * Speed) * Amplitude;

	FVector NewLocation = StartLocation;
	NewLocation.Z += DeltaHeight;

	SetActorLocation(NewLocation);

	// optional: add rotation
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += DeltaTime * 50.0f;
	SetActorRotation(NewRotation);
}

