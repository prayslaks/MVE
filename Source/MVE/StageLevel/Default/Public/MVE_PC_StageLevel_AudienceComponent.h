// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_PC_StageLevel_AudienceComponent.generated.h"

class AMVE_PC_StageLevel;
class USoundBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_PC_StageLevel_AudienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_PC_StageLevel_AudienceComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	AMVE_PC_StageLevel* GetBindingPC() const;

protected:
	// 물건 던지기 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> ThrowSound;
	
	// 응원봉 흔들기 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> WaveLightStickSound;

	// 환호 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> CheerUpSound;

public:
	// 던지기 기능과 관련된 ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_ThrowSomething(const FVector& Location, const FVector& Direction);
	
	// 응원봉 기능과 관련된 ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_WaveLightStick(const FVector& Location);

	// 포토 기능과 관련된 ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_TakePhoto();

	// 환호 기능과 관련된 ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_CheerUp(const FVector& Location);
};
