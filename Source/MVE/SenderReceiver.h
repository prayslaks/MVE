// Source/MVE/GenAISenderReceiver.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetTypes.h"
#include "SenderReceiver.generated.h"

UCLASS()
class MVE_API UGenAISenderReceiver : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
// ---------------------------------- UI 바인딩할 델리게이트 ---------------------------------------//
    // 에셋 생성 완료시 발동
    UPROPERTY(BlueprintAssignable, Category = "GenAI")
    FOnAssetGenerated OnAssetGenerated;

    // 다운로드 진행중 발동
    UPROPERTY(BlueprintAssignable, Category = "GenAI")
    FOnDownloadProgress OnDownloadProgress;

public:
    // 서버 설정
    // TODO 나중에 로그 가져와야한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString ServerURL = TEXT("http://localhost:8000");

    // api TODO 얘도 똑같이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString GenerateEndpoint = TEXT("/api/generate");

public:
  
    // --------------------------------------- 송신부 ------------------------------------------------//
    UFUNCTION(BlueprintCallable, Category = "GenAI")
    void RequestGeneration(
        const FString& Prompt,
        const FString& UserEmail,
        const FString& OptionalImagePath = TEXT("")
    );
    // ---------------------------------------- 수신부 -----------------------------------------------//
    UFUNCTION(BlueprintCallable, Category = "GenAI")
    void DownloadAsset(const FAssetMetadata& Metadata);

    // 로컬에서 직접 로드
    UFUNCTION(BlueprintCallable, Category = "GenAI")
    void LoadLocalAsset(const FAssetMetadata& Metadata);

private:
    // 송신내부구현
    void OnGenerationRequestComplete(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful
    );

    // 수신 내부 구현
    void OnAssetDownloaded(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        FAssetMetadata Metadata
    );
    // 타입별 에셋 로드
    UObject* LoadAssetFromLocalFile(const FAssetMetadata& Metadata);

    // 이미지 파일 로더
    UTexture2D* LoadImageFromFile(const FString& FilePath);

    // GLB 파일 로더
    USkeletalMesh* LoadMeshFromFile(const FString& FilePath);


};