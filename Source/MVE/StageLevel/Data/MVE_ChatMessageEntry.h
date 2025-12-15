
#pragma once

#include "CoreMinimal.h"
#include "MVE_ChatTypes.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "MVE_ChatMessageEntry.generated.h"

/**
 * ListView용 채팅 메시지 엔트리 오브젝트
 */
UCLASS(BlueprintType)
class MVE_API UMVE_ChatMessageEntry : public UObject
{
	GENERATED_BODY()

public:
	/** 메시지 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FChatMessage MessageData;

	/**
	 * 메시지 엔트리 생성 헬퍼
	 * @param Message 메시지 데이터
	 * @return 생성된 엔트리 오브젝트
	 */
	static UMVE_ChatMessageEntry* Create(UObject* Outer, const FChatMessage& Message)
	{
		UMVE_ChatMessageEntry* Entry = NewObject<UMVE_ChatMessageEntry>(Outer);
		Entry->MessageData = Message;
		return Entry;
	}

	/**
	 * 메시지 ID 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FGuid GetMessageID() const { return MessageData.MessageID; }

	/**
	 * 발신자 이름 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FString GetSenderName() const { return MessageData.SenderName; }

	/**
	 * 메시지 내용 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FString GetMessageContent() const { return MessageData.MessageContent; }

	/**
	 * 타임스탬프 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FDateTime GetTimestamp() const { return MessageData.Timestamp; }

	/**
	 * 타임스탬프 문자열 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FString GetTimestampString() const 
	{ 
		return MessageData.Timestamp.ToString(TEXT("%H:%M:%S")); 
	}

	/**
	 * 메시지 상태 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	EChatMessageState GetMessageState() const { return MessageData.State; }

	/**
	 * Pending 상태 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsPending() const { return MessageData.State == EChatMessageState::Pending; }

	/**
	 * 시스템 메시지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsSystemMessage() const { return MessageData.bIsSystemMessage; }

	/**
	 * 메시지 데이터 업데이트 (Pending -> Confirmed 등)
	 * @param NewMessage 새로운 메시지 데이터
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void UpdateMessage(const FChatMessage& NewMessage)
	{
		MessageData = NewMessage;
	}
};