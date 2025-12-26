// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_STU_WC_EffectSequencePreview.h"
#include "MVE_StageLevel_EffectSequenceManager.h"
#include "MVE.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UMVE_STU_WC_EffectSequencePreview::NativeConstruct()
{
	Super::NativeConstruct();

	bIsSliderDragging = false;
	TotalDurationTimeStamp = 0;

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

void UMVE_STU_WC_EffectSequencePreview::OnPlayButtonClicked()
{
	if (EffectSequenceManager)
	{
		if (EffectSequenceManager->IsPlaying())
		{
			EffectSequenceManager->StopSequence();
			OnStopClicked.Broadcast();
			PRINTLOG(TEXT("재생 중지"));
		}
		else
		{
			EffectSequenceManager->StartSequence(false);
			OnPlayClicked.Broadcast();
			PRINTLOG(TEXT("재생 시작"));
		}

		UpdatePlayPauseButton(EffectSequenceManager->IsPlaying());
	}
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
