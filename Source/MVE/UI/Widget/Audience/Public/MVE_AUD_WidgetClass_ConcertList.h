
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/RoomInfo.h"
#include "Data/RoomInfoData.h"
#include "MVE_AUD_WidgetClass_ConcertList.generated.h"

class UTileView;

UCLASS(Config=Game)
class MVE_API UMVE_AUD_WidgetClass_ConcertList : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> TempRoomSessionTableAsset;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTileView> RoomTileView;

public:
	/**
	 * 방 목록 업데이트
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateRoomList(const TArray<FRoomInfo>& Rooms);

	/**
	 * 방 목록 초기화
	 */
	UFUNCTION(BlueprintCallable)
	void ClearRoomList();

	// 방 선택 이벤트
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomSelected, URoomInfoData*);
	FOnRoomSelected OnRoomSelected;

public:
	/**
	 * 세션 목록 새로고침
	 */
	UFUNCTION(BlueprintCallable)
	void RefreshSessionList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/**
	 * SessionManager의 OnSessionsFound 콜백
	 */
	UFUNCTION()
	void OnSessionsFoundCallback(bool bSuccess, const TArray<FRoomInfo>& Sessions);

	/**
	 * ListView 항목 선택 이벤트
	 */
	UFUNCTION()
	void OnRoomItemSelected(UObject* SelectedItem);

	/**
	 * ListView 항목 클릭 이벤트
	 */
	UFUNCTION()
	void OnRoomItemClicked(UObject* ClickedItem);
};
