
#include "MVE_STU_WC_EffectSequencePreview.h"
#include "MVE_StageLevel_EffectSequenceManager.h"
#include "MVE.h"
#include "MVE_API_Helper.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/AudioComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeAudioFunctionLibrary.h"

void UMVE_STU_WC_EffectSequencePreview::NativeConstruct()
{
	Super::NativeConstruct();

	bIsSliderDragging = false;
	bIsAudioPaused = false;
	TotalDurationTimeStamp = 0;

	// PlayerController 가져오기
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PRINTLOG(TEXT("PlayerController를 찾을 수 없습니다!"));
		return;
	}

	// AudioComponent 생성 및 PlayerController에 부착
	AudioComponent = NewObject<UAudioComponent>(PC);
	if (AudioComponent)
	{
		AudioComponent->RegisterComponent();
		AudioComponent->bAutoActivate = false;

		// PlayerController에 부착
		AudioComponent->AttachToComponent(PC->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// UI 사운드 설정 (2D 사운드, Attenuation 없음)
		AudioComponent->SetUISound(true);
		AudioComponent->bAllowSpatialization = false;
		AudioComponent->bIsUISound = true;
		AudioComponent->bOverrideAttenuation = true;
		AudioComponent->SetVolumeMultiplier(1.0f);

		// Activate 호출
		AudioComponent->Activate();

		// 재생 퍼센트 델리게이트 바인딩
		AudioComponent->OnAudioPlaybackPercent.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnAudioPlaybackPercentChanged);

		PRINTLOG(TEXT("AudioComponent 생성됨 (PlayerController에 부착, UI Sound 설정 완료)"));
	}

	// 버튼 이벤트 바인딩
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnPlayButtonClicked);
	}

	// 슬라이더 이벤트 바인딩
	if (PlaybackSlider)
	{
		PlaybackSlider->OnMouseCaptureBegin.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderMouseCaptureBegin);
		PlaybackSlider->OnMouseCaptureEnd.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderMouseCaptureEnd);
		PlaybackSlider->OnValueChanged.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderValueChanged);
	}
}

void UMVE_STU_WC_EffectSequencePreview::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMVE_STU_WC_EffectSequencePreview::NativeDestruct()
{
	Super::NativeDestruct();

	ClearEffectIcons();
}

void UMVE_STU_WC_EffectSequencePreview::SetSequenceData(const TArray<FEffectSequenceData>& SequenceData, int32 TotalDuration)
{
	SequenceDataArray = SequenceData;
	TotalDurationTimeStamp = TotalDuration;

	// 슬라이더 범위 설정
	if (PlaybackSlider)
	{
		PlaybackSlider->SetMinValue(0.0f);
		PlaybackSlider->SetMaxValue(static_cast<float>(TotalDuration));
		PlaybackSlider->SetValue(0.0f);
	}

	// 총 시간 텍스트 업데이트
	if (TotalTimeTextBlock)
	{
		TotalTimeTextBlock->SetText(FText::FromString(FormatTime(TotalDuration)));
	}

	// 기존 아이콘 제거 후 새로 생성
	ClearEffectIcons();
	CreateEffectIcons();

	// EffectSequenceManager에 데이터 전달
	if (EffectSequenceManager)
	{
		EffectSequenceManager->SetSequenceData(SequenceData);
	}

	PRINTLOG(TEXT("Effect Sequence Preview 데이터 설정 완료 - %d개 이펙트, 총 길이: %s"),
		SequenceData.Num(), *FormatTime(TotalDuration));
}

void UMVE_STU_WC_EffectSequencePreview::UpdatePlaybackProgress(int32 CurrentTimeStamp)
{
	// 슬라이더 드래그 중이면 업데이트 무시
	if (bIsSliderDragging)
	{
		return;
	}

	// 슬라이더 값 업데이트
	if (PlaybackSlider)
	{
		PlaybackSlider->SetValue(static_cast<float>(CurrentTimeStamp));
	}

	// 현재 시간 텍스트 업데이트
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(FormatTime(CurrentTimeStamp)));
	}
}

void UMVE_STU_WC_EffectSequencePreview::UpdatePlayPauseButton(bool bIsPlaying)
{
	if (PlayStateImage)
	{
		UTexture2D* IconTexture = bIsPlaying ? PauseIconTexture : PlayIconTexture;
		if (IconTexture)
		{
			PlayStateImage->SetBrushFromTexture(IconTexture);
		}
	}
}

void UMVE_STU_WC_EffectSequencePreview::ResetPreview()
{
	// 슬라이더 초기화
	if (PlaybackSlider)
	{
		PlaybackSlider->SetValue(0.0f);
	}

	// 시간 텍스트 초기화
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(TEXT("0:00")));
	}

	// AudioComponent 정지
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}

	// EffectSequenceManager 초기화
	if (EffectSequenceManager)
	{
		EffectSequenceManager->ResetSequence();
	}

	PRINTLOG(TEXT("Effect Sequence Preview 초기화"));
}

void UMVE_STU_WC_EffectSequencePreview::SetEffectSequenceManager(AMVE_StageLevel_EffectSequenceManager* Manager)
{
	EffectSequenceManager = Manager;
	PRINTLOG(TEXT("EffectSequenceManager 설정됨"));
}

void UMVE_STU_WC_EffectSequencePreview::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	if (StagePreviewImage && RenderTarget)
	{
		// RenderTarget을 Brush로 변환
		FSlateBrush Brush;
		Brush.SetResourceObject(RenderTarget);
		Brush.ImageSize = FVector2D(RenderTarget->SizeX, RenderTarget->SizeY);
		Brush.DrawAs = ESlateBrushDrawType::Image;

		StagePreviewImage->SetBrush(Brush);

		PRINTLOG(TEXT("StagePreviewImage에 RenderTarget 설정됨: %s"), *RenderTarget->GetName());
	}
	else
	{
		PRINTLOG(TEXT("StagePreviewImage 또는 RenderTarget이 null입니다"));
	}
}

void UMVE_STU_WC_EffectSequencePreview::LoadTestData()
{
	// 테스트용 더미 데이터 생성
	// AI가 분석한 결과를 시뮬레이션
	TArray<FEffectSequenceData> TestData;

	// 0:10초 - Spotlight VerySlowSpeed (고요한 인트로)
	TestData.Add(FEffectSequenceData(100, FGameplayTag::RequestGameplayTag(FName("VFX.Spotlight.VerySlowSpeed"))));

	// 0:30초 - Spotlight SlowSpeed (서정적인 시작)
	TestData.Add(FEffectSequenceData(300, FGameplayTag::RequestGameplayTag(FName("VFX.Spotlight.SlowSpeed"))));

	// 1:00분 - Fanfare LowSpawnRate (설레는 첫 소절)
	TestData.Add(FEffectSequenceData(600, FGameplayTag::RequestGameplayTag(FName("VFX.Fanfare.LowSpawnRate"))));

	// 1:30분 - Spotlight NormalSpeed (경쾌한 팝)
	TestData.Add(FEffectSequenceData(900, FGameplayTag::RequestGameplayTag(FName("VFX.Spotlight.NormalSpeed"))));

	// 2:00분 - Flame NormalSmallSizeAndNormalSpeed (힙합 비트)
	TestData.Add(FEffectSequenceData(1200, FGameplayTag::RequestGameplayTag(FName("VFX.Flame.NormalSmallSizeAndNormalSpeed"))));

	// 2:30분 - Fanfare NormalSpawnRate (즐거운 축제)
	TestData.Add(FEffectSequenceData(1500, FGameplayTag::RequestGameplayTag(FName("VFX.Fanfare.NormalSpawnRate"))));

	// 3:00분 - Spotlight FastSpeed (고조되는 댄스)
	TestData.Add(FEffectSequenceData(1800, FGameplayTag::RequestGameplayTag(FName("VFX.Spotlight.FastSpeed"))));

	// 3:30분 - Flame FastSizeAndFastSpeed (파워풀한 하이라이트)
	TestData.Add(FEffectSequenceData(2100, FGameplayTag::RequestGameplayTag(FName("VFX.Flame.FastSizeAndFastSpeed"))));

	// 4:00분 - Fanfare HighSpawnRate (폭발적인 메인 하이라이트)
	TestData.Add(FEffectSequenceData(2400, FGameplayTag::RequestGameplayTag(FName("VFX.Fanfare.HighSpawnRate"))));

	// 4:30분 - Spotlight VeryFastSpeed (락 장르 클라이막스)
	TestData.Add(FEffectSequenceData(2700, FGameplayTag::RequestGameplayTag(FName("VFX.Spotlight.VeryFastSpeed"))));

	// 5:00분 - Flame VeryFastSizeAndVeryFastSpeed (극강 클라이막스)
	TestData.Add(FEffectSequenceData(3000, FGameplayTag::RequestGameplayTag(FName("VFX.Flame.VeryFastSizeAndVeryFastSpeed"))));

	// 5:30분 - Fanfare VeryHighSpawnRate (그랜드 피날레)
	TestData.Add(FEffectSequenceData(3300, FGameplayTag::RequestGameplayTag(FName("VFX.Fanfare.VeryHighSpawnRate"))));

	// 총 길이 6분 (3600 = 360초 * 10)
	int32 TotalDuration = 3600;

	// 데이터 설정
	SetSequenceData(TestData, TotalDuration);

	PRINTLOG(TEXT("테스트 데이터 로드 완료 - %d개 이펙트, 총 길이: 6:00"), TestData.Num());
}

void UMVE_STU_WC_EffectSequencePreview::SetAudioFile(const FAudioFile& AudioFile)
{
	PRINTLOG(TEXT("🎵 새 음악 선택: %s - %s"), *AudioFile.Title, *AudioFile.Artist);

	// 🔹 UI 먼저 초기화 (어떤 경로로 가든 무조건 리셋)
	if (PlaybackSlider)
	{
		PlaybackSlider->SetValue(0.0f);
	}
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(TEXT("0:00")));
	}

	// 🔹 델리게이트 먼저 해제
	if (AudioComponent)
	{
		AudioComponent->OnAudioPlaybackPercent.Clear();
	}

	// 🔹 기존 재생 중인 음악 완전히 정지
	if (AudioComponent)
	{
		if (AudioComponent->IsPlaying())
		{
			AudioComponent->Stop();
			PRINTLOG(TEXT("기존 음악 정지됨"));
		}
		AudioComponent->SetPaused(false);
		AudioComponent->SetSound(nullptr);
	}

	// EffectSequenceManager 초기화
	if (EffectSequenceManager)
	{
		EffectSequenceManager->ResetSequence();
	}

	// CurrentSound 초기화 (새로운 곡 로드될 때까지 null)
	CurrentSound = nullptr;
	CurrentAudioFile = AudioFile;
	bIsAudioPaused = false; // 일시정지 상태 플래그 리셋

	// Duration is in seconds, convert to 1/10 second units
	TotalDurationTimeStamp = AudioFile.Duration * 10;

	// 🔹 UI 초기화 (Stop() 후에 설정하여 델리게이트로 인한 덮어쓰기 방지)
	if (PlaybackSlider)
	{
		PlaybackSlider->SetMinValue(0.0f);
		PlaybackSlider->SetMaxValue(static_cast<float>(TotalDurationTimeStamp));
		PlaybackSlider->SetValue(0.0f);
	}

	// 현재 시간 텍스트 초기화
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(TEXT("0:00")));
	}

	// 총 시간 텍스트 업데이트
	if (TotalTimeTextBlock)
	{
		TotalTimeTextBlock->SetText(FText::FromString(FormatTime(TotalDurationTimeStamp)));
	}

	// 재생 버튼 상태 업데이트 (정지 상태로)
	UpdatePlayPauseButton(false);

	PRINTLOG(TEXT("음악 설정 완료: 길이 %s"), *FormatTime(TotalDurationTimeStamp));

	// 🎯 Asset 캐시 확인
	if (CachedAssets.Contains(AudioFile.Id))
	{
		PRINTLOG(TEXT("✅ 캐시에서 Asset 로드: %s"), *AudioFile.Title);

		UglTFRuntimeAsset* CachedAsset = CachedAssets[AudioFile.Id];
		if (CachedAsset)
		{
			// 🔹 매번 새로운 SoundWave 생성 (재생 위치 완전히 리셋!)
			const EglTFRuntimeAudioDecoder Decoder{};
			const FglTFRuntimeAudioConfig Config{};
			USoundWave* FreshSoundWave = UglTFRuntimeAudioFunctionLibrary::LoadSoundFromBlob(CachedAsset, Decoder, Config);

			if (FreshSoundWave)
			{
				CurrentSound = FreshSoundWave;
				PRINTLOG(TEXT("✅ 새 SoundWave 생성 완료: %s (Duration: %.2f초)"), *AudioFile.Title, FreshSoundWave->Duration);

				// 🔹 TotalDurationTimeStamp 업데이트
				TotalDurationTimeStamp = FMath::RoundToInt(FreshSoundWave->Duration * 10.0f);
				PRINTLOG(TEXT("TotalDurationTimeStamp 업데이트: %d (%s)"), TotalDurationTimeStamp, *FormatTime(TotalDurationTimeStamp));

				// 🔹 슬라이더 범위 업데이트
				if (PlaybackSlider)
				{
					PlaybackSlider->SetMaxValue(static_cast<float>(TotalDurationTimeStamp));
					PlaybackSlider->SetValue(0.0f);
				}

				// 🔹 현재 시간 텍스트 초기화
				if (CurrentTimeTextBlock)
				{
					CurrentTimeTextBlock->SetText(FText::FromString(TEXT("0:00")));
				}

				// 🔹 총 시간 텍스트 업데이트
				if (TotalTimeTextBlock)
				{
					TotalTimeTextBlock->SetText(FText::FromString(FormatTime(TotalDurationTimeStamp)));
				}

				// 🔹 델리게이트 바인딩
				if (AudioComponent)
				{
					AudioComponent->OnAudioPlaybackPercent.Clear();
					AudioComponent->OnAudioPlaybackPercent.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnAudioPlaybackPercentChanged);
				}

				return; // 서버 요청 불필요!
			}
		}
	}

	// 캐시 미스 - 서버에서 로드
	PRINTLOG(TEXT("❌ 캐시 미스 - 서버에서 음악 다운로드: %s"), *AudioFile.Title);

	// StreamAudio API를 호출하여 Presigned URL 받기
	FOnStreamAudioComplete OnResult;
	OnResult.BindLambda([this](const bool bSuccess, const FStreamAudioResponseData& ResponseData, const FString& ErrorCode)
	{
		if (bSuccess)
		{
			const FString PresignedUrl = ResponseData.StreamUrl;
			PRINTLOG(TEXT("Presigned URL 수신 성공: %s"), *PresignedUrl);

			// glTFRuntime으로 URL에서 오디오 다운로드
			FglTFRuntimeHttpResponse Response;
			Response.BindUFunction(this, FName("OnAudioLoadedFromUrl"));

			const TMap<FString, FString> Headers;
			FglTFRuntimeConfig Config;
			Config.bAsBlob = true; // 바이너리 데이터(Blob)로 받도록 설정
			UglTFRuntimeFunctionLibrary::glTFLoadAssetFromUrl(PresignedUrl, Headers, Response, Config);
		}
		else
		{
			PRINTLOG(TEXT("Presigned URL 받기 실패: %s"), *ErrorCode);
		}
	});

	UMVE_API_Helper::StreamAudio(AudioFile.Id, OnResult);
}

void UMVE_STU_WC_EffectSequencePreview::OnAudioPlaybackPercentChanged(const USoundWave* PlayingSoundWave, const float PlaybackPercent)
{
	if (!PlayingSoundWave || bIsSliderDragging)
	{
		return;
	}

	// 현재 재생 시간 계산 (초 단위)
	float PlaybackTime = PlaybackPercent * PlayingSoundWave->Duration;

	// 1/10초 단위로 변환
	int32 CurrentTimeStamp = static_cast<int32>(PlaybackTime * 10.0f);

	// UI 업데이트 (슬라이더는 드래그 중이 아닐 때만)
	UpdatePlaybackProgress(CurrentTimeStamp);

	// EffectSequenceManager의 타임라인은 자체적으로 Tick에서 업데이트되므로
	// 여기서는 UI만 동기화
}

void UMVE_STU_WC_EffectSequencePreview::OnPlayButtonClicked()
{
	PRINTLOG(TEXT("PlayButton 클릭됨!"));

	if (!EffectSequenceManager)
	{
		PRINTLOG(TEXT("EffectSequenceManager가 null입니다! SetEffectSequenceManager()를 먼저 호출하세요."));

		// EffectSequenceManager 없이도 음악만 재생
		if (AudioComponent && CurrentSound)
		{
			if (bIsAudioPaused)
			{
				AudioComponent->SetPaused(false);
				bIsAudioPaused = false;
				PRINTLOG(TEXT("AudioComponent 재개 (EffectSequenceManager 없음)"));
			}
			else if (!AudioComponent->IsPlaying())
			{
				AudioComponent->Stop(); // 재생 위치 리셋을 위해 명시적으로 Stop 호출
				AudioComponent->SetSound(CurrentSound);
				AudioComponent->Play(0.0f); // 명시적으로 0초부터 재생
				PRINTLOG(TEXT("AudioComponent 재생 시작 (EffectSequenceManager 없음) - 0:00부터"));
				PRINTLOG(TEXT("  - Sound: %s"), CurrentSound ? *CurrentSound->GetName() : TEXT("null"));
				PRINTLOG(TEXT("  - Duration: %.2f초"), CurrentSound ? CurrentSound->Duration : 0.0f);
				PRINTLOG(TEXT("  - IsPlaying: %s"), AudioComponent->IsPlaying() ? TEXT("Yes") : TEXT("No"));
				PRINTLOG(TEXT("  - Volume: %.2f"), AudioComponent->VolumeMultiplier);
				PRINTLOG(TEXT("  - bIsUISound: %s"), AudioComponent->bIsUISound ? TEXT("Yes") : TEXT("No"));
			}
			else
			{
				AudioComponent->SetPaused(true);
				bIsAudioPaused = true;
				PRINTLOG(TEXT("AudioComponent 일시정지 (EffectSequenceManager 없음)"));
			}
		}
		else
		{
			PRINTLOG(TEXT("AudioComponent 또는 CurrentSound가 null입니다!"));
			PRINTLOG(TEXT("AudioComponent: %s, CurrentSound: %s"),
				AudioComponent ? TEXT("존재") : TEXT("null"),
				CurrentSound ? TEXT("존재") : TEXT("null"));
		}
		return;
	}

	if (EffectSequenceManager->IsPlaying())
	{
		// 중지
		EffectSequenceManager->StopSequence();

		// AudioComponent 일시정지
		if (AudioComponent && AudioComponent->IsPlaying())
		{
			AudioComponent->SetPaused(true);
			bIsAudioPaused = true;
		}

		OnStopClicked.Broadcast();
		PRINTLOG(TEXT("재생 중지"));
	}
	else
	{
		// 재생 시작
		EffectSequenceManager->StartSequence(false);

		// AudioComponent 재생 또는 재개
		if (AudioComponent)
		{
			if (CurrentSound)
			{
				if (bIsAudioPaused)
				{
					AudioComponent->SetPaused(false);
					bIsAudioPaused = false;
					PRINTLOG(TEXT("AudioComponent 재개"));
				}
				else if (!AudioComponent->IsPlaying())
				{
					AudioComponent->Stop(); // 재생 위치 리셋을 위해 명시적으로 Stop 호출
					AudioComponent->SetSound(CurrentSound);
					AudioComponent->Play(0.0f); // 명시적으로 0초부터 재생
					PRINTLOG(TEXT("AudioComponent 재생 시작 - 0:00부터"));
					PRINTLOG(TEXT("  - Sound: %s"), CurrentSound ? *CurrentSound->GetName() : TEXT("null"));
					PRINTLOG(TEXT("  - Duration: %.2f초"), CurrentSound ? CurrentSound->Duration : 0.0f);
					PRINTLOG(TEXT("  - IsPlaying: %s"), AudioComponent->IsPlaying() ? TEXT("Yes") : TEXT("No"));
					PRINTLOG(TEXT("  - Volume: %.2f"), AudioComponent->VolumeMultiplier);
					PRINTLOG(TEXT("  - bIsUISound: %s"), AudioComponent->bIsUISound ? TEXT("Yes") : TEXT("No"));
				}
			}
			else
			{
				PRINTLOG(TEXT("재생할 음악이 설정되지 않았습니다. LoadTestData() 또는 SetAudioFile()을 먼저 호출하세요."));
			}
		}
		else
		{
			PRINTLOG(TEXT("AudioComponent가 null입니다!"));
		}

		OnPlayClicked.Broadcast();
		PRINTLOG(TEXT("재생 시작"));
	}

	UpdatePlayPauseButton(EffectSequenceManager->IsPlaying());
}

void UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderMouseCaptureBegin()
{
	bIsSliderDragging = true;
}

void UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderMouseCaptureEnd()
{
	bIsSliderDragging = false;

	// 드래그 종료 시 EffectSequenceManager의 시간 업데이트
	if (PlaybackSlider && EffectSequenceManager)
	{
		int32 NewTimeStamp = static_cast<int32>(PlaybackSlider->GetValue());
		EffectSequenceManager->SeekToTimeStamp(NewTimeStamp);

		// AudioComponent도 같은 위치로 시크
		if (AudioComponent && CurrentSound && TotalDurationTimeStamp > 0)
		{
			float SeekTime = static_cast<float>(NewTimeStamp) / 10.0f; // 1/10초 → 초 변환
			AudioComponent->SetFloatParameter(FName("StartTime"), SeekTime);

			// 재생 중이었으면 다시 재생
			if (EffectSequenceManager->IsPlaying())
			{
				AudioComponent->Stop();
				AudioComponent->SetSound(CurrentSound);
				AudioComponent->Play(SeekTime);
			}
		}

		PRINTLOG(TEXT("슬라이더 드래그 종료 - TimeStamp: %d"), NewTimeStamp);
	}
}

void UMVE_STU_WC_EffectSequencePreview::OnPlaybackSliderValueChanged(float Value)
{
	// 드래그 중일 때만 시간 텍스트 업데이트 (EffectSequenceManager는 드래그 종료 시 업데이트)
	if (bIsSliderDragging && CurrentTimeTextBlock)
	{
		int32 TimeStamp = static_cast<int32>(Value);
		CurrentTimeTextBlock->SetText(FText::FromString(FormatTime(TimeStamp)));
	}
}

void UMVE_STU_WC_EffectSequencePreview::CreateEffectIcons()
{
	if (!IconCanvas || !PlaybackSlider)
	{
		PRINTLOG(TEXT("IconCanvas 또는 PlaybackSlider가 없습니다"));
		return;
	}

	// 슬라이더의 실제 픽셀 너비 가져오기
	FVector2D SliderSize = PlaybackSlider->GetCachedGeometry().GetLocalSize();
	float SliderWidth = SliderSize.X;

	// 슬라이더가 아직 렌더링되지 않아 크기가 0일 수 있음
	if (SliderWidth <= 0.0f)
	{
		PRINTLOG(TEXT("슬라이더 너비가 0입니다. 나중에 다시 시도하세요."));
		return;
	}

	for (const FEffectSequenceData& Data : SequenceDataArray)
	{
		// TimeStamp를 0~1 범위로 정규화
		float NormalizedPosition = 0.0f;
		if (TotalDurationTimeStamp > 0)
		{
			NormalizedPosition = static_cast<float>(Data.TimeStamp) / static_cast<float>(TotalDurationTimeStamp);
		}

		// 슬라이더 너비에 따라 X 좌표 계산
		float XPosition = NormalizedPosition * SliderWidth;

		// 아이콘 텍스처 가져오기
		UTexture2D* IconTexture = GetIconTextureForTag(Data.AssetID);
		if (!IconTexture)
		{
			PRINTLOG(TEXT("아이콘 텍스처를 찾을 수 없습니다: %s"), *Data.AssetID.ToString());
			continue;
		}

		// Image 위젯 생성
		UImage* IconImage = NewObject<UImage>(this);
		if (IconImage)
		{
			IconImage->SetBrushFromTexture(IconTexture);

			// Canvas Panel에 추가
			UCanvasPanelSlot* IconSlot = IconCanvas->AddChildToCanvas(IconImage);
			if (IconSlot)
			{
				// 위치 설정 (슬라이더 위에 배치)
				IconSlot->SetPosition(FVector2D(XPosition, 0.0f));
				IconSlot->SetSize(FVector2D(24.0f, 24.0f)); // 아이콘 크기
				IconSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬

				// 생성된 아이콘 저장 (나중에 정리용)
				EffectIconWidgets.Add(IconImage);

				PRINTLOG(TEXT("아이콘 생성 - TimeStamp: %d, X: %.2f, Tag: %s"),
					Data.TimeStamp, XPosition, *Data.AssetID.ToString());
			}
		}
	}

	PRINTLOG(TEXT("총 %d개 아이콘 생성 완료"), EffectIconWidgets.Num());
}

void UMVE_STU_WC_EffectSequencePreview::ClearEffectIcons()
{
	if (IconCanvas)
	{
		for (UImage* IconWidget : EffectIconWidgets)
		{
			if (IconWidget)
			{
				IconCanvas->RemoveChild(IconWidget);
			}
		}
	}

	EffectIconWidgets.Empty();
	PRINTLOG(TEXT("아이콘 제거 완료"));
}

UTexture2D* UMVE_STU_WC_EffectSequencePreview::GetIconTextureForTag(const FGameplayTag& Tag) const
{
	FString TagString = Tag.ToString();

	if (TagString.StartsWith(TEXT("VFX.Spotlight")))
	{
		return SpotlightIconTexture;
	}
	else if (TagString.StartsWith(TEXT("VFX.Flame")))
	{
		return FlameIconTexture;
	}
	else if (TagString.StartsWith(TEXT("VFX.Fanfare")))
	{
		return FanfareIconTexture;
	}

	return nullptr;
}

FString UMVE_STU_WC_EffectSequencePreview::FormatTime(int32 TimeStamp)
{
	// TimeStamp는 1/10초 단위
	int32 TotalSeconds = TimeStamp / 10;
	int32 Minutes = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;

	return FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
}

void UMVE_STU_WC_EffectSequencePreview::OnAudioLoadedFromUrl(UglTFRuntimeAsset* Asset)
{
	if (!Asset)
	{
		PRINTLOG(TEXT("오디오 로드 실패: Asset이 null입니다"));
		return;
	}

	PRINTLOG(TEXT("오디오 다운로드 완료, SoundWave 생성 중..."));

	// 🎯 Asset을 캐시에 저장 (SoundWave가 아니라!)
	if (!CachedAssets.Contains(CurrentAudioFile.Id))
	{
		CachedAssets.Add(CurrentAudioFile.Id, Asset);
		PRINTLOG(TEXT("💾 캐시에 Asset 저장: %s (ID: %d, 총 캐시: %d곡)"),
			*CurrentAudioFile.Title, CurrentAudioFile.Id, CachedAssets.Num());
	}

	// 다운로드된 바이너리 데이터를 SoundWave로 변환
	const EglTFRuntimeAudioDecoder Decoder{};
	const FglTFRuntimeAudioConfig Config{};
	USoundWave* LoadedSoundWave = UglTFRuntimeAudioFunctionLibrary::LoadSoundFromBlob(Asset, Decoder, Config);

	if (LoadedSoundWave)
	{
		CurrentSound = LoadedSoundWave;
		PRINTLOG(TEXT("음악 로드 성공: %s (Duration: %.2f초)"), *CurrentAudioFile.Title, LoadedSoundWave->Duration);

		// 🔹 TotalDurationTimeStamp 업데이트 (실제 로드된 음악 길이 사용)
		TotalDurationTimeStamp = FMath::RoundToInt(LoadedSoundWave->Duration * 10.0f);
		PRINTLOG(TEXT("TotalDurationTimeStamp 업데이트: %d (%s)"), TotalDurationTimeStamp, *FormatTime(TotalDurationTimeStamp));

		// 🔹 슬라이더 범위 업데이트
		if (PlaybackSlider)
		{
			PlaybackSlider->SetMaxValue(static_cast<float>(TotalDurationTimeStamp));
			PlaybackSlider->SetValue(0.0f);
			PRINTLOG(TEXT("슬라이더 범위 업데이트: 0 ~ %d"), TotalDurationTimeStamp);
		}

		// 🔹 총 시간 텍스트 업데이트
		if (TotalTimeTextBlock)
		{
			TotalTimeTextBlock->SetText(FText::FromString(FormatTime(TotalDurationTimeStamp)));
		}

		// AudioComponent의 델리게이트 바인딩
		if (AudioComponent)
		{
			AudioComponent->OnAudioPlaybackPercent.Clear();
			AudioComponent->OnAudioPlaybackPercent.AddDynamic(this, &UMVE_STU_WC_EffectSequencePreview::OnAudioPlaybackPercentChanged);
			PRINTLOG(TEXT("AudioComponent 델리게이트 바인딩 완료"));
		}
	}
	else
	{
		PRINTLOG(TEXT("SoundWave 생성 실패"));
	}
}
