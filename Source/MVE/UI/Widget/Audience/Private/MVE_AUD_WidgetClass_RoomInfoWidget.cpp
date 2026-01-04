
#include "../Public/MVE_AUD_WidgetClass_RoomInfoWidget.h"
#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/RoomInfoData.h"
#include "UI/Widget/PopUp/Public/MVE_AUD_WidgetClass_JoinRoomConfirmPopup.h"

void UMVE_AUD_WidgetClass_RoomInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConcertRoomButton)
	{
		ConcertRoomButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnConcertRoomButtonClicked);
		ConcertRoomButton.Get()->OnHovered.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonHovered);
		ConcertRoomButton.Get()->OnUnhovered.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonUnhovered);
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// ListView가 자동으로 호출! 데이터 객체를 받아옴
	ConcertData = Cast<UConcertInfoData>(ListItemObject);

	if (!ConcertData)
	{
		PRINTLOG(TEXT("ListItemObject is not UConcertInfoData!"));
		return;
	}

	// UI 업데이트
	UpdateUI(ConcertData);
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);

	PRINTLOG(TEXT("Item selection changed: %s"), bIsSelected ? TEXT("Selected") : TEXT("Deselected"));

	// 선택 시 시각적 피드백
	if (RoomBorder)
	{
		FLinearColor BorderColor = bIsSelected 
			? FLinearColor(0.2f, 0.2f, 0.2f, 1.0f) // 회색
			: FLinearColor(0.2f, 0.2f, 0.2f, 0.0f); // 투명
        
		RoomBorder->SetBrushColor(BorderColor);
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::UpdateUI(UConcertInfoData* RoomData)
{
	if (!RoomData)
	{
		return;
	}

	PRINTLOG(TEXT("Updating room UI: %s"), *RoomData->ConcertInfo.ConcertName);
	PRINTLOG(TEXT("  RoomId: %s"), *RoomData->ConcertInfo.RoomId);
	PRINTLOG(TEXT("  StudioUserId: %d"), RoomData->ConcertInfo.StudioUserId);
	PRINTLOG(TEXT("  StudioName: %s"), *RoomData->ConcertInfo.StudioName);
	PRINTLOG(TEXT("  ConcertName: %s"), *RoomData->ConcertInfo.ConcertName);

	// 콘서트 제목
	if (ConcertNameText)
	{
		ConcertNameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *RoomData->ConcertInfo.ConcertName)));
	}

	// 방송인 이름 (이메일에서 @ 앞부분만 추출)
	if (SessionOwnerNameText)
	{
		FString StudioName = RoomData->ConcertInfo.StudioName;

		// 이메일 형식이면 @ 앞부분만 추출 (예: abcd1234@gmail.com → abcd1234)
		int32 AtIndex;
		if (StudioName.FindChar(TEXT('@'), AtIndex))
		{
			StudioName = StudioName.Left(AtIndex);
		}
		StudioName.Append(TEXT(" 님의 콘서트"));

		SessionOwnerNameText->SetText(FText::FromString(StudioName));
	}

	// 방송 시간
	if (BroadcastTimeText)
	{
		//FString TimeStr = RoomData->RoomInfo.BroadcastTime.ToString(TEXT("%H:%M"));
		FString TimeStr = TEXT("1분 전");
		BroadcastTimeText->SetText(FText::FromString(TimeStr));
	}

	// 시청자 수
	if (ViewerCountText)
	{
		ViewerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d"), RoomData->ConcertInfo.CurrentAudience)));
	}
	
	if (ThumbnailImage)
	{
		if (Thumbnails.Num() > 0)
		{
			int idx = FMath::RandRange(0, Thumbnails.Num()-1);
			ThumbnailImage->SetBrushFromTexture(Thumbnails[idx]);	
		}
	}
	
	// 라이브 인디케이터
	if (LiveIndicator)
	{
		LiveIndicator->SetVisibility(RoomData->ConcertInfo.IsOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 추천 배지
	if (FeaturedBadge)
	{
		//FeaturedBadge->SetVisibility(RoomData->RoomInfo.bIsFeatured ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 신규 배지
	if (NewBadge)
	{
		//NewBadge->SetVisibility(RoomData->RoomInfo.bIsNew ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 커스텀 색상 적용
	if (RoomBorder)
	{
		//RoomBorder->SetBrushColor(RoomData->RoomInfo.RoomColor);
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::OnConcertRoomButtonClicked()
{
	if (ConcertData)
	{
		PRINTLOG(TEXT("Room button clicked: %s"), *ConcertData->ConcertInfo.ConcertName);
		OnConcertRoomClicked.Broadcast(ConcertData);

		// 참가 확인 팝업 띄우기
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UUserWidget* PopupWidget = UIManager->ShowPopup(EUIPopup::JoinRoomConfirm, true);
			UMVE_AUD_WidgetClass_JoinRoomConfirmPopup* JoinPopup = Cast<UMVE_AUD_WidgetClass_JoinRoomConfirmPopup>(PopupWidget);
			
			if (JoinPopup)
			{
				const FConcertInfo Info = ConcertData->GetConcertInfo();
				JoinPopup->SetRoomInfo(Info);
			}
		}
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonHovered()
{
	if (RoomBorder)
	{
		FLinearColor BorderColor = HoveredColor; // 회색
		RoomBorder->SetBrushColor(BorderColor);
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonUnhovered()
{
	if (RoomBorder)
	{
		FLinearColor BorderColor = UnhoveredColor; // 투명색
		RoomBorder->SetBrushColor(BorderColor);
	}
}
