#pragma once

#include "CoreMinimal.h"
#include "InputPromptData.generated.h"

USTRUCT()
struct FInputPromptData
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 UserID;

	UPROPERTY()
	FString PromptMessageText;

	UPROPERTY()
	TArray<uint8> ReferenceImageData;

	UPROPERTY()
	FString ImageFormat;
	
};