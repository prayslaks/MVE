#pragma once

#include "CoreMinimal.h"
#include "AvatarData.generated.h"

USTRUCT(BlueprintType)
struct FAvatarData
{
	GENERATED_BODY()

	UPROPERTY()
	FString FileName;

	UPROPERTY()
	FString UniqueID;

	UPROPERTY()
	FString FilePath;

	UPROPERTY()
	FDateTime SavedDate;

	FAvatarData()
		: FileName(TEXT(""))
		, UniqueID(TEXT(""))
		, FilePath(TEXT(""))
		, SavedDate(FDateTime::Now())
	{}
};