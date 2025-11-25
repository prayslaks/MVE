// Source/MVE/GenAISenderReceiver.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SenderReceiver.generated.h"

class UTextureRenderTarget2D;
class UNiagaraComponent;
class UNiagaraDataInterfaceTexture;

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

    // -------------------------------------------- comfyUI 연동 --------------------------------------------------//

    // comfyUI 서버 설정
    UPROPERTY(EditAnywhere, Category = "ComfyUI")
    FString ComfyUIServerURL = TEXT("https://127.0.0.1:7800");

    UPROPERTY(EditAnywhere, Category = "ComfyUI")
    FString ComfyUIUploadEndpoint = TEXT("/upload/image");

    UPROPERTY(EditAnywhere, Category = "ComfyUI")
    FString ComfyUIPromptEndpoint = TEXT("/Prompt");

    // comfyui 이미지 생성 완료 델리게이트
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComfyUIImageGenerated,
        UTexture2D*, GeneratedTexture,
        FGuid, RequestID);
    UPROPERTY(BlueprintAssignable, Category = "ComfyUI")
    FOnComfyUIImageGenerated OnComfyUIImageGenerated;

    //랜더 타겟을 comfyui 에 전송하며 이미지 생성 요청
    UFUNCTION(BlueprintCallable, Category = "ComfyUI")
    FGuid SendRenderTargetToComfyUI(UTextureRenderTarget2D* RenderTarget,
        const FString& WorkflowPrompt = TEXT(""),
        const FString& FileName = TEXT(""));

    //base64 문자열로 받은거 texture 로 변환
    UFUNCTION(BlueprintCallable, Category = "ComfyUI")
    UTexture2D* Base64ToTexture2D(FString& Base64Data);

    UFUNCTION(BlueprintCallable, Category = "Niagara")
    bool ApplyTextureToNiagara(UNiagaraComponent* NiagaraComp,
        UTexture2D* Texture,
        FName ParameterName = TEXT("UserTTexture"));

    UFUNCTION(BlueprintCallable, Category = "Material")
    bool ApplyTextureToMaterial(UMaterialInstanceDynamic* MID,
        UTexture2D* Texture,
        FName ParameterName = TEXT("UserTTexture"));

    void OnComfyUIUploadComplete(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        FGuid RequestID,
        FString UserEmail);

    void OnComfyUIPromptComplete(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        FGuid RequestID);

    void OnComfyUIResultFetched(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        FGuid RequestID);
};