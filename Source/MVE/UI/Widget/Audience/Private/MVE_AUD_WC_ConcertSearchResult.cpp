#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearchResult.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UMVE_AUD_WC_ConcertSearchResult::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMVE_AUD_WC_ConcertSearchResult::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(HoveredColor);
	}
}

void UMVE_AUD_WC_ConcertSearchResult::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(DeselectedColor);
	}
}

void UMVE_AUD_WC_ConcertSearchResult::UpdateUI(const FMVE_AUD_ConcertSearchResultData& InConcertSearchResult)
{
	BindingConcertSearchResult = InConcertSearchResult;

	if (ConcertNameTextBlock)
	{
		ConcertNameTextBlock->SetText(FText::FromString(BindingConcertSearchResult.ConcertName));
	}

	if (HostNameTextBlock)
	{
		HostNameTextBlock->SetText(FText::FromString(BindingConcertSearchResult.StudioName));
	}

	if (AudienceCountTextBlock)
	{
		const FString AudienceCountString = FString::Printf(TEXT("%d / %d"), BindingConcertSearchResult.CurrentAudience, BindingConcertSearchResult.MaxAudience);
		AudienceCountTextBlock->SetText(FText::FromString(AudienceCountString));
	}

	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(BindingConcertSearchResult.bIsOpen ? TEXT("Open") : TEXT("Closed")));
        StatusTextBlock->SetColorAndOpacity(BindingConcertSearchResult.bIsOpen ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red));
	}

	SetSelected(false);
}

void UMVE_AUD_WC_ConcertSearchResult::SetSelected(const bool bInIsSelected)
{
	bIsSelected = bInIsSelected;
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
	}
}

FReply UMVE_AUD_WC_ConcertSearchResult::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (BindingConcertSearchResult.bIsOpen)
	{
		OnConcertSearchResultDoubleClicked.Broadcast(this);
	}
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}