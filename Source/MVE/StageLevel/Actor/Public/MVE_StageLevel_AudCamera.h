// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVE_StageLevel_AudObject.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_AudCamera.generated.h"

class UArrowComponent;
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
	void TakePhoto(const AController* FlashMan);

protected:
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<USpotLightComponent> SpotLightComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<UStaticMeshComponent> FlashBeamMeshComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<UAudioComponent> AudioComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<UArrowComponent> ArrowComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<UTimelineComponent> FlashTimelineComp;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> FlashBeamMaterialMID;

	UPROPERTY(EditAnywhere, Category = "MVE|TakePhoto")
	TObjectPtr<UCurveFloat> FlashCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MVE|TakePhoto")
	float FlashIntensityMultiplier = 5000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MVE|TakePhoto", meta = (ClampMin = "0.0"))
	float FlashEffectiveDistance = 5000.0f;

	// 플래시 유효 각도의 Dot Product 값. 1에 가까울수록 정면. (0.5 = 60도 반각)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MVE|TakePhoto", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FlashAngleDotThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|TakePhoto")
	TObjectPtr<USoundBase> TakePhotoSound;
	
private:
	UFUNCTION(Category = "MVE|TakePhoto")
	void OnFlashUpdate(float Value) const;
};
