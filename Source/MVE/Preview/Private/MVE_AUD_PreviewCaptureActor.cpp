
#include "../Public/MVE_AUD_PreviewCaptureActor.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkyLightComponent.h"


AMVE_AUD_PreviewCaptureActor::AMVE_AUD_PreviewCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
    RootComponent = SceneCaptureComponent;

	// 캡처 설정
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bCaptureOnMovement = false;
}

void AMVE_AUD_PreviewCaptureActor::SetCaptureTarget(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
    
	if (TargetActor && SceneCaptureComponent)
	{
		SceneCaptureComponent->ShowOnlyActors.Empty();
		SceneCaptureComponent->ShowOnlyActors.Add(TargetActor);
        
		if (RenderTarget)
		{
			SceneCaptureComponent->TextureTarget = RenderTarget;
		}
	}
}

