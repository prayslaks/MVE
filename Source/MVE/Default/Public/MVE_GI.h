// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVE_API_ResponseData.h"
#include "Engine/GameInstance.h"
#include "MVE_GI.generated.h"

struct FUser;

UCLASS()
class MVE_API UMVE_GI : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FUser GetUserData() const { return UserData; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetUserData(const FUser Value) { UserData = Value; }

private:
	UPROPERTY()
	FUser UserData = FUser();
};