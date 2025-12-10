// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVE_GIS_API.generated.h"

UCLASS()
class MVE_API UMVE_GIS_API : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets a localized error message for a given error code.
	 * @param ErrorCode The error code from the API response.
	 * @return The localized error message as FText. Returns a default message if the code is not found.
	 */
	FText GetTranslatedErrorMessage(const FString& ErrorCode) const;
	
	static UMVE_GIS_API* Get(const UObject* WorldContextObject);
	
	// 현재 참가 중인 콘서트 룸 ID
	UPROPERTY(BlueprintReadWrite)
	FString CurrentRoomId;

protected:
	// Called when the subsystem is initialized
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	// Map of error codes to localized error messages
	UPROPERTY()
	TMap<FString, FText> ResponseCodeToKoreanTextMap;

	// Initializes the ResponseCodeToKoreanTextMap with values
	void MapResponseCodeToText();
	
};
