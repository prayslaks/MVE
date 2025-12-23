// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STU_WC_ConcertStudioPanel.generated.h"

class UMVE_WC_Chat;
class UMVE_STU_WidgetController_StudioConcert;
class UglTFRuntimeAsset;
class UMVE_STD_WC_AudioPlayer;
class UMVE_STD_WC_AudioSearch;
struct FMVE_STD_AudioSearchResultData; // Forward declare
class USoundWave; // Forward declare

UCLASS()
class MVE_API UMVE_STU_WC_ConcertStudioPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* Chat 위젯 */
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_WC_Chat> ChatWidget;

	/** AudioSearch 위젯 */
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioSearch> AudioSearch;

	/** AudioPlayer 위젯 */
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_STD_WC_AudioPlayer> AudioPlayer;

protected:

	
private:
	/** Audio 관련 모든 로직을 담당하는 Controller */
	UPROPERTY()
	TObjectPtr<UMVE_STU_WidgetController_StudioConcert> AudioController;
};
