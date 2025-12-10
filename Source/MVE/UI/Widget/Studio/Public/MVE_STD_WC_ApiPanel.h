// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_ApiPanel.generated.h"

class UMVE_STD_WC_ConcertPanel;
class UMVE_STD_WC_AudioPanel;

UCLASS()
class MVE_API UMVE_STD_WC_ApiPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioPanel> AudioPanel;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_ConcertPanel> ConcertPanel;
};
