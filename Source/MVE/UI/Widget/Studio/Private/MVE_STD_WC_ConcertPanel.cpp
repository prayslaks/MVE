// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_STD_WC_ConcertPanel.h"

#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_GIS_API.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMVE_STD_WC_ConcertPanel::OnOpenToggleButtonClicked()
{
	if (const auto Subsystem = GetGameInstance()->GetSubsystem<UMVE_GIS_API>())
	{
		FOnGenericApiComplete OnResult;
		OnResult.BindLambda([this](const bool bSuccess, const FString& ErrorCode)
		{
			if (bSuccess)
			{
				IsOpen = !IsOpen;
				OpenToggleButtonTextBlock->SetText(IsOpen ? FText::FromString(TEXT("콘서트 닫기!")) : FText::FromString(TEXT("콘서트 열기!")));
				const FLinearColor TextColor = IsOpen ? FLinearColor::Green : FLinearColor::White;
				OpenToggleButtonTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
			}
			else
			{
				PRINTLOG(TEXT("ToggleConcertOpen 실패: %s"), *ErrorCode);
				OpenToggleButtonTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
			}
		});
		UMVE_API_Helper::ToggleConcertOpen(Subsystem->CurrentRoomId, !IsOpen, OnResult);	
	}
}

void UMVE_STD_WC_ConcertPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	OpenToggleButtonTextBlock->SetText(FText::FromString(TEXT("콘서트 열기!")));
	OpenToggleButtonTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	IsOpen = false;
	
	if (OpenToggleButton)
	{
		OpenToggleButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_ConcertPanel::OnOpenToggleButtonClicked);
	}
}