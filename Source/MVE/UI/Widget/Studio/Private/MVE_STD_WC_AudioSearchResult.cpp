// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Studio/Public/MVE_STD_WC_AudioSearchResult.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UMVE_STD_WC_AudioSearchResult::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMVE_STD_WC_AudioSearchResult::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(HoveredColor);
	}
}

void UMVE_STD_WC_AudioSearchResult::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(DeselectedColor);
	}
}

void UMVE_STD_WC_AudioSearchResult::UpdateUI(const FMVE_STD_AudioSearchResultData& InAudioSearchResult)
{
	// 업데이트
	BindingAudioSearchResult = InAudioSearchResult;
	
	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(FText::FromString(BindingAudioSearchResult.Title));
	}
	
	if (ArtistTextBlock)
	{
		ArtistTextBlock->SetText(FText::FromString(BindingAudioSearchResult.Artist));
	}
	
	if (DurationTextBlock)
	{
		// Duration is in seconds, format as MM:SS
		const int32 TotalSeconds = BindingAudioSearchResult.Duration;
		const int32 Minutes = TotalSeconds / 60;
		const int32 Seconds = TotalSeconds % 60;
		const FString FormattedDuration = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DurationTextBlock->SetText(FText::FromString(FormattedDuration));
	}
	
	SetSelected(false);
}

void UMVE_STD_WC_AudioSearchResult::SetSelected(const bool bInIsSelected)
{
	bIsSelected = bInIsSelected;
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
	}
}

FReply UMVE_STD_WC_AudioSearchResult::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnAudioSearchResultDoubleClicked.Broadcast(this);
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}