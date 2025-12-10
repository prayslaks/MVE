// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_API_PCTest_StdComponent.generated.h"

class UglTFRuntimeAsset;
class AMVE_Speaker;
class USoundWave;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_API_PCTest_StdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_API_PCTest_StdComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<TObjectPtr<AMVE_Speaker>> FoundSpeakers;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Audio Sync RPCs ---

	UFUNCTION(Client, Reliable)
	void Client_PrepareAudio(const FString& PresignedUrl);

	UFUNCTION(Client, Reliable)
	void Client_PlayPreparedAudio();

	UFUNCTION(Client, Reliable)
	void Client_StopPreparedAudio();

protected:
	UFUNCTION()
	void OnAudioLoadedFromUrl(UglTFRuntimeAsset* Asset);
};
