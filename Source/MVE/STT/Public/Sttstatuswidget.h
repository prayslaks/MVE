// STTStatusWidget.h - STT 상태 표시 UI 위젯
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "STTSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "STTStatusWidget.generated.h"

// ================================================================================================
// 명령어 히스토리 항목 구조체
// ================================================================================================

USTRUCT(BlueprintType)
struct FSTTHistoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Text;

    UPROPERTY(BlueprintReadWrite)
    ESTTCommandType CommandType = ESTTCommandType::None;

    UPROPERTY(BlueprintReadWrite)
    FDateTime Timestamp;

    FSTTHistoryEntry()
        : Timestamp(FDateTime::Now())
    {}

    FSTTHistoryEntry(const FString& InText, ESTTCommandType InCommand)
        : Text(InText)
        , CommandType(InCommand)
        , Timestamp(FDateTime::Now())
    {}
};

// ================================================================================================
// STT 상태 위젯
// ================================================================================================

UCLASS()
class MVE_API USTTStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ================================================================================================
    // UI 바인딩 (블루프린트에서 연결)
    // ================================================================================================

    // 연결 상태 표시
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ConnectionStatusText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> ConnectionIndicator;

    // 마지막 인식된 텍스트
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> LastRecognizedText;

    // 현재 명령어 표시
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CurrentCommandText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UBorder> CommandHighlightBorder;

    // 오디오 레벨 미터
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> AudioLevelBar;

    // 명령어별 아이콘/표시기
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> PlayIcon;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> StopIcon;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> NextIcon;

    // 히스토리 컨테이너
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> HistoryContainer;

    // ================================================================================================
    // 설정
    // ================================================================================================

    // 히스토리 최대 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Settings")
    int32 MaxHistoryCount = 5;

    // 명령어 하이라이트 지속 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Settings")
    float CommandHighlightDuration = 2.0f;

    // 색상 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Colors")
    FLinearColor ConnectedColor = FLinearColor(0.0f, 1.0f, 0.3f, 1.0f);  // 초록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Colors")
    FLinearColor DisconnectedColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);  // 빨강

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Colors")
    FLinearColor PlayCommandColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);  // 초록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Colors")
    FLinearColor StopCommandColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);  // 주황

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT Widget|Colors")
    FLinearColor NextCommandColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);  // 파랑

    // ================================================================================================
    // 상태
    // ================================================================================================

    UPROPERTY(BlueprintReadOnly, Category = "STT Widget|Status")
    TArray<FSTTHistoryEntry> CommandHistory;

    UPROPERTY(BlueprintReadOnly, Category = "STT Widget|Status")
    bool bIsConnected = false;

    // ================================================================================================
    // 블루프린트 호출 가능 함수
    // ================================================================================================

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void UpdateConnectionStatus(bool bConnected);

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void UpdateRecognizedText(const FString& Text);

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void ShowCommand(ESTTCommandType CommandType, const FString& OriginalText);

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void UpdateAudioLevel(float NormalizedLevel);

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void ClearHistory();

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void StartSTT();

    UFUNCTION(BlueprintCallable, Category = "STT Widget")
    void StopSTT();

    // ================================================================================================
    // 블루프린트 구현 가능 이벤트
    // ================================================================================================

    // 명령어 수신 시 블루프린트에서 추가 처리 가능
    UFUNCTION(BlueprintImplementableEvent, Category = "STT Widget|Events")
    void OnCommandReceived(ESTTCommandType CommandType, const FString& Text);

    // 연결 상태 변경 시
    UFUNCTION(BlueprintImplementableEvent, Category = "STT Widget|Events")
    void OnConnectionChanged(bool bConnected);

protected:
    // ================================================================================================
    // 내부 함수
    // ================================================================================================

    UFUNCTION()
    void HandleSTTTextStream(const FSTTResponse& Response);

    UFUNCTION()
    void HandleSTTCommand(ESTTCommandType CommandType, const FString& OriginalText);

    UFUNCTION()
    void HandleConnectionStatus(bool bConnected);

    UFUNCTION()
    void HandleAudioLevel(float Level);

    UFUNCTION()
    void HandleSTTError(const FString& Error);

    void AddToHistory(const FString& Text, ESTTCommandType CommandType);
    void UpdateHistoryDisplay();
    FLinearColor GetCommandColor(ESTTCommandType CommandType) const;
    void ResetCommandHighlight();

    USTTSubsystem* GetSTTSubsystem() const;

private:
    FTimerHandle HighlightTimerHandle;
};