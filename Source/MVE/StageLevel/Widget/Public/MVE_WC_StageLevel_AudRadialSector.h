// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WC_StageLevel_AudRadialSector.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class MVE_API UMVE_WC_StageLevel_AudRadialSector : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="Radial Sector")
	TObjectPtr<UImage> SectorImage;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="Radial Sector")
	TObjectPtr<UTextBlock> SectorTitleTextBlock;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="Radial Sector")
	TObjectPtr<UImage> SectorIconImage;
};