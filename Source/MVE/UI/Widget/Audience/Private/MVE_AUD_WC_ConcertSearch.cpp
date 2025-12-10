#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearch.h"

#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_GIS_API.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearchResult.h"
#include "Styling/SlateTypes.h"


void UMVE_AUD_WC_ConcertSearch::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (PollingToggleButton)
	{
		PollingToggleButton->OnClicked.AddDynamic(this, &UMVE_AUD_WC_ConcertSearch::OnPollingToggleButtonClicked);
	}

	if (ConcertCountTextBlock)
	{
		ConcertCountTextBlock->SetText(FText::FromString(TEXT("00개 콘서트 OnAir")));
	}

	// Initialize button appearance
	UpdatePollingButtonAppearance();
}

void UMVE_AUD_WC_ConcertSearch::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	Super::NativeDestruct();
}

void UMVE_AUD_WC_ConcertSearch::OnPollingToggleButtonClicked()
{
	bIsPollingActive = !bIsPollingActive;

	if (bIsPollingActive)
	{
		// Start polling
		GetWorld()->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UMVE_AUD_WC_ConcertSearch::RequestConcertList, 5.0f, true, 0.0f);
	}
	else
	{
		// Stop polling
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
		
		// Clear UI
		if (ConcertListScrollBox)
		{
			ConcertListScrollBox->ClearChildren();
		}
		SearchResultWidgets.Empty();
		SelectedSearchResult = nullptr;
		if (ConcertCountTextBlock)
		{
			ConcertCountTextBlock->SetText(FText::FromString(TEXT("00개 콘서트 OnAir")));
		}
	}
	// Update button appearance after state change
	UpdatePollingButtonAppearance();
}

void UMVE_AUD_WC_ConcertSearch::RequestConcertList()
{
	FOnGetConcertListComplete OnResult;
	OnResult.BindLambda([this](const bool bSuccess, const FGetConcertListData& ResponseData, const FString& ErrorCode)
	{
		// Stop processing if polling has been deactivated since the request was made
		if (!bIsPollingActive)
		{
			return;
		}

		if (bSuccess)
		{
			if (ConcertListScrollBox)
			{
				ConcertListScrollBox->ClearChildren();
			}
			SearchResultWidgets.Empty();
			SelectedSearchResult = nullptr;
		
			if (ConcertCountTextBlock)
			{
				const FString CountText = FString::Printf(TEXT("%02d개 콘서트 OnAir"), ResponseData.concerts.Num());
				ConcertCountTextBlock->SetText(FText::FromString(CountText));
			}
			
			if (ResponseData.concerts.IsEmpty() == false)
			{
				for (const FConcertInfo& ConcertValue : ResponseData.concerts)
				{
					FString HostName;
					const FString ConcertName = ConcertValue.concertName;
					const FString RoomId = ConcertValue.roomId;
					const int32 CurrentAudience = ConcertValue.currentAudience;
					const int32 MaxAudience = ConcertValue.maxAudience;
					const bool bIsOpen = ConcertValue.isOpen;
					
					// 위젯 생성 후 추가
					if(auto* NewWidget = CreateWidget<UMVE_AUD_WC_ConcertSearchResult>(this, ConcertSearchResultWidgetClass))
					{
						FMVE_AUD_ConcertSearchResultData ResultData(ConcertName, RoomId, HostName, CurrentAudience, MaxAudience, bIsOpen);
						NewWidget->UpdateUI(ResultData);
						NewWidget->OnConcertSearchResultDoubleClicked.AddUObject(this, &UMVE_AUD_WC_ConcertSearch::OnSearchResultDoubleClicked);
						SearchResultWidgets.Add(NewWidget);
						ConcertListScrollBox->AddChild(NewWidget);
					}
				}
			}
			else
			{
				if (ConcertCountTextBlock)
				{
					ConcertCountTextBlock->SetText(FText::FromString(TEXT("00개 콘서트 OnAir")));
				}
			}
		}
		else
		{
			if (ConcertCountTextBlock)
			{
				ConcertCountTextBlock->SetText(FText::FromString(TEXT("00개 콘서트 OnAir")));
			}
					
			if (const UMVE_GIS_API* Subsystem = UMVE_GIS_API::Get(this))
			{
				const FText TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}
	});
	UMVE_API_Helper::GetConcertList(OnResult);
}

void UMVE_AUD_WC_ConcertSearch::UpdatePollingButtonAppearance() const
{
	if (PollingToggleButtonTextBlock)
	{
		PollingToggleButtonTextBlock->SetText(bIsPollingActive ? ActiveText : InactiveText);
	}
	if (PollingToggleButton)
	{
		FButtonStyle ButtonStyle = PollingToggleButton->GetStyle();
		ButtonStyle.Normal.TintColor = bIsPollingActive ? ActiveColor : InactiveColor;
		// Also apply to hovered and pressed to maintain color consistency
		ButtonStyle.Hovered.TintColor = bIsPollingActive ? ActiveColor : InactiveColor;
		ButtonStyle.Pressed.TintColor = bIsPollingActive ? ActiveColor : InactiveColor;
		PollingToggleButton->SetStyle(ButtonStyle);
	}
}



void UMVE_AUD_WC_ConcertSearch::OnSearchResultDoubleClicked(UMVE_AUD_WC_ConcertSearchResult* ClickedWidget)
{
	if (!ClickedWidget || !ClickedWidget->GetConcertData().bIsOpen)
	{
		return;
	}

	if (SelectedSearchResult)
	{
		SelectedSearchResult->SetSelected(false);
	}

	SelectedSearchResult = ClickedWidget;
	SelectedSearchResult->SetSelected(true);
	
	OnConcertDoubleClicked.Broadcast(ClickedWidget->GetConcertData());
}