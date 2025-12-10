// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h" // Required for UAudioComponent
#include "Template/MVEPlayerController.h"
#include "MVE_API_PCTest.generated.h"

class UMVE_API_PCTest_AudComponent;
class UMVE_API_PCTest_StdComponent;

/**
 * 
 */
UCLASS()
class MVE_API AMVE_API_PCTest : public AMVEPlayerController
{
	GENERATED_BODY()
	
public:
	AMVE_API_PCTest();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVE_API_PCTest_AudComponent> AudComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMVE_API_PCTest_StdComponent> StdComponent;

};
