// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVE_ChatTypes.generated.h"

/**
 * 채팅 메시지 상태
 */
UENUM(BlueprintType)
enum class EChatMessageState : uint8
{
	Pending		UMETA(DisplayName = "Pending"),		// 클라이언트 로컬 큐 대기
	Confirmed	UMETA(DisplayName = "Confirmed"),	// 서버 검증 통과
	Rejected	UMETA(DisplayName = "Rejected")		// 필터링 차단
};

/**
 * 메시지 거부 이유
 */
UENUM(BlueprintType)
enum class EChatRejectReason : uint8
{
	None			UMETA(DisplayName = "None"),
	RateLimit		UMETA(DisplayName = "Rate Limit"),			// Rate Limit 초과
	TooLong			UMETA(DisplayName = "Too Long"),			// 메시지 길이 초과
	InvalidContent	UMETA(DisplayName = "Invalid Content"),		// 기본 검증 실패
	AIFiltered		UMETA(DisplayName = "AI Filtered")			// AI 필터링 차단
};

/**
 * 채팅 메시지 구조체
 */
USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

	/** 발신자 이름 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	FString SenderName;

	/** 발신자 고유 ID (PlayerState UniqueID) */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	FString SenderID;

	/** 메시지 내용 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	FString MessageContent;

	/** 타임스탬프 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;

	/** 시스템 메시지 여부 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	bool bIsSystemMessage;

	/** AI 필터링 통과 여부 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	bool bIsFiltered;

	/** 메시지 상태 */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	EChatMessageState State;

	/** 메시지 고유 ID (서버에서 생성) */
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
	FGuid MessageID;

	FChatMessage()
		: SenderName(TEXT(""))
		, SenderID(TEXT(""))
		, MessageContent(TEXT(""))
		, Timestamp(FDateTime::Now())
		, bIsSystemMessage(false)
		, bIsFiltered(false)
		, State(EChatMessageState::Pending)
		, MessageID(FGuid::NewGuid())
	{
	}

	FChatMessage(const FString& InSenderName, const FString& InSenderID, const FString& InContent)
		: SenderName(InSenderName)
		, SenderID(InSenderID)
		, MessageContent(InContent)
		, Timestamp(FDateTime::Now())
		, bIsSystemMessage(false)
		, bIsFiltered(false)
		, State(EChatMessageState::Pending)
		, MessageID(FGuid::NewGuid())
	{
	}

	/** 시스템 메시지 생성 헬퍼 */
	static FChatMessage CreateSystemMessage(const FString& InContent)
	{
		FChatMessage Msg;
		Msg.SenderName = TEXT("System");
		Msg.SenderID = TEXT("SYSTEM");
		Msg.MessageContent = InContent;
		Msg.bIsSystemMessage = true;
		Msg.State = EChatMessageState::Confirmed;
		return Msg;
	}
};

/**
 * Rate Limit 토큰 버킷 구조체
 */
USTRUCT()
struct FRateLimitBucket
{
	GENERATED_BODY()

	/** 현재 토큰 수 */
	UPROPERTY()
	float Tokens;

	/** 마지막 토큰 충전 시간 */
	UPROPERTY()
	FDateTime LastRefill;

	FRateLimitBucket()
		: Tokens(5.0f)
		, LastRefill(FDateTime::UtcNow())
	{
	}
};