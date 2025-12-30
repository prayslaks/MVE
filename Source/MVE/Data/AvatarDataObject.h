#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AvatarData.h"
#include "AvatarDataObject.generated.h"

/**
 * UTileView에 전달하기 위한 AvatarData 래퍼 클래스
 * TileView는 UObject 기반 데이터만 받기 때문에 FAvatarData를 래핑
 */
UCLASS()
class MVE_API UAvatarDataObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FAvatarData AvatarData;
};
