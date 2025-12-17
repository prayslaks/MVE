// Fill out your copyright notice in the Description page of Project Settings.


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

    if (GetWorld())
    {
        ApiHelper = GetWorld()->GetGameInstance()->GetSubsystem<UMVE_API_Helper>();
    }

    if (!ApiHelper)
    {
        PRINTLOG(Error, TEXT("UAIGenerateRequestPollingWidget: UMVE_API_Helper subsystem not found!"));
    }
    
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
    if (!ApiHelper)
    {
        PRINTLOG(Error, TEXT("API Helper is not available."));
        UpdateStatusText(TEXT("Error: API Helper not found."));
        return;
    }

    UpdateStatusText(TEXT("Requesting model generation..."));

    FOnGenerateModelComplete OnResult;
    OnResult.BindUObject(this, &UAIGenerateRequestPollingWidget::OnModelGenerationRequested);
    
    ApiHelper->GenerateModel(Prompt, ImagePath, OnResult);
}

void UAIGenerateRequestPollingWidget::OnModelGenerationRequested(bool bSuccess, FGenerateModelResponseData Response, const FString& Error)
{
    if (bSuccess && Response.Success)
    {
        CurrentJobId = Response.JobId;
        PRINTLOG(Log, TEXT("Model generation started. Job ID: %s"), *CurrentJobId);
        UpdateStatusText(FString::Printf(TEXT("Generation request sent. Job ID: %s"), *CurrentJobId));
        StartPollingStatus();
    }
    else
    {
        const FString ErrorMessage = FString::Printf(TEXT("Error starting generation: %s"), *Error);
        PRINTLOG(Error, TEXT("%s"), *ErrorMessage);
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
    if (!ApiHelper)
    {
        PRINTLOG(Error, TEXT("API Helper is not available."));
        UpdateStatusText(TEXT("Error: API Helper not found."));
        if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        return;
    }

    if (CurrentJobId.IsEmpty())
    {
        PRINTLOG(Warning, TEXT("Attempted to poll status with an empty Job ID."));
        if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        return;
    }
    
    PRINTLOG(Log, TEXT("Polling status for Job ID: %s"), *CurrentJobId);
    UpdateStatusText(FString::Printf(TEXT("Polling status for Job ID: %s..."), *CurrentJobId));
    
    FOnGetJobStatusComplete OnResult;
    OnResult.BindUObject(this, &UAIGenerateRequestPollingWidget::OnStatusPolled);
    
    ApiHelper->GetModelGenerationStatus(CurrentJobId, OnResult);
}

void UAIGenerateRequestPollingWidget::OnStatusPolled(bool bSuccess, FGetJobStatusResponseData Response, const FString& Error)
{
    if (bSuccess && Response.Success)
    {
        CurrentStatus = Response.Status;
        FString StatusMessage;

        if (CurrentStatus.Equals(TEXT("processing"), ESearchCase::IgnoreCase))
        {
            StatusMessage = FString::Printf(TEXT("Status: %s (%.0f%%)"), *CurrentStatus, Response.Progress);
        }
        else
        {
            StatusMessage = FString::Printf(TEXT("Status: %s"), *CurrentStatus);
        }
        
        UpdateStatusText(StatusMessage);
        PRINTLOG(Log, TEXT("Job %s status: %s"), *CurrentJobId, *StatusMessage);

        // Stop polling if the job is completed or has failed
        if (CurrentStatus.Equals(TEXT("completed"), ESearchCase::IgnoreCase) || CurrentStatus.Equals(TEXT("failed"), ESearchCase::IgnoreCase))
        {
            if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
            
            if(CurrentStatus.Equals(TEXT("completed"), ESearchCase::IgnoreCase))
            {
                UpdateStatusText(FString::Printf(TEXT("Generation complete! Model URL: %s"), *Response.ModelUrl));
            }
            else // failed
            {
                 UpdateStatusText(FString::Printf(TEXT("Generation failed. Reason: %s"), *Response.Error));
            }
            CurrentJobId.Empty();
        }
    }
    else
    {
        const FString ErrorMessage = FString::Printf(TEXT("Error polling status: %s"), *Error);
        PRINTLOG(Error, TEXT("%s"), *ErrorMessage);
        UpdateStatusText(ErrorMessage);
        // Stop polling on error
        if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(PollingTimerHandle);
        CurrentJobId.Empty();
    }
}

void UAIGenerateRequestPollingWidget::UpdateStatusText(const FString& InStatus)
{
    if (StatusTextBlock)
    {
        StatusTextBlock->SetText(FText::FromString(InStatus));
    }
}

void UAIGenerateRequestPollingWidget::OnTestGenerateButtonClicked()
{
    if (!CurrentJobId.IsEmpty())
    {
        PRINTLOG(Warning, TEXT("A generation job is already in progress."));
        return;
    }
    // For testing purposes, we can use a hardcoded prompt and no image.
    // In a real scenario, this would come from other UI elements.
    const FString TestPrompt = TEXT("A cute cat wearing a wizard hat");
    const FString TestImagePath = TEXT(""); // Empty for no image
    
    StartModelGeneration(TestPrompt, TestImagePath);
}