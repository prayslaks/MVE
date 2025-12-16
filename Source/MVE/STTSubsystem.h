#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioCaptureCore.h"
#include "GameplayTagContainer.h"
#include "IWebSocket.h" // WebSocket 모듈 포함
#include "STTSubsystem.generated.h"

// ------------------------------------ 응답 데이터 ------------------------------------

USTRUCT(BlueprintType)
struct FSTTResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString TranscribedText;

    UPROPERTY(BlueprintReadWrite)
    bool bIsFinal = false; // 문장이 완성되었는지 여부

    UPROPERTY(BlueprintReadWrite)
    float Confidence = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FString Language;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Keywords;

    UPROPERTY(BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadWrite)
    FString ErrorMessage;
};

// ------------------------------------ 델리게이트 ------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTTextStream, const FSTTResponse&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeywordDetected, FGameplayTag, KeywordTag, const FString&, FullText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTConnectionStatus, bool, bIsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTError, const FString&, ErrorMessage);

// ------------------------------------ STT Subsystem ------------------------------------

UCLASS()
class MVE_API USTTSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ------------------------------------ 델리게이트 ------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTTextStream OnSTTTextStream;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnKeywordDetected OnKeywordDetected;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTConnectionStatus OnConnectionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTError OnSTTError;

    // ------------------------------------ 서버 설정 ------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    FString WebSocketURL = TEXT("ws://172.16.100.40:8001/ws");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    int32 SampleRate = 48000; // AI 서버가 기대하는 샘플레이트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    int32 NumChannels = 1; // 모노 채널

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float AudioGain = 8.0f; // 마이크 볼륨 증폭 (1.0 = 원본, 권장: 3.0~8.0, WASAPI는 낮게 캡처됨)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    bool bUseAutoNormalization = true; // RMS 기반 자동 정규화 (VAD 최적화)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float TargetRMS = 0.1f; // 목표 RMS 레벨 (0.05~0.15 권장, 기본 0.1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    bool bUsePeakNormalization = true; // 피크 정규화 (최대 볼륨 강제, VAD 극대화)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    TMap<FString, FGameplayTag> KeywordTagMap;

    // ------------------------------------ Public API ------------------------------------

    UFUNCTION(BlueprintCallable, Category = "STT|Control")
    void StartTranscribing();

    UFUNCTION(BlueprintCallable, Category = "STT|Control")
    void StopTranscribing();

    UFUNCTION(BlueprintCallable, Category = "STT|Keywords")
    void RegisterKeyword(const FString& Keyword, FGameplayTag ActionTag);
    
    // ------------------------------------ 오디오 캡처 및 버퍼 ------------------------------------

    TUniquePtr<Audio::FAudioCapture> AudioCapture;
    bool bIsActive = false;

    // 마이크에서 받은 데이터를 512 바이트 청크로 쪼개기 위해 임시 저장하는 버퍼
    TArray<uint8> PendingAudioBuffer; 

    // 오디오 캡처 콜백 (오디오 스레드에서 실행됨)
    void OnAudioCapture(
        const float* Data, 
        int32 NumFrames, 
        int32 NumChannels,
        int32 SampleRate, 
        double StreamTime, 
        bool bOverflow
    );

    // ------------------------------------ WebSocket ------------------------------------

    TSharedPtr<IWebSocket> WebSocket;
    
    void ConnectWebSocket();
    void CloseWebSocket();

    // 소켓 이벤트 핸들러
    void OnWebSocketConnected();
    void OnWebSocketConnectionError(const FString& Error);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketMessage(const FString& Message);

    // 오디오 데이터 전송
    void SendAudioChunk(const TArray<uint8>& AudioData);

    // ------------------------------------ 데이터 처리 ------------------------------------

    FSTTResponse ParseJSONResponse(const FString& JSONString);
    void DispatchKeywords(const FString& Text);
    void BroadcastError(const FString& ErrorMessage);

    // 로깅
    void LogDebug(const FString& Message);
    void LogError(const FString& Message);
};