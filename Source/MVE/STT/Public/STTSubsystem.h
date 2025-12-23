// STTSubsystem.h - 수정된 버전 (서버 응답 형식 매칭 + UI 연동)
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioCaptureCore.h"
#include "GameplayTagContainer.h"
#include "IWebSocket.h"
#include "STTSubsystem.generated.h"

// ================================================================================================
// 명령어 타입 Enum - Python 서버의 parse_command()와 매칭
// ================================================================================================

UENUM(BlueprintType)
enum class ESTTCommandType : uint8
{
    None        UMETA(DisplayName = "None"),
    NextTrack   UMETA(DisplayName = "다음 곡"),
    StopTrack   UMETA(DisplayName = "정지"),
    PlayTrack   UMETA(DisplayName = "재생"),
    ConcertOpen UMETA(DisplayName = "콘서트 열기"),
    ConcertClose UMETA(DisplayName = "콘서트 닫기")
};

// ================================================================================================
// 응답 데이터 구조체 - Python 서버 응답과 매칭
// ================================================================================================

USTRUCT(BlueprintType)
struct FSTTResponse
{
    GENERATED_BODY()

    // 서버에서 보내는 "content" 필드
    UPROPERTY(BlueprintReadWrite)
    FString TranscribedText;

    // 서버에서 보내는 "command" 필드 (문자열)
    UPROPERTY(BlueprintReadWrite)
    FString CommandString;

    // 파싱된 명령어 타입
    UPROPERTY(BlueprintReadWrite)
    ESTTCommandType CommandType = ESTTCommandType::None;

    // 서버에서 보내는 "is_command" 필드
    UPROPERTY(BlueprintReadWrite)
    bool bIsCommand = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsFinal = false;

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

// ================================================================================================
// 델리게이트 - UI 바인딩용
// ================================================================================================

// 텍스트 스트림 수신
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTTextStream, const FSTTResponse&, Response);

// 명령어 감지 (새로 추가)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSTTCommandReceived, ESTTCommandType, CommandType, const FString&, OriginalText);

// 키워드 감지 (기존 유지)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeywordDetected, FGameplayTag, KeywordTag, const FString&, FullText);

// 연결 상태 변경
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTConnectionStatus, bool, bIsConnected);

// 에러 발생
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSTTError, const FString&, ErrorMessage);

// 오디오 레벨 (UI 레벨 미터용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioLevelChanged, float, NormalizedLevel);

// ================================================================================================
// STT Subsystem 클래스
// ================================================================================================

UCLASS()
class MVE_API USTTSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ================================================================================================
    // 델리게이트 (BlueprintAssignable = UI에서 바인딩 가능)
    // ================================================================================================

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTTextStream OnSTTTextStream;

    // 🔥 새로 추가: 명령어 수신 이벤트
    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTCommandReceived OnSTTCommandReceived;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnKeywordDetected OnKeywordDetected;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTConnectionStatus OnConnectionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnSTTError OnSTTError;

    // 🔥 새로 추가: 오디오 레벨 이벤트 (UI 레벨 미터용)
    UPROPERTY(BlueprintAssignable, Category = "STT|Events")
    FOnAudioLevelChanged OnAudioLevelChanged;

    // ================================================================================================
    // 서버 설정
    // ================================================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    FString WebSocketURL = TEXT("ws://172.16.20.236:8001/ws");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    int32 SampleRate = 48000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    int32 NumChannels = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float AudioGain = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    bool bUseAutoNormalization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float TargetRMS = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    bool bUsePeakNormalization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config", meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float NoiseGateThreshold = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Config")
    TMap<FString, FGameplayTag> KeywordTagMap;

    // ================================================================================================
    // 상태 조회 (UI용)
    // ================================================================================================

    UPROPERTY(BlueprintReadOnly, Category = "STT|Status")
    bool bIsConnected = false;

    UPROPERTY(BlueprintReadOnly, Category = "STT|Status")
    FString LastReceivedText;

    UPROPERTY(BlueprintReadOnly, Category = "STT|Status")
    ESTTCommandType LastCommandType = ESTTCommandType::None;

    UPROPERTY(BlueprintReadOnly, Category = "STT|Status")
    float CurrentAudioLevel = 0.0f;

    // ================================================================================================
    // Public API
    // ================================================================================================

    UFUNCTION(BlueprintCallable, Category = "STT")
    void StartTranscribing();

    UFUNCTION(BlueprintCallable, Category = "STT")
    void StopTranscribing();

    UFUNCTION(BlueprintCallable, Category = "STT")
    void RegisterKeyword(const FString& Keyword, FGameplayTag ActionTag);

    UFUNCTION(BlueprintPure, Category = "STT")
    bool IsActive() const { return bIsActive; }

    UFUNCTION(BlueprintPure, Category = "STT")
    static FString GetCommandDisplayName(ESTTCommandType CommandType);

    // ================================================================================================
    // 내부 구현
    // ================================================================================================

    TUniquePtr<Audio::FAudioCapture> AudioCapture;
    bool bIsActive = false;
    TArray<uint8> PendingAudioBuffer;

    void OnAudioCapture(
        const float* Data,
        int32 NumFrames,
        int32 NumChannels,
        int32 SampleRate,
        double StreamTime,
        bool bOverflow
    );

    // WebSocket
    TSharedPtr<IWebSocket> WebSocket;

    void ConnectWebSocket();
    void CloseWebSocket();

    void OnWebSocketConnected();
    void OnWebSocketConnectionError(const FString& Error);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketMessage(const FString& Message);

    void SendAudioChunk(const TArray<uint8>& AudioData);

    // 데이터 처리
    FSTTResponse ParseJSONResponse(const FString& JSONString);
    ESTTCommandType ParseCommandString(const FString& CommandString);
    void DispatchKeywords(const FString& Text);
    void BroadcastError(const FString& ErrorMessage);

    void LogDebug(const FString& Message);
    void LogError(const FString& Message);
};