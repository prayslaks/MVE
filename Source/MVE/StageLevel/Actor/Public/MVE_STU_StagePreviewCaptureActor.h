// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_STU_StagePreviewCaptureActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UDirectionalLightComponent;
class UPointLightComponent;
class USkyLightComponent;

/**
 * Stage Preview Capture Actor
 * PreviewStageLevel의 무대를 SceneCapture2D로 캡처하여 위젯에 표시
 * - 무대 전체를 캡처 (이펙트 포함)
 * - RenderTarget으로 출력
 * - EffectSequencePreview 위젯에서 사용
 */
UCLASS()
class MVE_API AMVE_STU_StagePreviewCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	AMVE_STU_StagePreviewCaptureActor();

protected:
	virtual void BeginPlay() override;

public:
	/** SceneCapture2D 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	USceneCaptureComponent2D* SceneCaptureComponent;

	/** 캡처 결과를 저장할 RenderTarget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	UTextureRenderTarget2D* RenderTarget;

	/**
	 * RenderTarget 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);

	/**
	 * 카메라 위치/각도 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void SetCameraTransform(const FVector& Location, const FRotator& Rotation);

private:
	/** 프리뷰용 메인 조명 */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UDirectionalLightComponent* MainLight;

	/** 프리뷰용 보조 조명 */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UPointLightComponent* FillLight;

	/** 프리뷰용 림라이트 */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UPointLightComponent* RimLight;

	/** 프리뷰용 스카이라이트 */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	USkyLightComponent* SkyLight;
};
