// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVE_StageLevel_AudObject.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_AudCamera.generated.h"

class USpotLightComponent;
class UTimelineComponent;
class UCurveFloat;

UCLASS()
class MVE_API AMVE_StageLevel_AudCamera : public AMVE_StageLevel_AudObject
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_AudCamera();

	UFUNCTION(BlueprintCallable, Category = "CameraFlash")
	void TriggerFlash();
	
protected:
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void OnFlashUpdate(float Value) const;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> SpotLightComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTimelineComponent> FlashTimelineComp;

	UPROPERTY(EditAnywhere, Category = "CameraFlash")
	TObjectPtr<UCurveFloat> FlashCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFlash")
	float FlashIntensityMultiplier = 5000.0f;
};
