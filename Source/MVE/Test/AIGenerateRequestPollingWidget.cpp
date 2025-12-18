#include "Test/AIGenerateRequestPollingWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TimerManager.h"
#include "MVE.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/GameInstance.h"

void UAIGenerateRequestPollingWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (TestGenerateButton)
    {
        TestGenerateButton->OnClicked.AddDynamic(this, &UAIGenerateRequestPollingWidget::OnTestGenerateButtonClicked);
    }
    
    UpdateStatusText(TEXT("Ready"));
}

void UAIGenerateRequestPollingWidget::NativeDestruct()
{
    // Make sure to clear the timer when the widget is destroyed.
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
    }
    Super::NativeDestruct();
}

void UAIGenerateRequestPollingWidget::StartModelGeneration(const FString& Prompt, const FString& ImagePath)
{
    UpdateStatusText(TEXT("모델 생성 요청 중..."));
    FOnGenerateModelComplete OnResult;
    OnResult.BindUObject(this, &UAIGenerateRequestPollingWidget::OnModelGenerationRequested);
    UMVE_API_Helper::GenerateModel(Prompt, ImagePath, OnResult);
}

void UAIGenerateRequestPollingWidget::OnModelGenerationRequested(const bool bSuccess, const FGenerateModelResponseData& Response, const FString& Error)
{
    if (bSuccess && Response.Success)
    {
        CurrentJobId = Response.JobId;
        PRINTLOG(TEXT("Model generation started. Job ID: %s"), *CurrentJobId);
        UpdateStatusText(FString::Printf(TEXT("Generation request sent. Job ID: %s"), *CurrentJobId));
        StartPollingStatus();
    }
    else
    {
        const FString ErrorMessage = FString::Printf(TEXT("Error starting generation: %s"), *Error);
        PRINTLOG(TEXT("%s"), *ErrorMessage);
        UpdateStatusText(ErrorMessage);
        CurrentJobId.Empty();
    }
}

void UAIGenerateRequestPollingWidget::StartPollingStatus()
{
    if (CurrentJobId.IsEmpty())
    {
        UpdateStatusText(TEXT("Error: No Job ID to poll."));
        return;
    }

    // Clear any existing timer before starting a new one
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        // Start a new timer that calls PollStatus every PollingInterval seconds
        GetWorld()->GetTimerManager().SetTimer(PollingTimerHandle, this, &UAIGenerateRequestPollingWidget::PollStatus, PollingInterval, true, 0.0f);
        PollStatus(); // Poll immediately once
    }
}

void UAIGenerateRequestPollingWidget::PollStatus()
{
    if (CurrentJobId.IsEmpty())
    {
        PRINTLOG(TEXT("Attempted to poll status with an empty Job ID."));
        if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        return;
    }
    
    PRINTLOG(TEXT("Polling status for Job ID: %s"), *CurrentJobId);
    UpdateStatusText(FString::Printf(TEXT("Polling status for Job ID: %s..."), *CurrentJobId));
    
    FOnGetJobStatusComplete OnResult;
    OnResult.BindUObject(this, &UAIGenerateRequestPollingWidget::OnStatusPolled);
    
    UMVE_API_Helper::GetModelGenerationStatus(CurrentJobId, OnResult);
}

void UAIGenerateRequestPollingWidget::OnStatusPolled(const bool bSuccess, const FGetJobStatusResponseData& Response, const FString& Error)
{
    if (bSuccess && Response.Success)
    {
        CurrentStatus = Response.Data.Status;
        const FString StatusMessage = FString::Printf(TEXT("Status: %s"), *CurrentStatus);
        UpdateStatusText(StatusMessage);
        PRINTLOG(TEXT("Job %s status: %s"), *CurrentJobId, *StatusMessage);

        // Stop polling if the job is completed or has failed
        if (CurrentStatus.Equals(TEXT("completed"), ESearchCase::IgnoreCase) || CurrentStatus.Equals(TEXT("failed"), ESearchCase::IgnoreCase))
        {
            if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
            
            if(CurrentStatus.Equals(TEXT("completed"), ESearchCase::IgnoreCase))
            {
                UpdateStatusText(FString::Printf(TEXT("Generation complete! Model URL: %s"), *Response.Data.DownloadUrl));
            }
            else // failed
            {
                 UpdateStatusText(FString::Printf(TEXT("Generation failed. Reason: %s"), *Response.Code));
            }
            CurrentJobId.Empty();
        }
    }
    else
    {
        const FString ErrorMessage = FString::Printf(TEXT("Error polling status: %s"), *Error);
        PRINTLOG(TEXT("%s"), *ErrorMessage);
        UpdateStatusText(ErrorMessage);
        // Stop polling on error
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        }
        CurrentJobId.Empty();
    }
}

void UAIGenerateRequestPollingWidget::UpdateStatusText(const FString& InStatus) const
{
    if (StatusTextBlock)
    {
        StatusTextBlock->SetText(FText::FromString(InStatus));
    }
}

void UAIGenerateRequestPollingWidget::OnTestGenerateButtonClicked()
{
    if (CurrentJobId.IsEmpty() == false)
    {
        PRINTLOG(TEXT("A generation job is already in progress."));
        return;
    }
    // For testing purposes, we can use a hardcoded prompt and no image.
    // In a real scenario, this would come from other UI elements.
    const FString TestPrompt = TEXT("A cute cat wearing a wizard hat");
    const FString TestImagePath = TEXT(""); // Empty for no image
    
    StartModelGeneration(TestPrompt, TestImagePath);
}