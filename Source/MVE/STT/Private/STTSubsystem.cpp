// STTSubsystem.cpp - 수정된 버전 (Python 서버 응답 형식 매칭)
#include "../Public/STTSubsystem.h"
#include "MVE.h"
#include "WebSocketsModule.h"
#include "JsonObjectConverter.h"
#include "Async/Async.h"
#include "Serialization/JsonSerializer.h"

// ================================================================================================
// 초기화
// ================================================================================================

void USTTSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RegisterKeyword(TEXT("생성"), FGameplayTag::RequestGameplayTag(FName("Command.Spawn")));
    RegisterKeyword(TEXT("삭제"), FGameplayTag::RequestGameplayTag(FName("Command.Delete")));
    RegisterKeyword(TEXT("불켜"), FGameplayTag::RequestGameplayTag(FName("Command.Light")));
    RegisterKeyword(TEXT("아바타"), FGameplayTag::RequestGameplayTag(FName("Command.Avatar")));

    LogDebug(TEXT("STT 서브시스템 초기화 완료 (WebSocket 스트리밍 모드)"));
}

void USTTSubsystem::Deinitialize()
{
    StopTranscribing();
    Super::Deinitialize();
}

// ================================================================================================
// 유틸리티 함수
// ================================================================================================

FString USTTSubsystem::GetCommandDisplayName(ESTTCommandType CommandType)
{
    switch (CommandType)
    {
    case ESTTCommandType::NextTrack:
        return TEXT("다음 곡");
    case ESTTCommandType::StopTrack:
        return TEXT("정지");
    case ESTTCommandType::PlayTrack:
        return TEXT("재생");
    case ESTTCommandType::ConcertOpen:
        return TEXT("콘서트 시작");
    case ESTTCommandType::ConcertClose:
        return TEXT("콘서트 끝");
    default:
        return TEXT("없음");
    }
}

ESTTCommandType USTTSubsystem::ParseCommandString(const FString& CommandString)
{
    // Python 서버의 parse_command() 반환값과 매칭
    if (CommandString == TEXT("NextTrack"))
    {
        return ESTTCommandType::NextTrack;
    }
    else if (CommandString == TEXT("StopTrack"))
    {
        return ESTTCommandType::StopTrack;
    }
    else if (CommandString == TEXT("PlayTrack"))
    {
        return ESTTCommandType::PlayTrack;
    }
    else if (CommandString == TEXT("ConcertOpen"))
    {
            return ESTTCommandType::ConcertOpen;
    }
    else if (CommandString == TEXT("ConcertClose"))
    {
        return ESTTCommandType::ConcertClose;
    }
    return ESTTCommandType::None;
}

// ================================================================================================
// 제어 로직
// ================================================================================================

void USTTSubsystem::StartTranscribing()
{
    if (bIsActive)
    {
        LogDebug(TEXT("이미 STT가 실행 중입니다."));
        return;
    }
    ConnectWebSocket();
}

void USTTSubsystem::StopTranscribing()
{
    if (AudioCapture.IsValid())
    {
        if (AudioCapture->IsStreamOpen())
        {
            AudioCapture->StopStream();
            AudioCapture->CloseStream();
        }
        AudioCapture.Reset();
    }

    CloseWebSocket();
    PendingAudioBuffer.Empty();

    bIsActive = false;
    bIsConnected = false;
    LogDebug(TEXT("STT 종료"));
}

void USTTSubsystem::RegisterKeyword(const FString& Keyword, FGameplayTag ActionTag)
{
    if (Keyword.IsEmpty()) return;
    KeywordTagMap.Add(Keyword, ActionTag);
}

// ================================================================================================
// WebSocket 처리
// ================================================================================================

void USTTSubsystem::ConnectWebSocket()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        OnWebSocketConnected();
        return;
    }

    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    LogDebug(FString::Printf(TEXT("서버 연결 시도: %s"), *WebSocketURL));
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(WebSocketURL, TEXT("ws"));

    WebSocket->OnConnected().AddUObject(this, &USTTSubsystem::OnWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &USTTSubsystem::OnWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &USTTSubsystem::OnWebSocketClosed);
    WebSocket->OnMessage().AddUObject(this, &USTTSubsystem::OnWebSocketMessage);

    WebSocket->Connect();
}

void USTTSubsystem::CloseWebSocket()
{
    if (WebSocket.IsValid())
    {
        WebSocket->Close();
        WebSocket.Reset();
    }
}

void USTTSubsystem::OnWebSocketConnected()
{
    LogDebug(TEXT("WebSocket 서버 연결 성공"));
    bIsConnected = true;
    OnConnectionStatusChanged.Broadcast(true);

    PendingAudioBuffer.Empty();

    if (!AudioCapture.IsValid())
    {
        AudioCapture = MakeUnique<Audio::FAudioCapture>();
    }

    Audio::FAudioCaptureDeviceParams Params;
    Params.DeviceIndex = 0;

    Audio::FOnAudioCaptureFunction CaptureCallback = [this](
        const void* InAudioData,
        int32 InNumFrames,
        int32 InNumChannels,
        int32 InSampleRate,
        double InStreamTime,
        bool bInOverflow)
    {
        OnAudioCapture(
            static_cast<const float*>(InAudioData),
            InNumFrames,
            InNumChannels,
            InSampleRate,
            InStreamTime,
            bInOverflow);
    };

    const int32 DesiredFramesPerChunk = 512;

    LogDebug(FString::Printf(TEXT("마이크 스트림 열기 (프레임 청크: %d)"), DesiredFramesPerChunk));

    if (AudioCapture->OpenAudioCaptureStream(Params, CaptureCallback, DesiredFramesPerChunk))
    {
        if (AudioCapture->StartStream())
        {
            bIsActive = true;
            LogDebug(TEXT("마이크 스트림 시작 성공"));
        }
        else
        {
            BroadcastError(TEXT("마이크 스트림 시작 실패"));
            CloseWebSocket();
        }
    }
    else
    {
        BroadcastError(TEXT("마이크 디바이스 열기 실패 - 권한 확인 필요"));
        CloseWebSocket();
    }
}

void USTTSubsystem::OnWebSocketConnectionError(const FString& Error)
{
    LogError(FString::Printf(TEXT("WebSocket 연결 오류: %s"), *Error));
    BroadcastError(TEXT("서버 연결 실패"));
    bIsActive = false;
    bIsConnected = false;
    OnConnectionStatusChanged.Broadcast(false);
}

void USTTSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    LogDebug(FString::Printf(TEXT("WebSocket 종료 (코드: %d, 사유: %s)"), StatusCode, *Reason));
    bIsActive = false;
    bIsConnected = false;
    OnConnectionStatusChanged.Broadcast(false);
    PendingAudioBuffer.Empty();

    if (AudioCapture.IsValid())
    {
        AudioCapture->StopStream();
        AudioCapture->CloseStream();
        AudioCapture.Reset();
    }
}

// ================================================================================================
// 🔥 핵심 수정: 서버 응답 처리 (Python 서버 형식 매칭)
// ================================================================================================

void USTTSubsystem::OnWebSocketMessage(const FString& Message)
{
    LogDebug(FString::Printf(TEXT("서버 응답 수신: %s"), *Message));

    FSTTResponse Response = ParseJSONResponse(Message);

    if (Response.bSuccess)
    {
        // 상태 업데이트 (UI용)
        LastReceivedText = Response.TranscribedText;
        LastCommandType = Response.CommandType;

        // 텍스트 스트림 이벤트 발송
        OnSTTTextStream.Broadcast(Response);

        // 🔥 명령어인 경우 별도 이벤트 발송
        if (Response.bIsCommand && Response.CommandType != ESTTCommandType::None)
        {
            PRINTLOG(TEXT("🎯 명령어 감지: [%s] -> %s"),
                *Response.TranscribedText,
                *GetCommandDisplayName(Response.CommandType));

            OnSTTCommandReceived.Broadcast(Response.CommandType, Response.TranscribedText);
        }

        // 키워드 매칭 (기존 로직 유지)
        if (!Response.TranscribedText.IsEmpty())
        {
            DispatchKeywords(Response.TranscribedText);
        }

        PRINTLOG(TEXT("📝 STT 수신: %s (명령어: %s)"),
            *Response.TranscribedText,
            Response.bIsCommand ? *Response.CommandString : TEXT("없음"));
    }
    else
    {
        if (!Response.ErrorMessage.IsEmpty())
        {
            LogDebug(FString::Printf(TEXT("서버 메시지: %s"), *Response.ErrorMessage));
        }
    }
}

// ================================================================================================
// 🔥 핵심 수정: JSON 파싱 (Python 서버 응답 형식)
// Python 서버 응답 형식:
// {
//     "content": "인식된 텍스트",
//     "command": "NextTrack" | "StopTrack" | "PlayTrack",
//     "is_command": true
// }
// ================================================================================================

FSTTResponse USTTSubsystem::ParseJSONResponse(const FString& JSONString)
{
    FSTTResponse Response;
    Response.bSuccess = false;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        LogError(FString::Printf(TEXT("JSON 파싱 실패: %s"), *JSONString));
        return Response;
    }

    // 🔥 Python 서버 응답 필드 매칭
    // 1. content 필드 (인식된 텍스트)
    if (JsonObject->HasField(TEXT("content")))
    {
        Response.TranscribedText = JsonObject->GetStringField(TEXT("content"));
    }
    // 폴백: 다른 가능한 필드명들
    else if (JsonObject->HasField(TEXT("text")))
    {
        Response.TranscribedText = JsonObject->GetStringField(TEXT("text"));
    }
    else if (JsonObject->HasField(TEXT("transcript")))
    {
        Response.TranscribedText = JsonObject->GetStringField(TEXT("transcript"));
    }

    // 2. command 필드 (명령어 코드)
    if (JsonObject->HasField(TEXT("command")))
    {
        Response.CommandString = JsonObject->GetStringField(TEXT("command"));
        Response.CommandType = ParseCommandString(Response.CommandString);
    }

    // 3. is_command 필드 (명령어 여부)
    if (JsonObject->HasField(TEXT("is_command")))
    {
        Response.bIsCommand = JsonObject->GetBoolField(TEXT("is_command"));
    }

    // 4. 기타 필드들 (호환성 유지)
    if (JsonObject->HasField(TEXT("is_final")))
    {
        Response.bIsFinal = JsonObject->GetBoolField(TEXT("is_final"));
    }
    else if (JsonObject->HasField(TEXT("final")))
    {
        Response.bIsFinal = JsonObject->GetBoolField(TEXT("final"));
    }

    if (JsonObject->HasField(TEXT("language")))
    {
        Response.Language = JsonObject->GetStringField(TEXT("language"));
    }

    if (JsonObject->HasField(TEXT("confidence")))
    {
        Response.Confidence = JsonObject->GetNumberField(TEXT("confidence"));
    }

    // 에러 처리
    if (JsonObject->HasField(TEXT("error")))
    {
        Response.ErrorMessage = JsonObject->GetStringField(TEXT("error"));
        Response.bSuccess = false;
    }
    else
    {
        Response.bSuccess = !Response.TranscribedText.IsEmpty();
    }

    return Response;
}

// ================================================================================================
// 오디오 처리 (기존 로직 유지)
// ================================================================================================

void USTTSubsystem::OnAudioCapture(
    const float* InAudioData,
    int32 InNumFrames,
    int32 InNumChannels,
    int32 InSampleRate,
    double InStreamTime,
    bool bInOverflow)
{
    if (!WebSocket.IsValid() || !WebSocket->IsConnected())
    {
        return;
    }

    const int32 TARGET_SAMPLE_RATE = 16000;

    TArray<float> ProcessedFloatData;

    // 1단계: 모노 변환
    if (InNumChannels > 1)
    {
        ProcessedFloatData.Reserve(InNumFrames);
        for (int32 i = 0; i < InNumFrames; ++i)
        {
            float MonoSample = 0.0f;
            for (int32 ch = 0; ch < InNumChannels; ++ch)
            {
                MonoSample += InAudioData[i * InNumChannels + ch];
            }
            ProcessedFloatData.Add(MonoSample / InNumChannels);
        }
    }
    else
    {
        ProcessedFloatData.Append(InAudioData, InNumFrames);
    }

    // 2단계: Cubic 보간 리샘플링
    TArray<float> ResampledData;

    if (InSampleRate != TARGET_SAMPLE_RATE)
    {
        const float ResampleRatio = (float)TARGET_SAMPLE_RATE / (float)InSampleRate;
        const int32 OutputFrames = FMath::FloorToInt(ProcessedFloatData.Num() * ResampleRatio);

        ResampledData.Reserve(OutputFrames);

        for (int32 OutIdx = 0; OutIdx < OutputFrames; ++OutIdx)
        {
            const float SrcPosFloat = OutIdx / ResampleRatio;
            const int32 SrcIdx1 = FMath::FloorToInt(SrcPosFloat);
            const float Fraction = SrcPosFloat - SrcIdx1;

            const int32 SrcIdx0 = FMath::Max(SrcIdx1 - 1, 0);
            const int32 SrcIdx2 = FMath::Min(SrcIdx1 + 1, ProcessedFloatData.Num() - 1);
            const int32 SrcIdx3 = FMath::Min(SrcIdx1 + 2, ProcessedFloatData.Num() - 1);

            const float p0 = ProcessedFloatData[SrcIdx0];
            const float p1 = ProcessedFloatData[SrcIdx1];
            const float p2 = ProcessedFloatData[SrcIdx2];
            const float p3 = ProcessedFloatData[SrcIdx3];

            const float t = Fraction;
            const float t2 = t * t;
            const float t3 = t2 * t;

            const float a0 = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
            const float a1 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
            const float a2 = -0.5f * p0 + 0.5f * p2;
            const float a3 = p1;

            const float Sample = a0 * t3 + a1 * t2 + a2 * t + a3;

            ResampledData.Add(Sample);
        }

        ProcessedFloatData = MoveTemp(ResampledData);
    }

    int32 TotalSamples = ProcessedFloatData.Num();

    // 🔥 오디오 레벨 계산 (UI용)
    float MaxPeak = 0.0f;
    for (int32 i = 0; i < TotalSamples; ++i)
    {
        MaxPeak = FMath::Max(MaxPeak, FMath::Abs(ProcessedFloatData[i]));
    }
    CurrentAudioLevel = MaxPeak;

    // 게임 스레드에서 오디오 레벨 이벤트 발송
    AsyncTask(ENamedThreads::GameThread, [this, MaxPeak]()
    {
        OnAudioLevelChanged.Broadcast(MaxPeak);
    });

    // 3단계: 볼륨 증폭
    float FinalGain = AudioGain;

    if (bUsePeakNormalization && TotalSamples > 0)
    {
        if (MaxPeak > NoiseGateThreshold)
        {
            float PeakGain = 0.9f / MaxPeak;
            PeakGain = FMath::Min(PeakGain, 30.0f);
            FinalGain *= PeakGain;
        }
    }
    else if (bUseAutoNormalization && TotalSamples > 0)
    {
        float SumSquares = 0.0f;
        for (int32 i = 0; i < TotalSamples; ++i)
        {
            SumSquares += ProcessedFloatData[i] * ProcessedFloatData[i];
        }
        float CurrentRMS = FMath::Sqrt(SumSquares / TotalSamples);

        if (CurrentRMS > (NoiseGateThreshold * 5.0f))
        {
            float AutoGain = TargetRMS / CurrentRMS;
            AutoGain = FMath::Clamp(AutoGain, 1.0f, 30.0f);
            FinalGain *= AutoGain;
        }
    }

    // 4단계: float -> int16 변환
    TArray<uint8> IncomingPCMData;
    IncomingPCMData.Reserve(TotalSamples * 2);

    int16 MaxVolumeInt16 = 0;

    for (int32 i = 0; i < TotalSamples; ++i)
    {
        float Sample = ProcessedFloatData[i] * FinalGain;
        Sample = FMath::Clamp(Sample, -1.0f, 1.0f);

        int16 IntSample = static_cast<int16>(Sample * 32767.0f);
        MaxVolumeInt16 = FMath::Max(MaxVolumeInt16, (int16)FMath::Abs(IntSample));

        uint8 LowByte = static_cast<uint8>(IntSample & 0xFF);
        uint8 HighByte = static_cast<uint8>((IntSample >> 8) & 0xFF);

        IncomingPCMData.Add(LowByte);
        IncomingPCMData.Add(HighByte);
    }

    PendingAudioBuffer.Append(IncomingPCMData);

    // 5단계: 1024 바이트 청크 전송
    const int32 DesiredChunkSize = 1024;

    while (PendingAudioBuffer.Num() >= DesiredChunkSize)
    {
        TArray<uint8> ChunkToSend;
        ChunkToSend.Append(PendingAudioBuffer.GetData(), DesiredChunkSize);

        SendAudioChunk(ChunkToSend);

        PendingAudioBuffer.RemoveAt(0, DesiredChunkSize);
    }
}

void USTTSubsystem::SendAudioChunk(const TArray<uint8>& AudioData)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Send(AudioData.GetData(), AudioData.Num(), true);
    }
}

// ================================================================================================
// 키워드 디스패치
// ================================================================================================

void USTTSubsystem::DispatchKeywords(const FString& Text)
{
    if (Text.IsEmpty()) return;

    FString LowerText = Text.ToLower();

    for (const auto& Pair : KeywordTagMap)
    {
        FString LowerKeyword = Pair.Key.ToLower();
        if (LowerText.Contains(LowerKeyword))
        {
            PRINTLOG(TEXT("키워드 감지: '%s' -> %s"), *Pair.Key, *Pair.Value.ToString());
            OnKeywordDetected.Broadcast(Pair.Value, Text);
        }
    }
}

// ================================================================================================
// 유틸리티
// ================================================================================================

void USTTSubsystem::BroadcastError(const FString& ErrorMessage)
{
    LogError(ErrorMessage);
    OnSTTError.Broadcast(ErrorMessage);
}

void USTTSubsystem::LogDebug(const FString& Message)
{
    PRINTLOG(TEXT("[STT] %s"), *Message);
}

void USTTSubsystem::LogError(const FString& Message)
{
    PRINTLOG(TEXT("[STT ERROR] %s"), *Message);
}