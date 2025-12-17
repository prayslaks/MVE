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
	
	virtual void BeginPlay() override;

	UFUNCTION(Category = "MVE|TakePhoto")
	void TakePhoto() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components")
	TObjectPtr<USpotLightComponent> SpotLightComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components")
	TObjectPtr<UAudioComponent> AudioComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components")
	TObjectPtr<UTimelineComponent> FlashTimelineComp;

	UPROPERTY(EditAnywhere, Category = "MVE|TakePhoto")
	TObjectPtr<UCurveFloat> FlashCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MVE|TakePhoto")
	float FlashIntensityMultiplier = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "MVE|TakePhoto")
	TObjectPtr<USoundBase> TakePhotoSound;
	
private:
	UFUNCTION(Category = "MVE|TakePhoto")
	void OnFlashUpdate(float Value) const;
};
