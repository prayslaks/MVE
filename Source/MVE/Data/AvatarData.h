#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "AvatarData.generated.h"

USTRUCT(BlueprintType)
struct FAvatarData
{
	GENERATED_BODY()

	UPROPERTY(BluePrintReadWrite)
	FString FileName;

	UPROPERTY(BluePrintReadWrite)
	FString UniqueID;

	UPROPERTY(BluePrintReadWrite)
	FString FilePath;

	UPROPERTY(BluePrintReadWrite)
	FDateTime SavedDate;

	UPROPERTY(BluePrintReadWrite)
	TObjectPtr<UTexture2D> ThumbnailTexture;

	FAvatarData()
		: FileName(TEXT(""))
		, UniqueID(TEXT(""))
		, FilePath(TEXT(""))
		, SavedDate(FDateTime::Now())
		, ThumbnailTexture(nullptr)
	{}
};