
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STU_WC_ConcertRoomInfo.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;

/**
 * Concert Room Information Widget
 * - 방 제목 입력/수정
 * - 콘서트 열기/닫기 토글
 * - 시청자 수 표시
 */

UCLASS()
class MVE_API UMVE_STU_WC_ConcertRoomInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	// Delegates - 컨트롤러에게 이벤트 전달
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomTitleChanged, const FString& /*NewTitle*/);
	FOnRoomTitleChanged OnRoomTitleChanged;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConcertToggled, bool /*bIsOpen*/);
	FOnConcertToggled OnConcertToggled;

	// Public API - 컨트롤러가 호출하는 함수들
	/**
	 * 방 제목 설정
	 * @param Title 새로운 방 제목
	 */
	void SetRoomTitle(const FString& Title);

	/**
	 * 시청자 수 업데이트
	 * @param Count 현재 시청자 수
	 */
	void UpdateViewerCount(int32 Count);

	/**
	 * 콘서트 상태 설정 (외부에서 강제로 상태 변경할 때)
	 * @param bIsOpen true면 콘서트 열림, false면 닫힘
	 */
	void SetConcertState(bool bIsOpen);

protected:
	// UI Components - Blueprint에서 BindWidget으로 연결
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RoomTitleTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ViewerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConcertToggleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConcertToggleButtonText;

	// Lifecycle
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// UI Event Handlers
	/**
	 * 방 제목 입력 완료 콜백 (엔터 키 입력 시)
	 */
	UFUNCTION()
	void OnRoomTitleCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/**
	 * 콘서트 토글 버튼 클릭 콜백
	 */
	UFUNCTION()
	void OnConcertToggleClicked();

	/**
	 * 버튼 외형 업데이트 (텍스트, 색상 등)
	 */
	void UpdateButtonAppearance();

	/**
	 * GameMode로부터 시청자 수 업데이트 받음
	 */
	UFUNCTION()
	void OnViewerCountUpdated(int32 NewCount);

	// State
	bool bIsConcertOpen = false;
};
