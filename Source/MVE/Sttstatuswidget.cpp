// STTStatusWidget.cpp - STT 상태 표시 UI 위젯 구현
#include "STTStatusWidget.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "TimerManager.h"

// ================================================================================================
// 생성/소멸
// ================================================================================================

void USTTStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // STT 서브시스템 이벤트 바인딩
    if (USTTSubsystem* STT = GetSTTSubsystem())
    {
        STT->OnSTTTextStream.AddDynamic(this, &USTTStatusWidget::HandleSTTTextStream);
        STT->OnSTTCommandReceived.AddDynamic(this, &USTTStatusWidget::HandleSTTCommand);
        STT->OnConnectionStatusChanged.AddDynamic(this, &USTTStatusWidget::HandleConnectionStatus);
        STT->OnAudioLevelChanged.AddDynamic(this, &USTTStatusWidget::HandleAudioLevel);
        STT->OnSTTError.AddDynamic(this, &USTTStatusWidget::HandleSTTError);

        // 현재 상태로 초기화
        UpdateConnectionStatus(STT->bIsConnected);

        PRINTLOG(TEXT("[STT Widget] 이벤트 바인딩 완료"));
    }
    else
    {
        PRINTLOG(TEXT("[STT Widget] 경고: STT 서브시스템을 찾을 수 없음"));
    }

    // 초기 UI 상태
    if (AudioLevelBar)
    {
        AudioLevelBar->SetPercent(0.0f);
    }

    if (CurrentCommandText)
    {
        CurrentCommandText->SetText(FText::FromString(TEXT("대기 중...")));
    }

    // 아이콘 초기화 (모두 비활성)
    if (PlayIcon) PlayIcon->SetOpacity(0.3f);
    if (StopIcon) StopIcon->SetOpacity(0.3f);
    if (NextIcon) NextIcon->SetOpacity(0.3f);
}

void USTTStatusWidget::NativeDestruct()
{
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HighlightTimerHandle);
    }

    // 이벤트 바인딩 해제
    if (USTTSubsystem* STT = GetSTTSubsystem())
    {
        STT->OnSTTTextStream.RemoveAll(this);
        STT->OnSTTCommandReceived.RemoveAll(this);
        STT->OnConnectionStatusChanged.RemoveAll(this);
        STT->OnAudioLevelChanged.RemoveAll(this);
        STT->OnSTTError.RemoveAll(this);
    }

    Super::NativeDestruct();
}

// ================================================================================================
// 공개 함수
// ================================================================================================

void USTTStatusWidget::StartSTT()
{
    if (USTTSubsystem* STT = GetSTTSubsystem())
    {
        STT->StartTranscribing();
    }
}

void USTTStatusWidget::StopSTT()
{
    if (USTTSubsystem* STT = GetSTTSubsystem())
    {
        STT->StopTranscribing();
    }
}

void USTTStatusWidget::UpdateConnectionStatus(bool bConnected)
{
    bIsConnected = bConnected;

    if (ConnectionStatusText)
    {
        FString StatusStr = bConnected ? TEXT("● 연결됨") : TEXT("○ 연결 끊김");
        ConnectionStatusText->SetText(FText::FromString(StatusStr));
        ConnectionStatusText->SetColorAndOpacity(bConnected ? ConnectedColor : DisconnectedColor);
    }

    if (ConnectionIndicator)
    {
        ConnectionIndicator->SetColorAndOpacity(bConnected ? ConnectedColor : DisconnectedColor);
    }

    // 블루프린트 이벤트 호출
    OnConnectionChanged(bConnected);
}

void USTTStatusWidget::UpdateRecognizedText(const FString& Text)
{
    if (LastRecognizedText)
    {
        LastRecognizedText->SetText(FText::FromString(Text));
    }
}

void USTTStatusWidget::ShowCommand(ESTTCommandType CommandType, const FString& OriginalText)
{
    // 현재 명령어 텍스트 업데이트
    if (CurrentCommandText)
    {
        FString CommandDisplay = FString::Printf(TEXT("🎯 %s: \"%s\""),
            *USTTSubsystem::GetCommandDisplayName(CommandType),
            *OriginalText);
        CurrentCommandText->SetText(FText::FromString(CommandDisplay));
    }

    // 명령어 하이라이트 색상
    FLinearColor HighlightColor = GetCommandColor(CommandType);

    if (CommandHighlightBorder)
    {
        CommandHighlightBorder->SetBrushColor(HighlightColor);
    }

    // 아이콘 활성화
    if (PlayIcon) PlayIcon->SetOpacity(CommandType == ESTTCommandType::PlayTrack ? 1.0f : 0.3f);
    if (StopIcon) StopIcon->SetOpacity(CommandType == ESTTCommandType::StopTrack ? 1.0f : 0.3f);
    if (NextIcon) NextIcon->SetOpacity(CommandType == ESTTCommandType::NextTrack ? 1.0f : 0.3f);

    // 히스토리에 추가
    AddToHistory(OriginalText, CommandType);

    // 일정 시간 후 하이라이트 해제
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HighlightTimerHandle);
        World->GetTimerManager().SetTimer(
            HighlightTimerHandle,
            this,
            &USTTStatusWidget::ResetCommandHighlight,
            CommandHighlightDuration,
            false
        );
    }

    // 블루프린트 이벤트 호출
    OnCommandReceived(CommandType, OriginalText);

    PRINTLOG(TEXT("[STT Widget] 명령어 표시: %s"), *USTTSubsystem::GetCommandDisplayName(CommandType));
}

void USTTStatusWidget::UpdateAudioLevel(float NormalizedLevel)
{
    if (AudioLevelBar)
    {
        // 부드러운 감쇠 효과
        float CurrentPercent = AudioLevelBar->GetPercent();
        float TargetPercent = FMath::Clamp(NormalizedLevel * 5.0f, 0.0f, 1.0f);  // 증폭

        // 상승은 빠르게, 하강은 천천히
        float NewPercent;
        if (TargetPercent > CurrentPercent)
        {
            NewPercent = FMath::Lerp(CurrentPercent, TargetPercent, 0.5f);
        }
        else
        {
            NewPercent = FMath::Lerp(CurrentPercent, TargetPercent, 0.1f);
        }

        AudioLevelBar->SetPercent(NewPercent);

        // 레벨에 따른 색상 변경
        FLinearColor LevelColor;
        if (NewPercent > 0.8f)
        {
            LevelColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);  // 빨강 (클리핑 경고)
        }
        else if (NewPercent > 0.5f)
        {
            LevelColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);  // 초록 (적정)
        }
        else
        {
            LevelColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);  // 회색 (낮음)
        }

        AudioLevelBar->SetFillColorAndOpacity(LevelColor);
    }
}

void USTTStatusWidget::ClearHistory()
{
    CommandHistory.Empty();
    UpdateHistoryDisplay();
}

// ================================================================================================
// 이벤트 핸들러
// ================================================================================================

void USTTStatusWidget::HandleSTTTextStream(const FSTTResponse& Response)
{
    UpdateRecognizedText(Response.TranscribedText);
}

void USTTStatusWidget::HandleSTTCommand(ESTTCommandType CommandType, const FString& OriginalText)
{
    ShowCommand(CommandType, OriginalText);
}

void USTTStatusWidget::HandleConnectionStatus(bool bConnected)
{
    UpdateConnectionStatus(bConnected);
}

void USTTStatusWidget::HandleAudioLevel(float Level)
{
    UpdateAudioLevel(Level);
}

void USTTStatusWidget::HandleSTTError(const FString& Error)
{
    if (LastRecognizedText)
    {
        LastRecognizedText->SetText(FText::FromString(FString::Printf(TEXT("⚠️ 오류: %s"), *Error)));
        LastRecognizedText->SetColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f));
    }
}

// ================================================================================================
// 내부 함수
// ================================================================================================

void USTTStatusWidget::AddToHistory(const FString& Text, ESTTCommandType CommandType)
{
    // 새 항목 추가
    FSTTHistoryEntry NewEntry(Text, CommandType);
    CommandHistory.Insert(NewEntry, 0);

    // 최대 개수 유지
    while (CommandHistory.Num() > MaxHistoryCount)
    {
        CommandHistory.Pop();
    }

    UpdateHistoryDisplay();
}

void USTTStatusWidget::UpdateHistoryDisplay()
{
    if (!HistoryContainer)
    {
        return;
    }

    // 기존 자식 위젯 제거
    HistoryContainer->ClearChildren();

    // 히스토리 항목 추가
    for (const FSTTHistoryEntry& Entry : CommandHistory)
    {
        // 간단한 텍스트 블록 생성
        UTextBlock* HistoryText = NewObject<UTextBlock>(this);
        if (HistoryText)
        {
            FString TimeStr = Entry.Timestamp.ToString(TEXT("%H:%M:%S"));
            FString CommandName = USTTSubsystem::GetCommandDisplayName(Entry.CommandType);

            FString DisplayText = FString::Printf(TEXT("[%s] %s: %s"),
                *TimeStr,
                *CommandName,
                *Entry.Text);

            HistoryText->SetText(FText::FromString(DisplayText));
            HistoryText->SetColorAndOpacity(GetCommandColor(Entry.CommandType));

            FSlateFontInfo FontInfo = HistoryText->GetFont();
            FontInfo.Size = 12;
            HistoryText->SetFont(FontInfo);

            HistoryContainer->AddChild(HistoryText);
        }
    }
}

FLinearColor USTTStatusWidget::GetCommandColor(ESTTCommandType CommandType) const
{
    switch (CommandType)
    {
    case ESTTCommandType::PlayTrack:
        return PlayCommandColor;
    case ESTTCommandType::StopTrack:
        return StopCommandColor;
    case ESTTCommandType::NextTrack:
        return NextCommandColor;
    default:
        return FLinearColor::White;
    }
}

void USTTStatusWidget::ResetCommandHighlight()
{
    // 아이콘 모두 비활성
    if (PlayIcon) PlayIcon->SetOpacity(0.3f);
    if (StopIcon) StopIcon->SetOpacity(0.3f);
    if (NextIcon) NextIcon->SetOpacity(0.3f);

    // 하이라이트 색상 초기화
    if (CommandHighlightBorder)
    {
        CommandHighlightBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f));
    }

    // 텍스트 초기화
    if (CurrentCommandText)
    {
        CurrentCommandText->SetText(FText::FromString(TEXT("대기 중...")));
    }
}

USTTSubsystem* USTTStatusWidget::GetSTTSubsystem() const
{
    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        return GI->GetSubsystem<USTTSubsystem>();
    }
    return nullptr;
}