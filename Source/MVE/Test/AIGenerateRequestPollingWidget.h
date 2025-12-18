// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "API/Public/MVE_API_Helper.h"
#include "API/Public/MVE_API_ResponseData.h"
#include "AIGenerateRequestPollingWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class MVE_API UAIGenerateRequestPollingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "AI Generate")
    void StartModelGeneration(const FString& Prompt, const FString& ImagePath);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void OnModelGenerationRequested(const bool bSuccess, const FGenerateModelResponseData& Response, const FString& Error);
    
    void StartPollingStatus();
    void PollStatus();
    
    void OnStatusPolled(const bool bSuccess, const FGetJobStatusResponseData& Response, const FString& Error);

    void UpdateStatusText(const FString& InStatus) const;

    // Bind this to a TextBlock widget in your UMG Designer to see the status.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusTextBlock;

    // A test button to trigger the generation for debugging. Bind it in UMG Designer.
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> TestGenerateButton;

    UFUNCTION()
    void OnTestGenerateButtonClicked();
    
    FString CurrentJobId;
    FString CurrentStatus;
    
    FTimerHandle PollingTimerHandle;

    // This property can be set from the Blueprint editor to configure the polling interval.
    UPROPERTY(EditAnywhere, Category = "AI Generate", meta=(ClampMin="1.0", UIMin="1.0"))
    float PollingInterval = 5.0f;
};