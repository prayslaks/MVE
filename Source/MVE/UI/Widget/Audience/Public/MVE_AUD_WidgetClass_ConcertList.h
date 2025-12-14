
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
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTileView> RoomTileView;

public:
	/**
	 * 콘서트 목록 업데이트
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateConcertList(const TArray<FConcertInfo>& Concerts);

	/**
	 * 콘서트 목록 초기화
	 */
	UFUNCTION(BlueprintCallable)
	void ClearConcertList();

	/**
	 * 세션 목록 새로고침
	 */
	UFUNCTION(BlueprintCallable)
	void RefreshSessionList();

	// 콘서트 선택 이벤트
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConcertSelected, class UConcertInfoData*);
	FOnConcertSelected OnConcertSelected;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/**
	 * SessionManager의 OnSessionsFound 콜백
	 */
	UFUNCTION()
	void OnSessionsFoundCallback(bool bSuccess, int32 NumSessions);

	/**
	 * TileView 항목 클릭 이벤트
	 */
	UFUNCTION()
	void OnConcertItemClicked(UObject* ClickedItem);
	
	/**
	 * SessionManager 참조
	 */
	UPROPERTY()
	TObjectPtr<class UMVE_GIS_SessionManager> SessionManager;
};
