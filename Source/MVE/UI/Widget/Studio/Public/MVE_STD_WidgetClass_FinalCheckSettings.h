
#pragma once

#include "CoreMinimal.h"
#include "MVE_API_ResponseData.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WidgetClass_FinalCheckSettings.generated.h"

class UMVE_STU_WC_EffectSequencePreview;
class UMVE_STD_WC_PlaylistBuilder;
class UEditableTextBox;
class UButton;



UCLASS()
class MVE_API UMVE_STD_WidgetClass_FinalCheckSettings : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartConcertButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMVE_STD_WC_PlaylistBuilder> PlaylistBuilderWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> RoomTitleEditableText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMVE_STU_WC_EffectSequencePreview> EffectSequencePreviewWidget;
	
private:
	UFUNCTION(BlueprintCallable)
	void OnStartConcertButtonClicked();

	UFUNCTION()
	void OnAudioFileSelected(const FAudioFile& SelectedAudio);

	UFUNCTION()
	void OnBatchAnalyzeRequested();

	UFUNCTION()
	void OnMusicAnalysisReceived(bool bSuccess, const TArray<struct FEffectSequenceData>& SequenceData, const FString& ErrorMessage);

	// 현재 선택된 오디오 파일 (AI 분석 결과 저장 시 AudioId로 사용)
	UPROPERTY()
	FAudioFile CurrentSelectedAudio;
};
