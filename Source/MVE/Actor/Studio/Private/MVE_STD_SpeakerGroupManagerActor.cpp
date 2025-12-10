// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_STD_SpeakerGroupManagerActor.h"


// Sets default values
AMVE_STD_SpeakerGroupManagerActor::AMVE_STD_SpeakerGroupManagerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMVE_STD_SpeakerGroupManagerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMVE_STD_SpeakerGroupManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

