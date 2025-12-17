#include "STTSubsystem.h"
#include "MVE.h"
#include "WebSocketsModule.h"
#include "JsonObjectConverter.h"
#include "Async/Async.h"
#include "Serialization/JsonSerializer.h"

// ------------------------------------ 초기화 ------------------------------------

void USTTSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RegisterKeyword(TEXT("생성"), FGameplayTag::RequestGameplayTag(FName("Command.Spawn")));
    RegisterKeyword(TEXT("삭제"), FGameplayTag::RequestGameplayTag(FName("Command.Delete")));
    RegisterKeyword(TEXT("불켜"), FGameplayTag::RequestGameplayTag(FName("Command.Light")));
    RegisterKeyword(TEXT("아바타"), FGameplayTag::RequestGameplayTag(FName("Command.Avatar")));

    LogDebug(TEXT("STT 서브시스템 초기화 (WebSocket 스트리밍 모드)"));
}

void USTTSubsystem::Deinitialize()
{
    StopTranscribing();
    Super::Deinitialize();
}

// ------------------------------------ 제어 로직 ------------------------------------

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
    LogDebug(TEXT("STT 종료"));
}

void USTTSubsystem::RegisterKeyword(const FString& Keyword, FGameplayTag ActionTag)
{
    if (Keyword.IsEmpty()) return;
    KeywordTagMap.Add(Keyword, ActionTag);
}

// ------------------------------------ WebSocket 처리 ------------------------------------

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
    OnConnectionStatusChanged.Broadcast(true);

    // ========================================
    // 핵심 수정 1: Python 클라이언트처럼 초기 설정 메시지 제거
    // 서버가 JSON 설정을 기대하지 않는 경우 아래 블록 전체를 주석 처리하세요.
    // ========================================
    
    /* 
    // 만약 서버가 초기 설정 JSON을 요구한다면 이 블록 활성화
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("sampleRate"), SampleRate);
    JsonObject->SetNumberField(TEXT("channels"), NumChannels);
    JsonObject->SetStringField(TEXT("encoding"), TEXT("LINEAR16"));
    JsonObject->SetStringField(TEXT("languageCode"), TEXT("ko-KR")); 

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        FTCHARToUTF8 Converter(*OutputString);
        TArray<uint8> ConfigData;
        ConfigData.Append((uint8*)Converter.Get(), Converter.Length());
        WebSocket->Send(ConfigData.GetData(), ConfigData.Num(), true); 
        LogDebug(FString::Printf(TEXT("초기 STT 설정 메시지 전송: %s"), *OutputString));
    }
    */

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

    const int32 DesiredFramesPerChunk = 512; // 512 샘플 = 1024 bytes (int16)
    // 실제 마이크는 WASAPI 기본값(480프레임)으로 들어올 수 있음 

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
    OnConnectionStatusChanged.Broadcast(false);
}

void USTTSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    LogDebug(FString::Printf(TEXT("WebSocket 종료 (코드: %d, 사유: %s)"), StatusCode, *Reason));
    bIsActive = false;
    OnConnectionStatusChanged.Broadcast(false);
    PendingAudioBuffer.Empty();
    
    if (AudioCapture.IsValid())
    {
        AudioCapture->StopStream();
        AudioCapture->CloseStream();
        AudioCapture.Reset();
    }
}

void USTTSubsystem::OnWebSocketMessage(const FString& Message)
{
    FSTTResponse Response = ParseJSONResponse(Message);
    
    if (Response.bSuccess)
    {
        OnSTTTextStream.Broadcast(Response);

        if (!Response.TranscribedText.IsEmpty())
        {
            DispatchKeywords(Response.TranscribedText);
        }
        
        PRINTLOG(TEXT("STT 수신: %s (신뢰도: %.2f, 완료: %s)"), 
            *Response.TranscribedText, 
            Response.Confidence, 
            Response.bIsFinal ? TEXT("O") : TEXT("X"));
    }
    else
    {
        if (!Response.ErrorMessage.IsEmpty())
        {
            LogDebug(FString::Printf(TEXT("서버 메시지: %s"), *Response.ErrorMessage));
        }
    }
}

// ------------------------------------ 오디오 처리 ------------------------------------

// ================================================================================================
// STTSubsystem.cpp 수정본 - OnAudioCapture 함수
// 위치: 라인 236 이후에 추가
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
    
    // ========================================
    // 2단계: 🔥 Cubic (Catmull-Rom) 보간 리샘플링
    // 선형보다 훨씬 좋고, Lanczos보다 가벼움
    // ========================================
    
    TArray<float> ResampledData;
    
    if (InSampleRate != TARGET_SAMPLE_RATE)
    {
        const float ResampleRatio = (float)TARGET_SAMPLE_RATE / (float)InSampleRate;
        const int32 OutputFrames = FMath::FloorToInt(ProcessedFloatData.Num() * ResampleRatio);
        
        ResampledData.Reserve(OutputFrames);
        
        for (int32 OutIdx = 0; OutIdx < OutputFrames; ++OutIdx)
        {
            const float SrcPosFloat = OutIdx / ResampleRatio;
            const int32 SrcIdx1 = FMath::FloorToInt(SrcPosFloat); // 중심 샘플 (왼쪽)
            const float Fraction = SrcPosFloat - SrcIdx1;
            
            // Cubic 보간을 위해 4개 샘플 필요: p0, p1, p2, p3
            const int32 SrcIdx0 = FMath::Max(SrcIdx1 - 1, 0);
            const int32 SrcIdx2 = FMath::Min(SrcIdx1 + 1, ProcessedFloatData.Num() - 1);
            const int32 SrcIdx3 = FMath::Min(SrcIdx1 + 2, ProcessedFloatData.Num() - 1);
            
            const float p0 = ProcessedFloatData[SrcIdx0];
            const float p1 = ProcessedFloatData[SrcIdx1];
            const float p2 = ProcessedFloatData[SrcIdx2];
            const float p3 = ProcessedFloatData[SrcIdx3];
            
            // Catmull-Rom 스플라인 (Cubic Hermite)
            // 음성 신호에 적합: 부드러우면서도 고주파 보존
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
        
        static bool bLoggedResampling = false;
        if (!bLoggedResampling)
        {
            PRINTLOG(TEXT("[🎵 Cubic 리샘플링] Catmull-Rom 보간 사용 (고품질)"));
            PRINTLOG(TEXT("[리샘플링] %d Hz → %d Hz | %d → %d 프레임 (%.1f%% 크기)"), 
                InSampleRate, TARGET_SAMPLE_RATE, 
                ProcessedFloatData.Num(), ResampledData.Num(),
                (ResampledData.Num() * 100.0f) / ProcessedFloatData.Num());
            bLoggedResampling = true;
        }
        
        ProcessedFloatData = MoveTemp(ResampledData);
    }
    
    int32 TotalSamples = ProcessedFloatData.Num();
    
    // 3단계: 볼륨 증폭
    float FinalGain = AudioGain;
    
    if (bUsePeakNormalization && TotalSamples > 0)
    {
        float MaxPeak = 0.0f;
        for (int32 i = 0; i < TotalSamples; ++i)
        {
            MaxPeak = FMath::Max(MaxPeak, FMath::Abs(ProcessedFloatData[i]));
        }
        
        if (MaxPeak > NoiseGateThreshold)
        {
            float PeakGain = 0.9f / MaxPeak;
            PeakGain = FMath::Min(PeakGain, 30.0f);
            FinalGain *= PeakGain;
            
            static bool bLoggedPeak = false;
            if (!bLoggedPeak)
            {
                PRINTLOG(TEXT("[피크정규화] MaxPeak: %.4f → 게인: %.1fx"), MaxPeak, FinalGain);
                bLoggedPeak = true;
            }
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
    
    // ========================================
    // 4단계: float → int16 변환
    // ========================================
    
    TArray<uint8> IncomingPCMData;
    IncomingPCMData.Reserve(TotalSamples * 2);
    
    int16 MaxVolumeInt16 = 0;
    int32 ClippedCount = 0; // 클리핑 카운트
    
    for (int32 i = 0; i < TotalSamples; ++i)
    {
        float Sample = ProcessedFloatData[i] * FinalGain;
        
        // 클리핑 체크
        if (FMath::Abs(Sample) >= 0.99f)
        {
            ClippedCount++;
        }
        
        // 클램핑
        Sample = FMath::Clamp(Sample, -1.0f, 1.0f);
        
        // int16 변환 (32767 사용)
        int16 IntSample = static_cast<int16>(Sample * 32767.0f);
        
        MaxVolumeInt16 = FMath::Max(MaxVolumeInt16, (int16)FMath::Abs(IntSample));
        
        // 리틀 엔디안 바이트 변환
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
    
    // 주기적 로그
    static double LastLogTime = 0.0;
    if (InStreamTime - LastLogTime > 5.0)
    {
        float VolumePercent = (MaxVolumeInt16 / 32767.0f) * 100.0f;
        float ClipPercent = (ClippedCount * 100.0f) / TotalSamples;
        
        PRINTLOG(TEXT("[🎤 상태] 버퍼: %d bytes | 볼륨: %d (%.1f%%) | 클리핑: %.1f%%"), 
            PendingAudioBuffer.Num(), MaxVolumeInt16, VolumePercent, ClipPercent);
        
        // 클리핑 경고
        if (ClipPercent > 5.0f)
        {
            PRINTLOG(TEXT("⚠️ [클리핑 경고] %.1f%% 샘플이 클리핑됨 - AudioGain을 %.1f로 낮추세요"), 
                ClipPercent, AudioGain * 0.7f);
        }
        
        LastLogTime = InStreamTime;
        ClippedCount = 0;
    }
}

void USTTSubsystem::SendAudioChunk(const TArray<uint8>& AudioData)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        // 바이너리 프레임 전송 (true = binary mode)
        WebSocket->Send(AudioData.GetData(), AudioData.Num(), true);
        
        // 디버그: 첫 전송 시 데이터 샘플 및 int16 값 출력
        static bool bFirstSend = true;
        if (bFirstSend && AudioData.Num() >= 8)
        {
            // 바이트 샘플
            PRINTLOG(TEXT("[전송 샘플] 처음 8바이트: %02X %02X %02X %02X %02X %02X %02X %02X"), 
                AudioData[0], AudioData[1], AudioData[2], AudioData[3],
                AudioData[4], AudioData[5], AudioData[6], AudioData[7]);
            
            // int16 값 확인 (리틀 엔디안)
            int16 Sample0 = (int16)(AudioData[0] | (AudioData[1] << 8));
            int16 Sample1 = (int16)(AudioData[2] | (AudioData[3] << 8));
            int16 Sample2 = (int16)(AudioData[4] | (AudioData[5] << 8));
            PRINTLOG(TEXT("[전송 샘플] int16 값: %d, %d, %d (최대: ±32767)"), 
                Sample0, Sample1, Sample2);
            
            bFirstSend = false;
        }
    }
    else
    {
        PRINTLOG(TEXT("[전송 오류] 웹소켓 연결 끊김"));
    }
}

// ------------------------------------ 데이터 파싱 ------------------------------------

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

    if (JsonObject->HasField(TEXT("text")))
        Response.TranscribedText = JsonObject->GetStringField(TEXT("text"));
    else if (JsonObject->HasField(TEXT("transcript")))
        Response.TranscribedText = JsonObject->GetStringField(TEXT("transcript"));
    else if (JsonObject->HasField(TEXT("result")))
        Response.TranscribedText = JsonObject->GetStringField(TEXT("result"));
    else if (JsonObject->HasField(TEXT("partial")))
        Response.TranscribedText = JsonObject->GetStringField(TEXT("partial"));

    if (JsonObject->HasField(TEXT("is_final")))
        Response.bIsFinal = JsonObject->GetBoolField(TEXT("is_final"));
    else if (JsonObject->HasField(TEXT("final")))
        Response.bIsFinal = JsonObject->GetBoolField(TEXT("final"));
    
    if (JsonObject->HasField(TEXT("language")))
        Response.Language = JsonObject->GetStringField(TEXT("language"));
    
    if (JsonObject->HasField(TEXT("confidence")))
        Response.Confidence = JsonObject->GetNumberField(TEXT("confidence"));

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

// ------------------------------------ 유틸리티 ------------------------------------

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