
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

	if (PlaylistBuilderWidget)
	{
		PlaylistBuilderWidget.Get()->OnAudioFileSelected.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnAudioFileSelected);
		PlaylistBuilderWidget.Get()->OnBatchAnalyzeRequested.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnBatchAnalyzeRequested);
	}

	// SenderReceiver 델리게이트 바인딩 (AI 서버 응답 수신용)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USenderReceiver* SenderReceiver = GI->GetSubsystem<USenderReceiver>())
		{
			SenderReceiver->OnMusicAnalysisComplete.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnMusicAnalysisReceived);
			PRINTLOG(TEXT("✅ SenderReceiver OnMusicAnalysisComplete 델리게이트 바인딩 완료"));
		}
	}

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

void UMVE_STD_WidgetClass_FinalCheckSettings::NativeDestruct()
{
	// 델리게이트 언바인딩
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USenderReceiver* SenderReceiver = GI->GetSubsystem<USenderReceiver>())
		{
			SenderReceiver->OnMusicAnalysisComplete.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
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
	// 현재 선택된 오디오 저장
	CurrentSelectedAudio = SelectedAudio;

	// 음악 파일 설정
	EffectSequencePreviewWidget->SetAudioFile(SelectedAudio);

	// SessionManager에서 캐시된 데이터 가져오기 (TestMode 상관없이)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMVE_GIS_SessionManager* SessionManager = GI->GetSubsystem<UMVE_GIS_SessionManager>())
		{
			PRINTLOG(TEXT("🔍 SessionManager에서 AudioId %d 데이터 확인 중..."), SelectedAudio.Id);

			if (SessionManager->HasEffectSequenceForAudio(SelectedAudio.Id))
			{
				// ✅ 캐시된 데이터 사용
				TArray<FEffectSequenceData> SequenceData = SessionManager->GetEffectSequenceForAudio(SelectedAudio.Id);
				PRINTLOG(TEXT("✅ SessionManager에서 이펙트 시퀀스 가져옴 - %d개 이펙트"), SequenceData.Num());

				// EffectSequencePreview에 데이터 전달
				if (EffectSequencePreviewWidget)
				{
					int32 TotalDuration = EffectSequencePreviewWidget->GetTotalDurationTimeStamp();
					PRINTLOG(TEXT("📏 TotalDuration: %d (1/10초 단위)"), TotalDuration);

					EffectSequencePreviewWidget->SetSequenceData(SequenceData, TotalDuration);
					PRINTLOG(TEXT("📊 SetSequenceData 호출 완료 → CreateEffectIcons 실행되어야 함"));
				}
				else
				{
					PRINTLOG(TEXT("❌ EffectSequencePreviewWidget이 null입니다!"));
				}
			}
			else if (EffectSequencePreviewWidget && EffectSequencePreviewWidget->bTestMode)
			{
				// 🧪 데이터 없음 + TestMode: SetAudioFile 이후 실제 Duration으로 TestData 생성
				int32 TotalDuration = EffectSequencePreviewWidget->GetTotalDurationTimeStamp();
				PRINTLOG(TEXT("🧪 TestMode - AudioId %d에 대한 캐시 없음, TotalDuration: %d로 TestData 생성"), SelectedAudio.Id, TotalDuration);

				if (TotalDuration > 0)
				{
					// TestData 생성 (내부적으로 SetSequenceData, SessionManager 저장 모두 수행됨)
					EffectSequencePreviewWidget->GenerateTestDataFromDuration(TotalDuration, SelectedAudio.Title);
					PRINTLOG(TEXT("📊 GenerateTestDataFromDuration 호출 완료 → SetSequenceData & SessionManager 저장 완료"));
				}
				else
				{
					PRINTLOG(TEXT("⚠️ TotalDuration이 0입니다. 음악 파일 로딩을 기다리는 중..."));
				}
			}
			else
			{
				PRINTLOG(TEXT("⚠️ AudioId %d에 대한 이펙트 시퀀스가 SessionManager에 없습니다"), SelectedAudio.Id);
				PRINTLOG(TEXT("💡 먼저 'Analyze Playlist' 버튼을 눌러주세요"));
			}
		}
		else
		{
			PRINTLOG(TEXT("❌ SessionManager를 찾을 수 없습니다!"));
		}
	}
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnBatchAnalyzeRequested()
{
	if (!PlaylistBuilderWidget || !EffectSequencePreviewWidget)
	{
		PRINTLOG(TEXT("❌ 필수 위젯을 찾을 수 없습니다"));
		return;
	}

	TArray<FAudioFile> Playlist = PlaylistBuilderWidget->GetPlaylist();

	if (Playlist.Num() == 0)
	{
		PRINTLOG(TEXT("❌ 재생목록이 비어있습니다"));
		return;
	}

	if (EffectSequencePreviewWidget->bTestMode)
	{
		// 🧪 테스트 모드: 2초 로딩 애니메이션 재생 (UX 시뮬레이션)
		PRINTLOG(TEXT("🧪 TestMode 활성화 - 2초 로딩 애니메이션 재생"));

		if (EffectSequencePreviewWidget)
		{
			EffectSequencePreviewWidget->StartLoadingAnimationWithDuration(2.0f);
		}

		PRINTLOG(TEXT("💡 이제 재생목록에서 곡을 클릭하세요"));
	}
	else
	{
		// 🎯 실제 모드: AI 서버에 배치 음악 분석 요청
		PRINTLOG(TEXT("🎯 실제 모드 - AI 서버에 배치 음악 분석 요청"));

		// 🔄 로딩 애니메이션 시작
		if (EffectSequencePreviewWidget)
		{
			EffectSequencePreviewWidget->StartLoadingAnimation();
		}

		USenderReceiver* SenderReceiver = GetGameInstance()->GetSubsystem<USenderReceiver>();
		if (!SenderReceiver)
		{
			PRINTLOG(TEXT("❌ SenderReceiver 서브시스템을 찾을 수 없습니다"));

			// 에러 발생 시 로딩 애니메이션 중지
			if (EffectSequencePreviewWidget)
			{
				EffectSequencePreviewWidget->StopLoadingAnimation();
			}
			return;
		}

		// 배치 음악 분석 요청
		SenderReceiver->SendBatchMusicAnalysisRequest(Playlist);
		PRINTLOG(TEXT("✅ 배치 분석 요청 전송 완료"));
	}
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnMusicAnalysisReceived(bool bSuccess, const TArray<FEffectSequenceData>& SequenceData, const FString& ErrorMessage)
{
	// 배치 분석에서는 SessionManager에 이미 저장됨
	// 현재 선택된 곡의 분석이 완료되었는지 확인해서 UI만 업데이트

	// ⏹️ 로딩 애니메이션 중지
	if (EffectSequencePreviewWidget)
	{
		EffectSequencePreviewWidget->StopLoadingAnimation();
	}

	if (bSuccess)
	{
		PRINTLOG(TEXT("✅ 음악 분석 델리게이트 수신 - %d개 이펙트"), SequenceData.Num());

		// SessionManager에서 현재 선택된 곡의 데이터 가져오기
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMVE_GIS_SessionManager* SessionManager = GI->GetSubsystem<UMVE_GIS_SessionManager>())
			{
				if (SessionManager->HasEffectSequenceForAudio(CurrentSelectedAudio.Id))
				{
					TArray<FEffectSequenceData> CachedData = SessionManager->GetEffectSequenceForAudio(CurrentSelectedAudio.Id);

					// EffectSequencePreview 업데이트
					if (EffectSequencePreviewWidget)
					{
						int32 TotalDuration = EffectSequencePreviewWidget->GetTotalDurationTimeStamp();
						EffectSequencePreviewWidget->SetSequenceData(CachedData, TotalDuration);
						PRINTLOG(TEXT("📊 현재 선택된 곡('%s') EffectSequencePreview 업데이트 완료"), *CurrentSelectedAudio.Title);
					}
				}
			}
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ 음악 분석 실패: %s"), *ErrorMessage);
	}
}
