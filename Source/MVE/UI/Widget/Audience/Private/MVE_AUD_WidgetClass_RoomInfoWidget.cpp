
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

	if (RoomButton)
	{
		RoomButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnRoomButtonClicked);
		RoomButton.Get()->OnHovered.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonHovered);
		RoomButton.Get()->OnUnhovered.AddDynamic(this, &UMVE_AUD_WidgetClass_RoomInfoWidget::OnButtonUnhovered);
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
	URoomInfoData* RoomData = Cast<URoomInfoData>(ListItemObject);

	if (!RoomData)
	{
		PRINTLOG(TEXT("ListItemObject is not URoomInfoData!"));
		return;
	}

	// 데이터 캐싱
	CachedRoomData = RoomData;

	// UI 업데이트
	UpdateUI(RoomData);
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

void UMVE_AUD_WidgetClass_RoomInfoWidget::UpdateUI(URoomInfoData* RoomData)
{
	if (!RoomData)
	{
		return;
	}

	PRINTLOG(TEXT("Updating room UI: %s"), *RoomData->RoomInfo.RoomTitle);

	// 방 ID
	if (RoomIDText)
	{
		RoomIDText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *RoomData->RoomInfo.RoomID)));
	}

	// 방 제목
	if (RoomTitleText)
	{
		RoomTitleText->SetText(FText::FromString(RoomData->RoomInfo.RoomTitle));
	}

	// 방송 시간
	if (BroadcastTimeText)
	{
		FString TimeStr = RoomData->RoomInfo.BroadcastTime.ToString(TEXT("%H:%M"));
		BroadcastTimeText->SetText(FText::FromString(TimeStr));
	}

	// 시청자 수
	if (ViewerCountText)
	{
		ViewerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d"), RoomData->RoomInfo.ViewerCount)));
	}

	// 썸네일
	if (ThumbnailImage)
	{
		if (RoomData->RoomInfo.Thumbnail)
			ThumbnailImage->SetBrushFromTexture(RoomData->RoomInfo.Thumbnail);
		else
			ThumbnailImage->SetBrushTintColor(FColor::Black);
	}

	// 라이브 인디케이터
	if (LiveIndicator)
	{
		LiveIndicator->SetVisibility(RoomData->RoomInfo.bIsLive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 추천 배지
	if (FeaturedBadge)
	{
		FeaturedBadge->SetVisibility(RoomData->RoomInfo.bIsFeatured ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 신규 배지
	if (NewBadge)
	{
		NewBadge->SetVisibility(RoomData->RoomInfo.bIsNew ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 커스텀 색상 적용
	if (RoomBorder)
	{
		RoomBorder->SetBrushColor(RoomData->RoomInfo.RoomColor);
	}
}

void UMVE_AUD_WidgetClass_RoomInfoWidget::OnRoomButtonClicked()
{
	if (CachedRoomData)
	{
		PRINTLOG(TEXT("Room button clicked: %s"), *CachedRoomData->RoomInfo.RoomTitle);
		OnRoomClicked.Broadcast(CachedRoomData);

		// 참가 확인 팝업 띄우기
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UUserWidget* PopupWidget = UIManager->ShowPopup(FName("EnterRoomConfirmation"), true);
			UMVE_AUD_WidgetClass_JoinRoomConfirmPopup* JoinPopup = Cast<UMVE_AUD_WidgetClass_JoinRoomConfirmPopup>(PopupWidget);
			
			if (JoinPopup)
			{
				const FRoomInfo Info = FRoomInfo(CachedRoomData->GetRoomInfo());
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
