#pragma once

#include "CoreMinimal.h"
#include "RoomInfo.generated.h"

/**
 * 방 정보 구조체
 * 네트워크/DB에서 받아온 순수 데이터
 */

USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString RoomID;

	UPROPERTY(BlueprintReadWrite)
	FString RoomTitle;

	UPROPERTY(BlueprintReadWrite)
	FDateTime BroadcastTime;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Thumbnail;

	UPROPERTY(BlueprintReadWrite)
	FString ConcertType;

	UPROPERTY(BlueprintReadWrite)
	int32 ViewerCount;

	UPROPERTY(BlueprintReadWrite)
	bool bIsLive;

	UPROPERTY(BlueprintReadWrite)
	bool bIsFeatured;

	UPROPERTY(BlueprintReadWrite)
	bool bIsNew;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor RoomColor;

	// 기본 생성자
	FRoomInfo()
		: RoomID(TEXT(""))
		, RoomTitle(TEXT(""))
		, BroadcastTime(FDateTime::Now())
		, Thumbnail()
		, ConcertType(TEXT(""))
		, ViewerCount(0)
		, bIsLive(false)
		, bIsFeatured(false)
		, bIsNew(false)
	{}
};