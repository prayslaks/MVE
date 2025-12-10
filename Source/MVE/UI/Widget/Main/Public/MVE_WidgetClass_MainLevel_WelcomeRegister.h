// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_WelcomeRegister.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_WelcomeRegister : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveLoginButton;
	
	UFUNCTION()
	void OnMoveLoginButtonClicked();
};
