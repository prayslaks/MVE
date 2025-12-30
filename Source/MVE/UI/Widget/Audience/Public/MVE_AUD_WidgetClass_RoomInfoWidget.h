
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Data/RoomInfoData.h"
#include "MVE_AUD_WidgetClass_RoomInfoWidget.generated.h"


class UBorder;
class UButton;
class UTextBlock;
class UImage;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_RoomInfoWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// === Bound Widgets ===
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConcertNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StudioNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BroadcastTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ViewerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ThumbnailImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConcertRoomButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> RoomBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> LiveIndicator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> FeaturedBadge;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> NewBadge;

	UPROPERTY(EditAnywhere)
	FLinearColor HoveredColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere)
	FLinearColor UnhoveredColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.0f);;

private:
	// 현재 표시 중인 데이터
	UPROPERTY()
	TObjectPtr<UConcertInfoData> ConcertData;

public:
	// 방 클릭 이벤트
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConcertRoomClicked, UConcertInfoData*);
	FOnConcertRoomClicked OnConcertRoomClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// === IUserObjectListEntry 인터페이스 구현 ===
    
	/**
	 * TileView가 이 위젯에 데이터를 설정할 때 자동 호출
	 * 이 함수에서 UI 업데이트 수행
	 */
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * 항목이 선택되었을 때 호출 (옵션)
	 */
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

private:
	/**
	 * UI 업데이트
	 */
	void UpdateUI(UConcertInfoData* RoomData);

	/**
	 * 버튼 클릭 핸들러
	 */
	UFUNCTION()
	void OnConcertRoomButtonClicked();

	UFUNCTION()
	void OnButtonHovered();

	UFUNCTION()
	void OnButtonUnhovered();
};
