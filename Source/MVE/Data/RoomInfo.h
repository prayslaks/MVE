#pragma once

#include "CoreMinimal.h"
#include "RoomInfo.generated.h"

/**
 * 방 정보 구조체
 * 네트워크에서 받아온 순수 데이터
 */

USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BroadcastTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ConcertType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewerCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxViewers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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

// 테스트용 Room 정보들을 담은 데이터 테이블
// 나중에 안 쓸 예정
USTRUCT(BlueprintType)
struct FDTRoomInfo : public FTableRowBase
{
	GENERATED_BODY()

	FDTRoomInfo() : RoomInfo(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRoomInfo RoomInfo;
};