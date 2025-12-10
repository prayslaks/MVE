// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h" // Required for USoundBase
#include "MVE_API_PCTest_AudComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_API_PCTest_AudComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_API_PCTest_AudComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Interaction")
	USoundBase* TakePhotoSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Interaction")
	USoundBase* CheerUpSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Interaction")
	USoundBase* SwingStickSound;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTakePhoto(FVector Location);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCheerUp(FVector Location);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Interaction")
	void TakePhotoOnServer(FVector Location);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Interaction")
	void CheerUpOnServer(FVector Location);

	// Server RPC: Client calls this to request a light stick swing on the server
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "LightStick")
	void SwingLightStickOnServer(FVector Location);

	// Multicast RPC: Server calls this to replicate light stick swing to all clients
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwingLightStick(FVector Location);
};
