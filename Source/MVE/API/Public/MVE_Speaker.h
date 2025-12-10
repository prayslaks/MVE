// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_Speaker.generated.h"

class UStaticMeshComponent;
class UAudioComponent;

UCLASS()
class MVE_API AMVE_Speaker : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMVE_Speaker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Allow external classes to get the audio component
	UAudioComponent* GetAudioComponent() const { return AudioComponent; }
};
