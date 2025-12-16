// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WC_InteractionPanel.generated.h"

class UMVE_WC_Chat;
class UButton;
class USoundBase;

UCLASS()
class MVE_API UMVE_AUD_WC_InteractionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMVE_WC_Chat> ChatWidget;

protected:
	virtual void NativeConstruct() override;

};
