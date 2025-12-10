#pragma once

#include "CoreMinimal.h"
#include "RoomInfo.h"
#include "RoomInfoData.generated.h"

/**
 * ListView에 전달할 방 정보 데이터 객체
 */
UCLASS(BlueprintType)
class MVE_API URoomInfoData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FRoomInfo RoomInfo;
	
	void SetRoomInfo(const FRoomInfo& InRoomInfo)
	{
		RoomInfo = InRoomInfo;
	}
	
	const FRoomInfo& GetRoomInfo() const
	{
		return RoomInfo;
	}
};