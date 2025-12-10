// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_Speaker.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "MVE.h" // Needed for PRINTNETLOG

// Sets default values
AMVE_Speaker::AMVE_Speaker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Ensure the actor is replicated

	PRINTNETLOG(this, TEXT("Speaker 생성 완료")); // Speaker creation log

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set a default cube mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
	}

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMVE_Speaker::BeginPlay()
{
	Super::BeginPlay();
	PRINTNETLOG(this, TEXT("Speaker BeginPlay 완료")); // Speaker initialization log
}

// Called every frame
void AMVE_Speaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

