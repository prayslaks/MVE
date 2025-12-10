// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioPanel.generated.h"

class UglTFRuntimeAsset;
class UMVE_STD_WC_AudioPlayer;
class UMVE_STD_WC_AudioSearch;
struct FMVE_STD_AudioSearchResultData; // Forward declare
class USoundWave; // Forward declare

UCLASS()
class MVE_API UMVE_STD_WC_AudioPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION()
	void HandleAudioSearchResultSelected(const FMVE_STD_AudioSearchResultData& AudioData);

	void LoadAudioFromPresignedUrlAndSetPlayer(const FMVE_STD_AudioSearchResultData& AudioData);

public:	
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioSearch> AudioSearch;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioPlayer> AudioPlayer;
	
private:
	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandleStopClicked();
};
