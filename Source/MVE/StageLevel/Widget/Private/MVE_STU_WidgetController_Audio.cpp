
#include "../Public/MVE_STU_WidgetController_Audio.h"

#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_STD_WC_AudioPlayer.h"
#include "MVE_STD_WC_AudioSearch.h"
#include "StageLevel/Default/Public/MVE_GM_StageLevel.h"

void UMVE_STU_WidgetController_Audio::Initialize(UMVE_STD_WC_AudioPlayer* InPlayerWidget)
{
    PlayerWidget = InPlayerWidget;
    
    if (!PlayerWidget)
    {
        PRINTNETLOG(this, TEXT("Failed to initialize AudioController: PlayerWidget is null"));
    }
}

void UMVE_STU_WidgetController_Audio::OnTrackSelected(const FMVE_STD_AudioSearchResultData& AudioData)
{
    PRINTNETLOG(this, TEXT("Track selected: %s"), *AudioData.Title);
    
    // 현재 트랙 정보 저장
    CurrentTrackData = AudioData;
    
    // Player UI 설정
    SetupPlayerUI(AudioData);
    
    // Presigned URL 로드 및 브로드캐스트
    LoadPresignedUrlAndBroadcast(AudioData);
}

void UMVE_STU_WidgetController_Audio::OnPlayRequested()
{
    PRINTNETLOG(this, TEXT("Play requested for track: %s"), *CurrentTrackData.Title);
    SendPlayCommandToClients();
}

void UMVE_STU_WidgetController_Audio::OnStopRequested()
{
    PRINTNETLOG(this, TEXT("Stop requested"));
    SendStopCommandToClients();
}

void UMVE_STU_WidgetController_Audio::LoadPresignedUrlAndBroadcast(const FMVE_STD_AudioSearchResultData& AudioData)
{
    PRINTNETLOG(this, TEXT("Requesting Presigned URL for Title: %s"), *AudioData.Title);
    
    FOnStreamAudioComplete OnResult;
    OnResult.BindLambda([this](const bool bSuccess, const FStreamAudioResponseData& ResponseData, const FString& ErrorCode)
    {
        if (bSuccess)
        {
            const FString PresignedUrl = ResponseData.StreamUrl;
            PRINTNETLOG(this, TEXT("Got PresignedUrl = {%s}, sending to all clients"), *PresignedUrl);

            // GameMode를 통해 모든 클라이언트에 URL 전송
            if (UWorld* World = GetWorld())
            {
                if (AMVE_GM_StageLevel* GameMode = World->GetAuthGameMode<AMVE_GM_StageLevel>())
                {
                    GameMode->SendPresignedUrlToAllClients(PresignedUrl);
                }
                else
                {
                    PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode"));
                }
            }
        }
        else
        {
            PRINTNETLOG(this, TEXT("Failed to get Presigned URL. Error: %s"), *ErrorCode);
        }
    });
    
    UMVE_API_Helper::StreamAudio(AudioData.Id, OnResult);
}

void UMVE_STU_WidgetController_Audio::SendPlayCommandToClients()
{
    PRINTNETLOG(this, TEXT("Sending play command to all clients"));
    
    if (UWorld* World = GetWorld())
    {
        if (AMVE_GM_StageLevel* GameMode = World->GetAuthGameMode<AMVE_GM_StageLevel>())
        {
            GameMode->SendPlayCommandToAllClients();
        }
        else
        {
            PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode"));
        }
    }
}

void UMVE_STU_WidgetController_Audio::SendStopCommandToClients()
{
    PRINTNETLOG(this, TEXT("Sending stop command to all clients"));
    
    if (UWorld* World = GetWorld())
    {
        if (AMVE_GM_StageLevel* GameMode = World->GetAuthGameMode<AMVE_GM_StageLevel>())
        {
            GameMode->SendStopCommandToAllClients();
        }
        else
        {
            PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode"));
        }
    }
}

void UMVE_STU_WidgetController_Audio::SetupPlayerUI(const FMVE_STD_AudioSearchResultData& AudioData)
{
    if (!PlayerWidget)
    {
        PRINTNETLOG(this, TEXT("Cannot setup Player UI: PlayerWidget is null"));
        return;
    }
    
    // AudioPlayer에 데이터 설정
    PlayerWidget->SetAudioData(AudioData);
    
    // 델리게이트 초기화 및 재등록
    PlayerWidget->OnPlayClicked.Clear();
    PlayerWidget->OnPlayClicked.AddDynamic(this, &UMVE_STU_WidgetController_Audio::OnPlayRequested);
    
    PlayerWidget->OnStopClicked.Clear();
    PlayerWidget->OnStopClicked.AddDynamic(this, &UMVE_STU_WidgetController_Audio::OnStopRequested);
    
    // UI 초기화
    PlayerWidget->ResetPlaybackUI();
    PlayerWidget->UpdatePlayPauseButton(false); // Paused state
}
