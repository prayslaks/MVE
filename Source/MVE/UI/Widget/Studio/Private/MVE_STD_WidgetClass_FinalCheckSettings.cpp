
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
	EffectSequencePreviewWidget->SetAudioFile(SelectedAudio);

	// TestMode 체크
	if (EffectSequencePreviewWidget->bTestMode)
	{
		// 🧪 테스트 모드: 더미 데이터로 EffectSequenceManager 동작 확인
		EffectSequencePreviewWidget->LoadTestData();
		PRINTLOG(TEXT("✅ TestMode 활성화 - 더미 데이터 로드"));
	}
	else
	{
		// 🎯 실제 모드: AI 서버에 메타데이터 전송
		// TODO: AI 서버와 통신하여 TimeStamp 분석 결과 받기
		// 예시:
		// FString Title = SelectedAudio.Title;
		// FString Artist = SelectedAudio.Artist;
		// AI_Client->AnalyzeMusicMetadata(Title, Artist, [this](const TArray<FEffectSequenceData>& SequenceData, int32 TotalDuration)
		// {
		//     EffectSequencePreviewWidget->SetSequenceData(SequenceData, TotalDuration);
		// });

		PRINTLOG(TEXT("⚠️ TestMode 비활성화 - AI 서버 통신은 아직 구현되지 않았습니다"));
	}
}
