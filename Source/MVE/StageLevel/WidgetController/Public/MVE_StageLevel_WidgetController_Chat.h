
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MVE_StageLevel_WidgetController_Chat.generated.h"

struct FChatMessage;
class AMVE_StageLevel_ChatManager;

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageAdded, const FChatMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatSendFailed, const FString&, Reason, EChatRejectReason, RejectType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageRemovedWC, const FGuid&, MessageID);


UCLASS()
class MVE_API UMVE_StageLevel_WidgetController_Chat : public UObject
{
	GENERATED_BODY()

public:
	UMVE_StageLevel_WidgetController_Chat();

	//~ 초기화 및 정리
	/**
	 * 컨트롤러 초기화
	 * @param InWorld 월드 컨텍스트
	 * @param bAutoFindManager ChatManager 자동 검색 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Initialize(bool bAutoFindManager = true);

	/**
	 * 컨트롤러 정리
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Shutdown();

	/**
	 * ChatManager 수동 설정
	 * @param InChatManager 연결할 ChatManager
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetChatManager(AMVE_StageLevel_ChatManager* InChatManager);

public:
	//~ 델리게이트 (UI가 구독)
	/** 새 메시지 추가 시 */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatMessageAdded OnChatMessageAdded;

	/** 메시지 전송 실패 시 */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatSendFailed OnChatSendFailed;

	/** 메시지 제거 시 */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatMessageRemovedWC OnChatMessageRemoved;

public:
	//~ Public API
	/**
	 * 메시지 전송
	 * @param MessageContent 메시지 내용
	 * @return 전송 성공 여부 (로컬 검증)
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool SendMessage(const FString& MessageContent);

	/**
	 * 현재 채팅 히스토리 가져오기
	 * @return 채팅 메시지 배열
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	TArray<FChatMessage> GetChatHistory() const;

	/**
	 * ChatManager 연결 상태 확인
	 * @return 연결 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsConnected() const { return ChatManager != nullptr; }

	/**
	 * Pending 메시지 큐 가져오기
	 * @return Pending 메시지 배열
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	const TArray<FChatMessage>& GetPendingMessages() const { return PendingMessages; }

protected:
	//~ ChatManager 델리게이트 콜백
	UFUNCTION()
	void OnMessageReceived(const FChatMessage& Message);

	UFUNCTION()
	void OnMessageRejected(const FString& Reason, EChatRejectReason RejectType);

	UFUNCTION()
	void OnMessageRemovedFromServer(const FGuid& MessageID, const FString& Reason);

protected:
	//~ 내부 메서드
	/**
	 * ChatManager 자동 검색
	 * @return 찾은 ChatManager (없으면 nullptr)
	 */
	AMVE_StageLevel_ChatManager* FindChatManager();

	/**
	 * Pending 큐에 메시지 추가
	 * @param Message 추가할 메시지
	 */
	void AddPendingMessage(const FChatMessage& Message);

	/**
	 * Pending 큐에서 메시지 제거 및 확정 처리
	 * @param MessageID 메시지 ID
	 * @return 제거된 메시지 (없으면 빈 Optional)
	 */
	TOptional<FChatMessage> RemovePendingMessage(const FGuid& MessageID);

	/**
	 * 로컬 메시지 검증
	 * @param MessageContent 메시지 내용
	 * @param OutReason 실패 이유 (실패 시)
	 * @return 검증 통과 여부
	 */
	bool ValidateMessageLocally(const FString& MessageContent, FString& OutReason);

private:
	/** 연결된 ChatManager */
	UPROPERTY()
	TObjectPtr<AMVE_StageLevel_ChatManager> ChatManager;

	/** Pending 메시지 큐 (서버 응답 대기 중) */
	UPROPERTY()
	TArray<FChatMessage> PendingMessages;

	/** ChatManager 검색 타이머 핸들 */
	FTimerHandle FindManagerTimerHandle;

	/** 최대 Pending 큐 크기 */
	static constexpr int32 MAX_PENDING_MESSAGES = 10;
};
