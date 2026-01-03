// Source/MVE/SenderReceiver.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetTypes.h"
#include "HttpModule.h"
#include "Templates/SharedPointer.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "SenderReceiver.generated.h"


 // [AI 서버 정보]
 // - 서버 주소: http://172.16.20.234:8001
 // - 생성 API:  POST /generate_mesh
 

UCLASS()
class MVE_API USenderReceiver : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // ========================================================================
    //                          델리게이트 (Widget 바인딩용)
    // ========================================================================

    // 
     // 에셋 로드 완료 시 발동
     // 
     // 다운로드 + UE 오브젝트 변환까지 완료된 후 호출
     // Widget에서 이 델리게이트에 바인딩하여 결과 수신
    UPROPERTY(BlueprintAssignable, Category = "GenAI|Delegate")
    FOnAssetLoaded OnAssetLoaded;

    // 
     // AI 서버 응답 수신 시 발동 (다운로드 전)
     // 
     // 메타데이터만 필요한 경우 사용
     
    UPROPERTY(BlueprintAssignable, Category = "GenAI|Delegate")
    FOnGenerationResponse OnGenerationResponse;

    // 다운로드 진행률
    UPROPERTY(BlueprintAssignable, Category = "GenAI|Delegate")
    FOnDownloadProgress OnDownloadProgress;

    //
     // 음악 분석 완료 시 발동
     //
     // AI 서버가 음악 메타데이터 분석 후 타임라인별 이펙트 데이터 반환
     // Widget에서 이 델리게이트에 바인딩하여 EffectSequenceManager에 데이터 전달
    UPROPERTY(BlueprintAssignable, Category = "GenAI|Delegate")
    FOnMusicAnalysisComplete OnMusicAnalysisComplete;

    // ========================================================================
    //                          서버 설정
    // ========================================================================

    // ComfyUI 서버 URL (메시 생성용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString ComfyUIServerURL = TEXT("http://172.16.100.123:8001");

    // 음악 분석 서버 URL
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString MusicAnalysisServerURL = TEXT("http://ec2-13-125-244-186.ap-northeast-2.compute.amazonaws.com");

    // 메시 생성 API 엔드포인트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString GenerateEndpoint = TEXT("/generate_3D_obj");

    // 음악 분석 API 엔드포인트 (배치 분석)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString MusicAnalysisEndpoint = TEXT("/songs/auto-process");

    // 다운로드 파일 저장 폴더명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    FString LocalSaveFolder = TEXT("GenAIAssets");

    // HTTP 요청 타임아웃 (초) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI|Config")
    float RequestTimeout = 600.0f;

    // ========================================================================
    //                          송신 API (Unreal → AI)
    // ========================================================================
	
     // SendGenerationRequest
     // 
     // AI 서버에 메시 생성 요청 전송
     // 프롬프트 + 이미지 + 메타데이터를 multipart/form-data로 전송
     // 
     // @param Prompt    - 생성할 에셋 설명 텍스트
     // @param UserEmail - 요청자 이메일 (계정 ID)
     // @param ImagePath - 참조 이미지 파일 경로 (선택, 빈 문자열 가능)
     // 
     // [전송 데이터]
     // - metadata (JSON): {prompt, user_email, request_id}
     // - image (Binary): PNG/JPG 이미지 파일 (선택)
     // 
     // [결과]
     // - 성공 시: OnGenerationResponse 발동 후 자동 다운로드 → OnAssetLoaded 발동
     // - 실패 시: OnGenerationResponse에서 에러 메시지 전달
     
    UFUNCTION(BlueprintCallable, Category = "GenAI|Send")
    void SendGenerationRequest(
        const FString& Prompt,
        const FString& UserEmail,
        const FString& ImagePath = TEXT("")
    );

    // SendMusicAnalysisRequest
     //
     // AI 서버에 음악 분석 요청 전송
     // 음악 메타데이터(Title, Artist)를 JSON으로 전송하여
     // TimeStamp별 이펙트 AssetID 배열 수신
     //
     // @param Title  - 음악 제목
     // @param Artist - 아티스트 이름
     //
     // [전송 데이터 (JSON)]
     // {
     //   "title": "노래 제목",
     //   "artist": "가수 이름"
     // }
     //
     // [응답 데이터 (JSON)]
     // {
     //   "success": true,
     //   "data": [
     //     {"timestamp": 50, "category": "Spotlight", "asset_id": "VFX.Spotlight.FastSpeed"},
     //     {"timestamp": 120, "category": "Flame", "asset_id": "VFX.Flame.VeryFastSizeAndVeryFastSpeed"},
     //     ...
     //   ]
     // }
     //
     // [결과]
     // - 성공 시: OnMusicAnalysisComplete 발동 (TArray<FEffectSequenceData> 전달)
     // - 실패 시: OnMusicAnalysisComplete에서 에러 메시지 전달

    UFUNCTION(BlueprintCallable, Category = "GenAI|Send")
    void SendMusicAnalysisRequest(
        const FString& Title,
        const FString& Artist
    );

    // SendBatchMusicAnalysisRequest
     //
     // AI 서버에 여러 곡의 음악 분석 요청을 배치로 전송
     // 재생목록 전체를 JSON 배열로 전송하여
     // 각 곡별 TimeStamp별 이펙트 AssetID 배열 수신
     //
     // [요청 예시]
     // POST /api/music-analysis/batch
     // {
     //   "songs": [
     //     { "title": "Super Shy", "artist": "NewJeans" },
     //     { "title": "Its Jennie", "artist": "Jennie" }
     //   ]
     // }
     //
     // [응답 예시]
     // {
     //   "results": [
     //     {
     //       "title": "Super Shy",
     //       "artist": "NewJeans",
     //       "effects": [
     //         { "timeStamp": 10, "assetID": "VFX.Spotlight.FastSpeed" },
     //         ...
     //       ]
     //     },
     //     ...
     //   ]
     // }
     //
     // [결과]
     // - 성공 시: 각 곡마다 OnMusicAnalysisComplete 발동
     // - 실패 시: OnMusicAnalysisComplete에서 에러 메시지 전달

    UFUNCTION(BlueprintCallable, Category = "GenAI|Send")
    void SendBatchMusicAnalysisRequest(const TArray<FAudioFile>& AudioFiles);

    // ========================================================================
    //                          수신 API (AI → Unreal)
    // ========================================================================

    //
     // DownloadFromFileServer
     // 
     // 파일 서버에서 에셋 다운로드
     // 보통 SendGenerationRequest 후 자동 호출됨
     // 
     // @param Metadata - 다운로드할 에셋 메타데이터 (RemotePath 필요)
     
    UFUNCTION(BlueprintCallable, Category = "GenAI|Receive")
    void DownloadFromFileServer(const FAssetMetadata& Metadata);

    //
     // LoadFromLocalFile
     // 
     // 로컬 파일 직접 로드 (테스트/디버그용)
     // 서버 없이 로컬 GLB/PNG 파일 테스트 시 사용
     // 
     // @param Metadata - 로드할 에셋 메타데이터 (RemotePath에 로컬 경로 설정)
     
    UFUNCTION(BlueprintCallable, Category = "GenAI|Debug")
    void LoadFromLocalFile(const FAssetMetadata& Metadata);

    // ========================================================================
    //                          유틸리티
    // ========================================================================

    // 에셋 타입별 파일 확장자 반환 
    UFUNCTION(BlueprintCallable, Category = "GenAI|Utility")
    FString GetFileExtension(EAssetType AssetType);

    // 저장 디렉토리 경로 반환 (없으면 생성) 
    UFUNCTION(BlueprintCallable, Category = "GenAI|Utility")
    FString GetSaveDirectory();

    // 날짜 문자열 파싱 (YYYY-MM-DD HH:MM:SS → FDateTime) 
    UFUNCTION(BlueprintCallable, Category = "GenAI|Utility")
    static FDateTime ParseDateString(const FString& DateStr);

    // 에셋 타입별 확장자 맵 
    static const TMap<EAssetType, FString> AssetTypeExtensions;

    UFUNCTION()
    void LogNetworkDiagnostics(const FString& URL);

    void AnalyzeConnectionError(TSharedPtr<IHttpResponse> Response, bool bWasSuccessful);

private:

    // ========================================================================
    //                          내부 HTTP 콜백
    // ========================================================================

    // 생성 요청 응답 처리 → 메타데이터 파싱 → 자동 다운로드
    void HandleGenerationResponse(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful
    );

    // 음악 분석 요청 응답 처리 → JSON 파싱 → FEffectSequenceData 배열 생성 → 델리게이트 발동
    void HandleMusicAnalysisResponse(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful
    );

    // 배치 음악 분석 요청 응답 처리 → 여러 곡의 분석 결과 처리
    void HandleBatchMusicAnalysisResponse(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful
    );

    // 다운로드 완료 처리 → 파일 저장 → UE 에셋 변환 → 델리게이트 발동
    void HandleDownloadComplete(
        TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        FAssetMetadata Metadata
    );

    // ========================================================================
    //                          에셋 로더
    // ========================================================================

    // 메타데이터 기반 에셋 로드 (타입별 분기) 
    UObject* ConvertFileToAsset(const FAssetMetadata& Metadata);

    // 이미지 파일 → UTexture2D 
    UTexture2D* LoadImageAsTexture(const FString& FilePath);

    // GLB 파일 → USkeletalMesh 

	UObject* LoadMeshFromGLB(const FString& FilePath);
	
    UStaticMesh* LoadStaticMeshFromGLB(const FString& FilePath);

public:
    FTimerHandle StatusCheckTimer;
    FString CurrentRequestID;

    void CheckGenerationStatus();
    void OnStatusCheckComplete(
        TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
        TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful
    );

private:
    // 배치 음악 분석 요청 시 전송한 AudioFiles (응답 매칭용)
    UPROPERTY()
    TArray<FAudioFile> LastBatchRequestAudioFiles;
};