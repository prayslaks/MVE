// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioPanel.generated.h"

class UMVE_STU_WidgetController_Audio;
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
	virtual void NativeDestruct() override;

protected:
	/** AudioSearch 위젯 */
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioSearch> AudioSearch;
    
	/** AudioPlayer 위젯 */
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioPlayer> AudioPlayer;

private:
	/** Audio 관련 모든 로직을 담당하는 Controller */
	UPROPERTY()
	TObjectPtr<UMVE_STU_WidgetController_Audio> AudioController;
};
