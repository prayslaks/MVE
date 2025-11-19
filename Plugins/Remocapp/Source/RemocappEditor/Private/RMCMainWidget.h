// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RMCRecorder.h"
#include "RMCMainWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UEditableText;
class UComboBoxString;
class ARMCRecorder;
class UTakeRecorder;

/**
 * RMCMainWidget
 * 
 * Main Remocapp widget which provides user with the recording options
 */
UCLASS()
class REMOCAPPEDITOR_API URMCMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set recorder object communicating with this widget instance
	UFUNCTION(BlueprintCallable, Category = "RemocappMainWidget")
	void SetRecorder(ARMCRecorder* InRecorder);
	
	// Returns the recorder object associated with this widget instance
	UFUNCTION(BlueprintPure, Category = "RemocappMainWidget")
	FORCEINLINE ARMCRecorder* GetRecorder() const { return Recorder; };
	
	// Initializes all the widgets related to recording with the recorder parameters
	UFUNCTION(BlueprintCallable, Category = "RemocappMainWidget")
	void InitializeRecordingOptions();
protected:
	UPROPERTY()
	ARMCRecorder* Recorder;
	
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappMainWidget" )
	void SetCountdown(ERMCRecorderCountdown InCountdown);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappMainWidget" )
	void OnRecordingStarted(UTakeRecorder* TakeRecorder);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappMainWidget" )
	void OnRecordingFinished(UTakeRecorder* TakeRecorder);

	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappMainWidget" )
	void OnRecordingFailed();

	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappMainWidget" )
	void OnAnimationsExported(const TArray<FAnimationExportResult>& AnimationExportResults);
private:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess), Category = "RemocappMainWidget")
	UButton* ToggleRecordingButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess), Category = "RemocappMainWidget")
	UButton* OptionsButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess), Category = "RemocappMainWidget")
	UButton* CloseOptionsButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess), Category = "RemocappMainWidget")
	UEditableText* SceneNameEditableText;
};
