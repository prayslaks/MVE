
#include "../Public/MVE_STU_WidgetController_StudioConcert.h"
#include "MVE.h"
#include "MVE_API_Helper.h"
#include "../Public/MVE_STD_WC_AudioPlayer.h"
#include "../Public/MVE_STD_WC_AudioSearch.h"
#include "StageLevel/Default/Public/MVE_GM_StageLevel.h"
#include "STT/Public/STTSubsystem.h"
#include "UI/Widget/Studio/Public/MVE_STD_WC_AudioSearchResult.h"


void UMVE_STU_WidgetController_StudioConcert::Initialize(UMVE_STD_WC_AudioPlayer* InPlayerWidget, UMVE_STD_WC_AudioSearch* InAudioSearch)
{
    PlayerWidget = InPlayerWidget;
    AudioSearch = InAudioSearch;

    if (!PlayerWidget)
    {
        PRINTNETLOG(this, TEXT("Failed to initialize AudioController: PlayerWidget is null"));
    }

    if (!AudioSearch)
    {
        PRINTNETLOG(this, TEXT("Failed to initialize AudioController: AudioSearch is null"));
    }

    if (USTTSubsystem* STTSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USTTSubsystem>())
    {
        STTSubsystem->OnSTTCommandReceived.AddDynamic(this, &UMVE_STU_WidgetController_StudioConcert::HandleVoiceCommand);
        PRINTNETLOG(this, TEXT("STT 음성 명령 바인딩 완료"));
    }
}

void UMVE_STU_WidgetController_StudioConcert::OnTrackSelected(const FMVE_STD_AudioSearchResultData& AudioData)
{
    PRINTNETLOG(this, TEXT("Track selected: %s"), *AudioData.Title);
    
    // 현재 트랙 정보 저장
    CurrentTrackData = AudioData;
    
    // Player UI 설정
    SetupPlayerUI(AudioData);
    
    // Presigned URL 로드 및 브로드캐스트
    LoadPresignedUrlAndBroadcast(AudioData);
}

void UMVE_STU_WidgetController_StudioConcert::OnPlayRequested()
{
    PRINTNETLOG(this, TEXT("Play requested for track: %s"), *CurrentTrackData.Title);
    SendPlayCommandToClients();
}

void UMVE_STU_WidgetController_StudioConcert::OnStopRequested()
{
    PRINTNETLOG(this, TEXT("Stop requested"));
    SendStopCommandToClients();
}

void UMVE_STU_WidgetController_StudioConcert::SetCurrentTrackData(const FMVE_STD_AudioSearchResultData& Data)
{
    CurrentTrackData = Data;
}

void UMVE_STU_WidgetController_StudioConcert::LoadPresignedUrlAndBroadcast(const FMVE_STD_AudioSearchResultData& AudioData)
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

                    // 🔥 자동 재생 플래그가 설정되어 있으면 재생 명령 전송
                    if (bAutoPlayAfterSelection)
                    {
                        PRINTNETLOG(this, TEXT("🎵 자동 재생: URL 로드 완료, 재생 명령 전송"));
                        bAutoPlayAfterSelection = false; // 플래그 리셋
                        SendPlayCommandToClients();
                    }
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
            bAutoPlayAfterSelection = false; // 실패 시 플래그 리셋
        }
    });

    UMVE_API_Helper::StreamAudio(AudioData.Id, OnResult);
}

void UMVE_STU_WidgetController_StudioConcert::SendPlayCommandToClients()
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

void UMVE_STU_WidgetController_StudioConcert::SendStopCommandToClients()
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

void UMVE_STU_WidgetController_StudioConcert::SetupPlayerUI(const FMVE_STD_AudioSearchResultData& AudioData)
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
    PlayerWidget->OnPlayClicked.AddDynamic(this, &UMVE_STU_WidgetController_StudioConcert::OnPlayRequested);
    
    PlayerWidget->OnStopClicked.Clear();
    PlayerWidget->OnStopClicked.AddDynamic(this, &UMVE_STU_WidgetController_StudioConcert::OnStopRequested);
    
    // UI 초기화
    PlayerWidget->ResetPlaybackUI();
    PlayerWidget->UpdatePlayPauseButton(false); // Paused state
}

int32 UMVE_STU_WidgetController_StudioConcert::FindTrackIndex(const FMVE_STD_AudioSearchResultData& TrackData) const
{
    if (!AudioSearch || AudioSearch->SearchResultWidgets.Num() == 0)
    {
        return -1;
    }

    // Id로 비교해서 현재 트랙 찾기
    for (int32 i = 0; i < AudioSearch->SearchResultWidgets.Num(); i++)
    {
        if (AudioSearch->SearchResultWidgets[i])
        {
            FMVE_STD_AudioSearchResultData Data = AudioSearch->SearchResultWidgets[i]->GetAudioData();
            if (Data.Id == TrackData.Id)
            {
                return i;  // 찾았음!
            }
        }
    }

    return -1;  // 못 찾음
}

void UMVE_STU_WidgetController_StudioConcert::OnNextTrackRequested(bool bAutoPlay)
{
    PRINTNETLOG(this, TEXT("⏭️ 다음 곡 요청"));

    // 1. 검색 결과 유효성 체크
    if (!AudioSearch || AudioSearch->SearchResultWidgets.Num() == 0)
    {
        PRINTNETLOG(this, TEXT("❌ 재생 목록이 비어있습니다."));
        return;
    }

    // 2. 현재 트랙 인덱스 찾기
    int32 CurrentIndex = FindTrackIndex(CurrentTrackData);

    // 3. 현재 트랙을 못 찾았으면 첫 번째 트랙 선택
    if (CurrentIndex == -1)
    {
        PRINTNETLOG(this, TEXT("⚠️ 현재 트랙을 찾을 수 없음. 첫 번째 트랙 선택"));
        CurrentIndex = -1;  // 다음 계산에서 0이 됨
    }

    // 4. 다음 인덱스 계산
    int32 NextIndex = CurrentIndex + 1;
    int32 TotalTracks = AudioSearch->SearchResultWidgets.Num();

    // 5. 경계 체크 (순환 재생)
    if (NextIndex >= TotalTracks)
    {
        PRINTNETLOG(this, TEXT("🔁 마지막 곡. 첫 번째 곡으로 순환"));
        NextIndex = 0;
    }

    // 6. 다음 트랙 가져오기
    UMVE_STD_WC_AudioSearchResult* NextTrackWidget = AudioSearch->SearchResultWidgets[NextIndex];
    if (!NextTrackWidget)
    {
        PRINTNETLOG(this, TEXT("❌ 다음 트랙 위젯이 null입니다."));
        return;
    }

    FMVE_STD_AudioSearchResultData NextTrackData = NextTrackWidget->GetAudioData();
    PRINTNETLOG(this, TEXT("✅ 다음 곡: %s (인덱스: %d/%d)"), *NextTrackData.Title, NextIndex + 1, TotalTracks);

    // 7. 자동 재생 플래그 설정
    if (bAutoPlay)
    {
        bAutoPlayAfterSelection = true;
    }

    // 8. 트랙 선택 (기존 함수 재사용!)
    OnTrackSelected(NextTrackData);
}

void UMVE_STU_WidgetController_StudioConcert::OnPreviousTrackRequested(bool bAutoPlay)
{
    PRINTNETLOG(this, TEXT("⏮️ 이전 곡 요청"));

    // 1. 검색 결과 유효성 체크
    if (!AudioSearch || AudioSearch->SearchResultWidgets.Num() == 0)
    {
        PRINTNETLOG(this, TEXT("❌ 재생 목록이 비어있습니다."));
        return;
    }

    // 2. 현재 트랙 인덱스 찾기
    int32 CurrentIndex = FindTrackIndex(CurrentTrackData);

    // 3. 현재 트랙을 못 찾았으면 첫 번째 트랙 선택
    if (CurrentIndex == -1)
    {
        PRINTNETLOG(this, TEXT("⚠️ 현재 트랙을 찾을 수 없음. 첫 번째 트랙 선택"));
        CurrentIndex = 0;
    }

    // 4. 이전 인덱스 계산
    int32 PrevIndex = CurrentIndex - 1;
    int32 TotalTracks = AudioSearch->SearchResultWidgets.Num();

    // 5. 경계 체크 (역순환)
    if (PrevIndex < 0)
    {
        PRINTNETLOG(this, TEXT("🔁 첫 번째 곡. 마지막 곡으로 순환"));
        PrevIndex = TotalTracks - 1;
    }

    // 6. 이전 트랙 가져오기
    UMVE_STD_WC_AudioSearchResult* PrevTrackWidget = AudioSearch->SearchResultWidgets[PrevIndex];
    if (!PrevTrackWidget)
    {
        PRINTNETLOG(this, TEXT("❌ 이전 트랙 위젯이 null입니다."));
        return;
    }

    FMVE_STD_AudioSearchResultData PrevTrackData = PrevTrackWidget->GetAudioData();
    PRINTNETLOG(this, TEXT("✅ 이전 곡: %s (인덱스: %d/%d)"), *PrevTrackData.Title, PrevIndex + 1, TotalTracks);

    // 7. 자동 재생 플래그 설정
    if (bAutoPlay)
    {
        bAutoPlayAfterSelection = true;
    }

    // 8. 트랙 선택 (기존 함수 재사용!)
    OnTrackSelected(PrevTrackData);
}

void UMVE_STU_WidgetController_StudioConcert::HandleVoiceCommand(ESTTCommandType CommandType,
    const FString& OriginalText)
{
    PRINTNETLOG(this, TEXT("🎤 음성 명령 수신: %s - \"%s\""),
        *USTTSubsystem::GetCommandDisplayName(CommandType),
        *OriginalText);

    switch (CommandType)
    {
    case ESTTCommandType::PlayTrack:
        {
            // 현재 트랙이 선택되지 않았는지 체크 (Id가 0이면 비어있음)
            if (CurrentTrackData.Id == 0)
            {
                PRINTNETLOG(this, TEXT("⚠️ 선택된 트랙이 없음. 첫 번째 트랙 자동 선택 시도..."));

                // AudioSearch가 유효하고 검색 결과가 있는지 확인
                if (AudioSearch && AudioSearch->SearchResultWidgets.Num() > 0)
                {
                    // 첫 번째 트랙 가져오기
                    UMVE_STD_WC_AudioSearchResult* FirstTrack = AudioSearch->SearchResultWidgets[0];
                    if (FirstTrack)
                    {
                        FMVE_STD_AudioSearchResultData FirstTrackData = FirstTrack->GetAudioData();
                        PRINTNETLOG(this, TEXT("✅ 첫 번째 트랙 선택: %s"), *FirstTrackData.Title);

                        // 자동 재생 플래그 설정
                        bAutoPlayAfterSelection = true;

                        // 트랙 선택 (URL 요청 및 UI 설정)
                        OnTrackSelected(FirstTrackData);
                    }
                    else
                    {
                        PRINTNETLOG(this, TEXT("❌ 첫 번째 트랙 위젯이 null입니다."));
                    }
                }
                else
                {
                    PRINTNETLOG(this, TEXT("❌ 검색 결과가 없습니다. 먼저 음악 목록을 불러오세요."));
                }
            }
            else
            {
                // 이미 트랙이 선택되어 있으면 바로 재생
                PRINTNETLOG(this, TEXT("▶️ 선택된 트랙 재생: %s"), *CurrentTrackData.Title);
                OnPlayRequested();
            }
        }
        break;

    case ESTTCommandType::StopTrack:
        PRINTNETLOG(this, TEXT("⏹️ Stop 명령 실행"));
        OnStopRequested();
        break;

    case ESTTCommandType::NextTrack:
        PRINTNETLOG(this, TEXT("⏭️ NextTrack 명령 실행"));
        OnNextTrackRequested(true);  // 자동 재생
        break;

    default:
        PRINTNETLOG(this, TEXT("⚠️ 처리되지 않은 명령: %d"), (int32)CommandType);
        break;
    }
}
