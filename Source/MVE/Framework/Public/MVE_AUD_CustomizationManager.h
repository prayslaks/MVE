
#pragma once

#include "CoreMinimal.h"
#include "Data/InputPromptData.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVE_AUD_CustomizationManager.generated.h"


UCLASS()
class MVE_API UMVE_AUD_CustomizationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 이미지 첨부 다이얼로그
	UFUNCTION(BlueprintCallable)
	FString OpenReferenceImageDialog();

	// 생성 요청
	UFUNCTION(BlueprintCallable)
	void RequestModelGeneration(const FString& PromptText);

private:
	// 현재 첨부된 참고 이미지
	TArray<uint8> ReferenceImageData;
	FString ReferenceImageFormat;
	FString ReferenceImageFileName;

	// 이미지 파일 로드
	bool LoadReferenceImage(const FString& FilePath);

	// HTTP 요청으로 서버에 전송
	void SendToExternalServer(const FInputPromptData& Request);

	// HTTP 응답 콜백
	void OnModelGenerationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	// 생성 완료 처리
	void OnModelGenerationComplete(const FString& ModelID, const FString& GLBFileURL);

};
