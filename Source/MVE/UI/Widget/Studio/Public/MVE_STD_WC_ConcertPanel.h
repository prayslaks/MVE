// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_ConcertPanel.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class MVE_API UMVE_STD_WC_ConcertPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> OpenToggleButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> OpenToggleButtonTextBlock;
	
	UPROPERTY()
	bool IsOpen;
	
	UFUNCTION()
	void OnOpenToggleButtonClicked();

protected:
	virtual void NativeConstruct() override;
};
