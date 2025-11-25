// Source/MVE/GenAISenderReceiver.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetTypes.h"
#include "SenderReceiver.generated.h"

UCLASS()
class MVE_API USenderReceiver : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
// ---------------------------------- UI 바인딩할 델리게이트 ---------------------------------------//
    // 에셋 생성 완료시 발동
    UPROPERTY(BlueprintAssignable, Category = "DOWNLOAD")
    FOnAssetLoaded OnAssetGenerated;

    // 다운로드 진행중 발동
    UPROPERTY(BlueprintAssignable, Category = "DOWNLOAD")
    FOnDownloadProgress OnDownloadProgress;

    // 생성 요청에 대한 서버 응답 수신시 발동
    UPROPERTY(BlueprintAssignable, Category = "DOWNLOAD")
    FOnGenerationResponse OnGenerationResponse;
    
    // -------------------------------------  서버 설정 ----------------------------------------// 
    // TODO 나중에 로그 가져와야한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FString ServerURL = TEXT("http://127.0.0.1:8000");

    // api TODO 얘도 똑같이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FString GenerateEndpoint = TEXT("/upload/api/generate");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FString LocalAssetDiscovery = TEXT("SourceAssets");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float RequestTimeOut = 30.0f;
  
    // --------------------------------------- 송신부 ------------------------------------------------//
    UFUNCTION(BlueprintCallable, Category = "Request")
    void SendGenerationRequest(
        const FString& Prompt,
        const FString& UserEmail,
        const FString& OptionalImagePath = TEXT("")
    );
    // ---------------------------------------- 수신부 -----------------------------------------------//
    UFUNCTION(BlueprintCallable, Category = "Responses")
    void DownloadFileServer(const FAssetMetadata& Metadata);

    // 로컬에서 직접 로드
    UFUNCTION(BlueprintCallable, Category = "Responses")
    void LoadLocalAsset(const FAssetMetadata& Metadata);
    

    UFUNCTION(BlueprintCallable, Category = "Utill")
    FString GetFileExtensionAsset(EAssetType AssetType);

    // 저장 디렉 가져오기
    UFUNCTION(BlueprintCallable, Category = "Utill")
    FString GetSaveDirectory();
    
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
    

    // 에셋 타입별 확장자 (glb 이긴 한데 이후에 쓸가봐)
    static const TMap<EAssetType, FString> AssetTypeExtensions;
};