
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WC_ConcertRoomInfo.generated.h"

class UTextBlock;

/**
 * Concert Room Information Widget (Audience 버전)
 * - 방 제목 표시 (읽기 전용)
 * - 시청자 수 표시
 */
UCLASS()
class MVE_API UMVE_AUD_WC_ConcertRoomInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 방 제목 설정
	 * @param Title 방 제목
	 */
	void SetRoomTitle(const FString& Title);

	/**
	 * 시청자 수 업데이트
	 * @param Count 현재 시청자 수
	 */
	void UpdateViewerCount(int32 Count);

protected:
	// UI Components - Blueprint에서 BindWidget으로 연결
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoomTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ViewerCountText;

	// Lifecycle
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/**
	 * GameMode로부터 시청자 수 업데이트 받음
	 */
	UFUNCTION()
	void OnViewerCountUpdated(int32 NewCount);
};
