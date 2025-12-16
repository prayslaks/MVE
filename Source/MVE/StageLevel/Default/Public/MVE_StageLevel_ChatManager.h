
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "StageLevel/Data/MVE_ChatTypes.h"
#include "MVE_StageLevel_ChatManager.generated.h"

class APlayerState;

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageReceived, const FChatMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageRejected, const FString&, Reason, EChatRejectReason, RejectType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageRemoved, const FGuid&, MessageID, const FString&, Reason);

/**
 * 채팅 시스템 매니저 (AInfo 기반)
 * - 서버에만 Spawn
 * - 모든 채팅 메시지 관리 및 동기화
 * - AI 필터링 및 Rate Limiting 처리
 */

UCLASS()
class MVE_API AMVE_StageLevel_ChatManager : public AInfo
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_ChatManager();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//~ Rate Limiting 설정
	/** 최대 토큰 수 (버스트 제한) */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|RateLimit")
	float MaxTokens = 5.0f;

	/** 토큰 충전 속도 (초당) */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|RateLimit")
	float TokenRefillRate = 1.0f;

	//~ 메시지 검증 설정
	/** 최대 메시지 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|Validation")
	int32 MaxMessageLength = 200;

	/** 최대 채팅 히스토리 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|History")
	int32 MaxChatHistory = 100;

	//~ AI 필터링 설정
	/** AI 필터링 서버 URL */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|AIFilter")
	FString AIFilterServerURL = TEXT("http://localhost:5000/filter");

	/** AI 필터링 타임아웃 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|AIFilter")
	float AIFilterTimeout = 5.0f;

	/** AI 필터링 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat|AIFilter")
	bool bEnableAIFilter = true;

public:
	//~ 델리게이트
	/** 새 메시지 수신 시 (모든 클라이언트) */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatMessageReceived OnChatMessageReceived;

	/** 메시지 거부 시 (해당 클라이언트만) */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatMessageRejected OnChatMessageRejected;

	/** 메시지 삭제 시 (모든 클라이언트) */
	UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnChatMessageRemoved OnChatMessageRemoved;

protected:
	//~ 리플리케이트 속성
	/** 채팅 히스토리 (신규 접속자를 위한 자동 동기화) */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_ChatHistory)
	TArray<FChatMessage> ChatHistory;

	/** Rate Limit 버킷 맵 (서버 전용) */
	UPROPERTY()
	TMap<APlayerState*, FRateLimitBucket> RateLimits;

	/** Pending 메시지 맵 (AI 필터링 대기 중) */
	UPROPERTY()
	TMap<FGuid, FChatMessage> PendingMessages;

public:
	//~ 클라이언트 API
	/** 
	 * 메시지 전송 (클라이언트 호출)
	 * @param MessageContent 메시지 내용
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendMessage(const FString& MessageContent);

	/**
	 * 채팅 히스토리 가져오기
	 * @return 현재 채팅 히스토리 배열
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	const TArray<FChatMessage>& GetChatHistory() const { return ChatHistory; }

	/**
	 * 시스템 메시지 브로드캐스트 (서버만 호출 가능)
	 * @param SystemMessage 시스템 메시지 내용
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Chat")
	void BroadcastSystemMessage(const FString& SystemMessage);

protected:
	//~ 서버 RPC
	/** 서버로 메시지 전송 요청 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendMessage(const FString& MessageContent, const FGuid& ClientMessageID);

	//~ 멀티캐스트 RPC
	/** 모든 클라이언트에 메시지 브로드캐스트 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastMessage(const FChatMessage& Message);

	/** 모든 클라이언트에 메시지 삭제 요청 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRemoveMessage(const FGuid& MessageID, const FString& Reason);

	//~ 클라이언트 RPC
	/** 특정 클라이언트에 메시지 거부 알림 */
	UFUNCTION(Client, Reliable)
	void ClientNotifyMessageRejected(const FString& Reason, EChatRejectReason RejectType);

protected:
	//~ 리플리케이션 콜백
	UFUNCTION()
	void OnRep_ChatHistory();

protected:
	//~ 내부 메서드
	/**
	 * Rate Limit 체크
	 * @param Player 플레이어 상태
	 * @return 전송 가능 여부
	 */
	bool CheckRateLimit(APlayerState* Player);

	/**
	 * 메시지 기본 검증
	 * @param MessageContent 메시지 내용
	 * @param OutReason 거부 이유 (실패 시)
	 * @return 검증 통과 여부
	 */
	bool ValidateMessage(const FString& MessageContent, FString& OutReason);

	/**
	 * AI 필터링 시작 (비동기)
	 * @param Message 검증할 메시지
	 */
	void FilterMessageAsync(const FChatMessage& Message);

	/**
	 * AI 필터링 완료 콜백
	 * @param MessageID 메시지 ID
	 * @param bPassed 필터링 통과 여부
	 * @param Reason 차단 이유 (차단 시)
	 */
	void OnAIFilterCompleted(const FGuid& MessageID, bool bPassed, const FString& Reason);

	/**
	 * 채팅 히스토리에 메시지 추가
	 * @param Message 추가할 메시지
	 */
	void AddToChatHistory(const FChatMessage& Message);

	/**
	 * 발신자 정보 가져오기
	 * @param Player 플레이어 상태
	 * @param OutName 플레이어 이름
	 * @param OutID 플레이어 고유 ID
	 */
	void GetSenderInfo(APlayerState* Player, FString& OutName, FString& OutID);

private:
	/** HTTP 모듈 참조 */
	TSharedPtr<class IHttpRequest> CurrentFilterRequest;
};
