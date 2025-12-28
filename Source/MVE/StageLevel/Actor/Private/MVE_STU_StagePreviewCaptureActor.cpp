// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_STU_StagePreviewCaptureActor.h"
#include "MVE.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"

AMVE_STU_StagePreviewCaptureActor::AMVE_STU_StagePreviewCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// SceneCapture2D 컴포넌트 생성
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	RootComponent = SceneCaptureComponent;

	// 캡처 설정 - 전체 씬 렌더링
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bCaptureOnMovement = false;

	// Capture Source 설정 - Final Color 사용
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// Show Flags 설정 - 모든 렌더링 기능 활성화
	SceneCaptureComponent->ShowFlags.SetLighting(true);
	SceneCaptureComponent->ShowFlags.SetSkyLighting(true);
	SceneCaptureComponent->ShowFlags.SetReflectionEnvironment(true);
	SceneCaptureComponent->ShowFlags.SetGlobalIllumination(true);
	SceneCaptureComponent->ShowFlags.SetAmbientOcclusion(true);
	SceneCaptureComponent->ShowFlags.SetDynamicShadows(true);
	SceneCaptureComponent->ShowFlags.SetTexturedLightProfiles(true);
	SceneCaptureComponent->ShowFlags.SetPostProcessing(true);
	SceneCaptureComponent->ShowFlags.SetAntiAliasing(true);

	// 조명 생성
	MainLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MainLight"));
	MainLight->SetupAttachment(RootComponent);
	MainLight->SetRelativeRotation(FRotator(-45.0f, 45.0f, 0.0f));
	MainLight->Intensity = 5.0f;
	MainLight->SetCastShadows(true);

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetRelativeLocation(FVector(-200.0f, -200.0f, 100.0f));
	FillLight->Intensity = 2000.0f;
	FillLight->AttenuationRadius = 1000.0f;
	FillLight->SetCastShadows(false);

	RimLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(RootComponent);
	RimLight->SetRelativeLocation(FVector(200.0f, 200.0f, 150.0f));
	RimLight->Intensity = 1500.0f;
	RimLight->AttenuationRadius = 800.0f;
	RimLight->SetCastShadows(false);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->Intensity = 1.0f;
	SkyLight->SetCastShadows(false);
}

void AMVE_STU_StagePreviewCaptureActor::BeginPlay()
{
	Super::BeginPlay();

	// 에디터에서 RenderTarget이 설정되어 있으면 자동으로 SceneCaptureComponent에 연결
	if (RenderTarget && SceneCaptureComponent)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
		PRINTLOG(TEXT("BeginPlay: SceneCaptureComponent에 RenderTarget 자동 설정됨: %s"), *RenderTarget->GetName());
	}
	else if (!RenderTarget)
	{
		PRINTLOG(TEXT("⚠️ BeginPlay: RenderTarget이 설정되지 않았습니다. 에디터에서 RT_StagePreview를 할당하세요."));
	}
}

void AMVE_STU_StagePreviewCaptureActor::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	RenderTarget = InRenderTarget;

	if (SceneCaptureComponent && RenderTarget)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
		PRINTLOG(TEXT("StagePreviewCaptureActor RenderTarget 설정됨: %s"), *RenderTarget->GetName());
	}
	else
	{
		PRINTLOG(TEXT("StagePreviewCaptureActor RenderTarget 설정 실패"));
	}
}

void AMVE_STU_StagePreviewCaptureActor::SetCameraTransform(const FVector& Location, const FRotator& Rotation)
{
	SetActorLocation(Location);
	SetActorRotation(Rotation);

	PRINTLOG(TEXT("StagePreviewCaptureActor 카메라 위치 설정: %s, 회전: %s"),
		*Location.ToString(), *Rotation.ToString());
}
