
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/RoomInfo.h"
#include "MVE_AUD_WidgetClass_JoinRoomConfirmPopup.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_JoinRoomConfirmPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 방 정보 설정 (외부에서 호출)
	 */
	UFUNCTION(BlueprintCallable)
	void SetRoomInfo(const FRoomInfo& RoomInfo);

	// 확인/취소 이벤트
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfirmed, const FRoomInfo&);
	FOnConfirmed OnConfirmed;

	DECLARE_MULTICAST_DELEGATE(FOnCancelled);
	FOnCancelled OnCancelled;

protected:
	virtual void NativeConstruct() override;

private:
	// 현재 표시 중인 방 정보
	FRoomInfo CurrentRoomInfo;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoomTitleText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

private:
	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	/**
	 * UI 업데이트
	 */
	void UpdateUI();
};
