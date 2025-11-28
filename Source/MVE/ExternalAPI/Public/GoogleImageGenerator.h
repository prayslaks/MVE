// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "GoogleImageGenerator.generated.h"

UCLASS()
class MVE_API AGoogleImageGenerator : public AActor
{
	GENERATED_BODY()

public:
	AGoogleImageGenerator();

	UFUNCTION(BlueprintCallable, Category = "Gemini API")
	void GenerateImage(FString PromptText);
	
	virtual void BeginPlay() override;

private:
	// HTTP 요청 완료 시 호출될 콜백 함수
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// API 키 (실제 게임에서는 별도 관리 필요)
	const FString ApiKey = TEXT("");
	
	// Base64 문자열을 UTexture2D로 변환하는 핵심 함수
	UTexture2D* LoadTextureFromBase64(const FString& Base64String);
	
	// 만들어진 텍스처를 메쉬에 적용하는 함수
	void ApplyTextureToMesh(UTexture2D* NewTexture);
	
	// 텍스처를 적용할 대상 메쉬 컴포넌트
	UPROPERTY()
	UStaticMeshComponent* TargetMesh;

	// 런타임에 교체 가능한 동적 재질 인스턴스
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
};