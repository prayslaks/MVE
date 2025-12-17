
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageLevel/Data/MVE_ChatMessageEntry.h"
#include "StageLevel/Data/MVE_ChatTypes.h"
#include "MVE_WC_Chat.generated.h"

class UTextBlock;
class UBorder;
class UEditableTextBox;
class UMVE_StageLevel_WidgetController_Chat;
class UButton;
class UListView;

UCLASS()
class MVE_API UMVE_WC_Chat : public UUserWidget
{
	GENERATED_BODY()

	protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	//~ 컨트롤러 설정
	/**
	 * ChatWidgetController 설정
	 * @param InController 컨트롤러 인스턴스
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetController(UMVE_StageLevel_WidgetController_Chat* InController);

	/**
	 * 컨트롤러 가져오기
	 * @return 현재 컨트롤러
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	UMVE_StageLevel_WidgetController_Chat* GetController() const { return Controller; }

protected:
	//~ Native 델리게이트 콜백
	UFUNCTION()
	void OnMessageAdded(const FChatMessage& Message);

	UFUNCTION()
	void OnSendFailed(const FString& Reason, EChatRejectReason RejectType);

	UFUNCTION()
	void OnMessageRemoved(const FGuid& MessageID);

public:
	//~ Public API
	/**
	 * 메시지 전송
	 * @param MessageContent 메시지 내용
	 * @return 전송 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool SendMessage(const FString& MessageContent);

	/**
	 * 채팅 히스토리 로드
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void LoadChatHistory();

	/**
	 * 채팅창 클리어
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ClearChat();

protected:
	//~ UI 업데이트 메서드
	/**
	 * ListView에 메시지 추가
	 * @param Message 추가할 메시지
	 */
	void AddMessageToListView(const FChatMessage& Message);

	/**
	 * ListView에서 메시지 제거
	 * @param MessageID 제거할 메시지 ID
	 */
	void RemoveMessageFromListView(const FGuid& MessageID);

	/**
	 * ListView 맨 아래로 스크롤
	 */
	void ScrollToBottom();

	/**
	 * 에러 메시지 표시 (토스트 형태)
	 * @param ErrorMessage 에러 메시지
	 */
	void ShowError(const FString& ErrorMessage);

protected:
	/** 컨트롤러 참조 */
	UPROPERTY()
	TObjectPtr<UMVE_StageLevel_WidgetController_Chat> Controller;

	/** 메시지 엔트리 리스트 (ListView 데이터) */
	UPROPERTY()
	TArray<TObjectPtr<UMVE_ChatMessageEntry>> MessageEntries;

	/** 메시지 ID -> 엔트리 맵 (빠른 검색용) */
	UPROPERTY()
	TMap<FGuid, TObjectPtr<UMVE_ChatMessageEntry>> MessageEntryMap;

public:
	//~ 위젯 참조 (BindWidget)
	/** 메시지 리스트 뷰 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Chat|Widgets")
	TObjectPtr<UListView> MessageListView;

	/** 메시지 입력창 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Chat|Widgets")
	TObjectPtr<UEditableTextBox> MessageInputBox;

	/** 전송 버튼 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Chat|Widgets")
	TObjectPtr<UButton> SendButton;

	/** 에러 표시용 Border (Optional) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Chat|Widgets")
	TObjectPtr<UBorder> ErrorBorder;

	/** 에러 표시용 TextBlock (Optional) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Chat|Widgets")
	TObjectPtr<UTextBlock> ErrorText;

protected:
	//~ 내부 메서드
	/**
	 * 입력창 초기화 (이벤트 바인딩)
	 */
	void SetupInputBox();

	/**
	 * 전송 버튼 초기화 (이벤트 바인딩)
	 */
	void SetupSendButton();

	/**
	 * 메시지 전송 처리 (Enter 키 또는 버튼 클릭)
	 */
	UFUNCTION()
	void HandleSendMessage();

	/**
	 * 입력창 텍스트 변경 시
	 */
	UFUNCTION()
	void OnInputTextChanged(const FText& Text);

	/**
	 * 입력창에서 Enter 키 입력 시
	 */
	UFUNCTION()
	void OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/**
	 * 에러 표시 타이머 콜백
	 */
	void HideError();

	/** 에러 표시 타이머 핸들 */
	FTimerHandle ErrorTimerHandle;
};
