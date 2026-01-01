// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MVE_PS_StageLevel.generated.h"

UCLASS()
class MVE_API AMVE_PS_StageLevel : public APlayerState
{
	GENERATED_BODY()
	
public:
	AMVE_PS_StageLevel();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	FORCEINLINE bool GetIsAudioReady() const { return bIsAudioReady; }
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsAudioReady(const bool Value);
	
private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_IsAudioReady)
	bool bIsAudioReady = false;
	
	UFUNCTION()
	void OnRep_IsAudioReady() const;
};