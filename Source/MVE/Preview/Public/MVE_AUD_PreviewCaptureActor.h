
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_AUD_PreviewCaptureActor.generated.h"

class UPointLightComponent;

UCLASS()
class MVE_API AMVE_AUD_PreviewCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	AMVE_AUD_PreviewCaptureActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneCaptureComponent2D* SceneCaptureComponent;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	UTextureRenderTarget2D* RenderTarget;

	// 캡처 타겟 (회전 중심점)
	UPROPERTY(BlueprintReadWrite)
	AActor* TargetActor;
    
	// 캡처할 대상 설정
	void SetCaptureTarget(AActor* TargetActor);

private:
	// 프리뷰용 조명들
	UPROPERTY(VisibleAnywhere)
	UDirectionalLightComponent* MainLight;
    
	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* FillLight;
    
	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* RimLight;

	UPROPERTY(VisibleAnywhere)
	USkyLightComponent* SkyLight;
};
