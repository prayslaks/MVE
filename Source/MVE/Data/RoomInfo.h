#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MVE_API_ResponseData.h"
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

/**
 * 광고용 콘서트 정보를 담은 데이터 테이블 Row
 * 썸네일, 제목, 광고주 이름을 설정하여 TileView에 광고로 표시
 */
USTRUCT(BlueprintType)
struct FDTAdvertisementConcert : public FTableRowBase
{
	GENERATED_BODY()

	FDTAdvertisementConcert(){}

	// 광고 콘서트 제목 (ConcertName으로 매핑)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advertisement")
	FString AdTitle;

	// 광고주 이름 (StudioName으로 매핑)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advertisement")
	FString AdvertiserName;

	// 썸네일 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advertisement")
	TSoftObjectPtr<UTexture2D> ThumbnailTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advertisement")
	FString AdvertisementText;

	// 광고 클릭 가능 여부 (false면 클릭 방지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advertisement")
	bool bIsClickable = false;
};