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

    // ========================================
    // 스테레오 → 모노 변환 (서버는 모노 채널 기대)
    // ========================================
    
    TArray<float> MonoData;
    if (InNumChannels == 2)
    {
        // 스테레오를 모노로: (L + R) / 2
        MonoData.Reserve(InNumFrames);
        for (int32 i = 0; i < InNumFrames; ++i)
        {
            float L = InAudioData[i * 2];
            float R = InAudioData[i * 2 + 1];
            MonoData.Add((L + R) * 0.5f);
        }
        InAudioData = MonoData.GetData();
        InNumChannels = 1;
        
        static bool bLoggedMono = false;
        if (!bLoggedMono)
        {
            PRINTLOG(TEXT("[채널] 스테레오 → 모노 변환 완료 (%d 프레임)"), InNumFrames);
            bLoggedMono = true;
        }
    }
    
    // ========================================
    // 핵심 수정 2: 개선된 다운샘플링 로직
    // ========================================
    
    TArray<float> ProcessedFloatData;
    int32 TotalSamples = InNumFrames * InNumChannels;
    
    // 실제 입력 샘플레이트 로그 (디버그용)
    static bool bLoggedOnce = false;
    if (!bLoggedOnce)
    {
        PRINTLOG(TEXT("[오디오] 입력 SR: %d Hz, 목표 SR: %d Hz, 채널: %d, 프레임: %d, 게인: %.1fx"), 
            InSampleRate, SampleRate, InNumChannels, InNumFrames, AudioGain);
        bLoggedOnce = true;
    }

    // 다운샘플링 처리
    if (InSampleRate > SampleRate)
    {
        // 정수배가 아니어도 처리 (48000 -> 16000은 3배)
        const float DownsampleRatio = static_cast<float>(InSampleRate) / static_cast<float>(SampleRate);
        const int32 ExpectedOutputSamples = FMath::CeilToInt(TotalSamples / DownsampleRatio);
        
        ProcessedFloatData.Reserve(ExpectedOutputSamples);

        // 출력 샘플 기준으로 순회
        for (int32 OutIdx = 0; OutIdx < ExpectedOutputSamples; OutIdx++)
        {
            // 소스 인덱스 계산 (반올림)
            int32 SrcIdx = FMath::RoundToInt(OutIdx * DownsampleRatio);
            if (SrcIdx < TotalSamples)
            {
                ProcessedFloatData.Add(InAudioData[SrcIdx]);
            }
        }
        
        TotalSamples = ProcessedFloatData.Num();
        PRINTLOG(TEXT("[다운샘플링] %d -> %d 샘플 (비율: %.2f)"), 
            InNumFrames * InNumChannels, TotalSamples, DownsampleRatio);
    }
    else if (InSampleRate == SampleRate)
    {
        // 샘플레이트 일치 - 복사만
        ProcessedFloatData.Append(InAudioData, TotalSamples);
    }
    else
    {
        // 업샘플링은 지원 안 함
        LogError(FString::Printf(TEXT("마이크 샘플레이트(%d)가 목표(%d)보다 낮음 - 업샘플링 미지원"), 
            InSampleRate, SampleRate));
        return;
    }
    
    // ========================================
    // RMS 기반 자동 정규화 (VAD 최적화)
    // ========================================
    
    float FinalGain = AudioGain;
    
    // 옵션 1: 피크 정규화 (가장 강력, VAD 극대화)
    if (bUsePeakNormalization && TotalSamples > 0)
    {
        float MaxPeak = 0.0f;
        for (int32 i = 0; i < TotalSamples; ++i)
        {
            MaxPeak = FMath::Max(MaxPeak, FMath::Abs(ProcessedFloatData[i]));
        }
        
        // 노이즈 게이트: 너무 작은 신호는 증폭하지 않음 (0.05 = 5%)
        if (MaxPeak > 0.05f)
        {
            // 피크를 0.9로 맞춤 (클리핑 여유)
            float PeakGain = 0.9f / MaxPeak;
            // 과도한 증폭 방지 (최대 10배)
            PeakGain = FMath::Min(PeakGain, 10.0f);
            FinalGain *= PeakGain;
            
            static bool bLoggedPeak = false;
            if (!bLoggedPeak)
            {
                int32 ExpectedMaxInt16 = (int32)(0.9f * 32767.0f);
                PRINTLOG(TEXT("[피크정규화] 최대 피크: %.4f, 피크 게인: %.2fx, 최종: %.2fx → 예상 MaxVolume: ~%d"), 
                    MaxPeak, PeakGain, FinalGain, ExpectedMaxInt16);
                bLoggedPeak = true;
            }
        }
        else
        {
            // 노이즈만 있음 - 실제 마이크 입력 없음
            static int32 NoiseCount = 0;
            NoiseCount++;
            if (NoiseCount % 50 == 1)
            {
                PRINTLOG(TEXT("[마이크 확인 필요] 신호가 너무 작음 (피크: %.5f < 0.05) - Windows에서 마이크 테스트 필요!"), MaxPeak);
            }
        }
    }
    // 옵션 2: RMS 정규화 (균형잡힌 증폭)
    else if (bUseAutoNormalization && TotalSamples > 0)
    {
        // RMS 계산
        float SumSquares = 0.0f;
        for (int32 i = 0; i < TotalSamples; ++i)
        {
            SumSquares += ProcessedFloatData[i] * ProcessedFloatData[i];
        }
        float CurrentRMS = FMath::Sqrt(SumSquares / TotalSamples);
        
        // 노이즈 게이트: 너무 작은 RMS는 증폭하지 않음 (0.02 이상)
        if (CurrentRMS > 0.02f)
        {
            float AutoGain = TargetRMS / CurrentRMS;
            // 과도한 증폭 방지 (최대 10배)
            AutoGain = FMath::Clamp(AutoGain, 1.0f, 10.0f);
            FinalGain *= AutoGain;
            
            static bool bLoggedRMS = false;
            if (!bLoggedRMS)
            {
                PRINTLOG(TEXT("[RMS정규화] 현재 RMS: %.4f, 목표: %.2f, 자동 게인: %.2fx, 최종: %.2fx"), 
                    CurrentRMS, TargetRMS, AutoGain, FinalGain);
                bLoggedRMS = true;
            }
        }
        else
        {
            // 노이즈만 있음
            static int32 NoiseCount = 0;
            NoiseCount++;
            if (NoiseCount % 50 == 1)
            {
                PRINTLOG(TEXT("[마이크 확인 필요] RMS가 너무 작음 (%.5f < 0.02) - Windows에서 마이크 테스트 필요!"), CurrentRMS);
            }
        }
    }
    
    // ========================================
    // 핵심 수정 3: 명시적 리틀 엔디안 int16 변환
    // ========================================
    
    TArray<uint8> IncomingPCMData;
    IncomingPCMData.Reserve(TotalSamples * 2); // int16 = 2 bytes

    int16 MaxVolume = 0; // 서버의 mx 값과 동일한 체크
    
    for (int32 i = 0; i < TotalSamples; ++i)
    {
        // 최종 게인 적용 후 클리핑
        float Sample = FMath::Clamp(ProcessedFloatData[i] * FinalGain, -1.0f, 1.0f);
        int16 IntSample = static_cast<int16>(Sample * 32767.0f); 
        
        // 최대 볼륨 트래킹 (서버의 np.max(np.abs(audio_int16))와 동일)
        MaxVolume = FMath::Max(MaxVolume, (int16)FMath::Abs(IntSample));
        
        // 리틀 엔디안 명시적 변환 (Python의 int16 tobytes()와 동일)
        uint8 LowByte = static_cast<uint8>(IntSample & 0xFF);
        uint8 HighByte = static_cast<uint8>((IntSample >> 8) & 0xFF);
        
        IncomingPCMData.Add(LowByte);
        IncomingPCMData.Add(HighByte);
    }

    PendingAudioBuffer.Append(IncomingPCMData);

    // ========================================
    // 1024 바이트 청크 분할 및 전송
    // 서버 요구사항:
    // - VAD_BYTE_SIZE = AUDIO_CHUNK_SIZE * 2 = 512 * 2 = 1024 bytes
    // - 볼륨 체크: mx = np.max(np.abs(audio_int16))
    // - VAD Threshold로 음성 감지
    // ========================================
    
    const int32 DesiredChunkSize = 1024; // 서버: VAD_BYTE_SIZE = AUDIO_CHUNK_SIZE(512) * 2

    while (PendingAudioBuffer.Num() >= DesiredChunkSize)
    {
        TArray<uint8> ChunkToSend; 
        ChunkToSend.Append(PendingAudioBuffer.GetData(), DesiredChunkSize);

        SendAudioChunk(ChunkToSend);

        PendingAudioBuffer.RemoveAt(0, DesiredChunkSize);
    }
    
    // 주기적 로그 (5초마다)
    static double LastLogTime = 0.0;
    static int32 LogCounter = 0;
    if (InStreamTime - LastLogTime > 5.0)
    {
        PRINTLOG(TEXT("[버퍼] 잔류: %d bytes | 볼륨(MaxVolume): %d / 32767 (%.1f%%)"), 
            PendingAudioBuffer.Num(), MaxVolume, (MaxVolume / 32767.0f) * 100.0f);
        LastLogTime = InStreamTime;
        LogCounter++;
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