
#include "../Public/MVE_STD_WidgetClass_FinalCheckSettings.h"

#include "MVE_API_Helper.h"
#include "MVE_AUD_WidgetClass_ConcertRoom.h"
#include "MVE_GIS_SessionManager.h"
#include "MVE_STD_WC_PlaylistBuilder.h"
#include "MVE_STU_WC_EffectSequencePreview.h"
#include "MVE_StageLevel_EffectSequenceManager.h"
#include "MVE_STU_StagePreviewCaptureActor.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Data/RoomInfo.h"
#include "Kismet/GameplayStatics.h"
#include "MVE.h"
#include "commu/Public/SenderReceiver.h"

void UMVE_STD_WidgetClass_FinalCheckSettings::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartConcertButton)
		StartConcertButton.Get()->OnClicked.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnStartConcertButtonClicked);

	if (PlaylistBuilderWidget)
		PlaylistBuilderWidget.Get()->OnAudioFileSelected.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnAudioFileSelected);

	// EffectSequenceManager 찾기 (PlaylistBuilder와 EffectSequencePreview에 공통 설정)
	AMVE_StageLevel_EffectSequenceManager* Manager = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_EffectSequenceManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		Manager = Cast<AMVE_StageLevel_EffectSequenceManager>(FoundActors[0]);
		if (Manager)
		{
			PRINTLOG(TEXT("EffectSequenceManager 찾기 성공"));
		}
	}
	else
	{
		PRINTLOG(TEXT("EffectSequenceManager를 찾을 수 없습니다. PreviewStageLevel에 배치되어 있는지 확인하세요."));
	}

	// PlaylistBuilder에 EffectSequenceManager 설정
	if (PlaylistBuilderWidget && Manager)
	{
		PlaylistBuilderWidget->SetEffectSequenceManager(Manager);
		PRINTLOG(TEXT("PlaylistBuilder에 EffectSequenceManager 설정 완료"));
	}

	// EffectSequencePreview 설정
	if (EffectSequencePreviewWidget)
	{
		// EffectSequenceManager 설정
		if (Manager)
		{
			EffectSequencePreviewWidget->SetEffectSequenceManager(Manager);
			PRINTLOG(TEXT("EffectSequencePreview에 EffectSequenceManager 설정 완료"));
		}

		// StagePreviewCaptureActor 찾기
		FoundActors.Empty();
		UGameplayStatics::GetAllActorsOfClass(this, AMVE_STU_StagePreviewCaptureActor::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0)
		{
			AMVE_STU_StagePreviewCaptureActor* CaptureActor = Cast<AMVE_STU_StagePreviewCaptureActor>(FoundActors[0]);
			if (CaptureActor && CaptureActor->RenderTarget)
			{
				// SceneCaptureComponent가 RenderTarget에 캡처하도록 설정
				CaptureActor->SetRenderTarget(CaptureActor->RenderTarget);

				// 위젯에 RenderTarget 전달
				EffectSequencePreviewWidget->SetRenderTarget(CaptureActor->RenderTarget);
				PRINTLOG(TEXT("StagePreviewCaptureActor RenderTarget 설정 완료"));
			}
		}
		else
		{
			PRINTLOG(TEXT("StagePreviewCaptureActor를 찾을 수 없습니다. PreviewStageLevel에 배치되어 있는지 확인하세요."));
		}
	}
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnStartConcertButtonClicked()
{
	PlaylistBuilderWidget->SavePlaylistToSessionManager();
	
	FConcertInfo ConcertInfo;
	ConcertInfo.ConcertName = RoomTitleEditableText->GetText().ToString();
	ConcertInfo.MaxAudience = 20;
	ConcertInfo.Songs = TArray<FConcertSong>();
	ConcertInfo.Accessories = TArray<FAccessory>();

	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		SessionManager->CreateSession(ConcertInfo);
	}
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnAudioFileSelected(const FAudioFile& SelectedAudio)
{
	// SetAudioFile()에서 음악 로드 완료 후 TestMode일 때 자동으로 LoadTestData() 호출됨
	EffectSequencePreviewWidget->SetAudioFile(SelectedAudio);

	// TestMode가 아닐 때만 AI 서버에 요청
	if (!EffectSequencePreviewWidget->bTestMode)
	{
		// 🎯 실제 모드: AI 서버에 음악 분석 요청
		PRINTLOG(TEXT("🎯 TestMode 비활성화 - AI 서버에 음악 분석 요청"));

		// SenderReceiver 가져오기
		USenderReceiver* SenderReceiver = GetGameInstance()->GetSubsystem<USenderReceiver>();
		if (!SenderReceiver)
		{
			PRINTLOG(TEXT("❌ SenderReceiver 서브시스템을 찾을 수 없습니다"));
			return;
		}

		// 델리게이트 바인딩 (기존 바인딩 제거 후 새로 바인딩)
		SenderReceiver->OnMusicAnalysisComplete.Clear();
		SenderReceiver->OnMusicAnalysisComplete.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnMusicAnalysisReceived);

		// 음악 분석 요청
		FString Title = SelectedAudio.Title;
		FString Artist = SelectedAudio.Artist;

		PRINTLOG(TEXT("📤 AI 서버로 음악 분석 요청 전송 - Title: %s, Artist: %s"), *Title, *Artist);
		SenderReceiver->SendMusicAnalysisRequest(Title, Artist);
	}
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnMusicAnalysisReceived(bool bSuccess, const TArray<FEffectSequenceData>& SequenceData, const FString& ErrorMessage)
{
	if (bSuccess)
	{
		PRINTLOG(TEXT("✅ 음악 분석 성공 - %d개 이펙트 시퀀스 수신"), SequenceData.Num());

		// EffectSequencePreview에 데이터 전달
		// TotalDuration은 EffectSequencePreview가 이미 SetAudioFile에서 설정했으므로
		// getter 함수로 가져오기
		if (EffectSequencePreviewWidget)
		{
			int32 TotalDuration = EffectSequencePreviewWidget->GetTotalDurationTimeStamp();
			EffectSequencePreviewWidget->SetSequenceData(SequenceData, TotalDuration);
			PRINTLOG(TEXT("📊 EffectSequencePreview에 분석 데이터 설정 완료"));
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ 음악 분석 실패: %s"), *ErrorMessage);

		// 실패 시 빈 데이터로 설정 (UI 정리)
		if (EffectSequencePreviewWidget)
		{
			TArray<FEffectSequenceData> EmptyData;
			int32 TotalDuration = EffectSequencePreviewWidget->GetTotalDurationTimeStamp();
			EffectSequencePreviewWidget->SetSequenceData(EmptyData, TotalDuration);
		}
	}
}
