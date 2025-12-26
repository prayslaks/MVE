
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

	// 캡처 설정 - ShowOnlyList 모드 유지 (배경 제거용)
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bCaptureOnMovement = false;

	// Capture Source 설정 - Final Color 사용
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// ⭐ Show Flags 설정 - 모든 렌더링 기능 활성화
	SceneCaptureComponent->ShowFlags.SetLighting(true);
	SceneCaptureComponent->ShowFlags.SetSkyLighting(true);
	SceneCaptureComponent->ShowFlags.SetReflectionEnvironment(true);
	SceneCaptureComponent->ShowFlags.SetGlobalIllumination(true);
	SceneCaptureComponent->ShowFlags.SetAmbientOcclusion(true);
	SceneCaptureComponent->ShowFlags.SetDynamicShadows(true);
	SceneCaptureComponent->ShowFlags.SetTexturedLightProfiles(true);
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

