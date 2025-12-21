// STTTestActor.cpp - 수정된 버전 (UI 연동 + 명령어 처리)
#include "STTTestActor.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

ASTTTestActor::ASTTTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASTTTestActor::BeginPlay()
{
    Super::BeginPlay();

    if (USTTSubsystem* Subsystem = GetSTTSubsystem())
    {
        // 이벤트 바인딩
        Subsystem->OnSTTTextStream.AddDynamic(this, &ASTTTestActor::OnTextStreamReceived);
        Subsystem->OnSTTCommandReceived.AddDynamic(this, &ASTTTestActor::OnCommandReceived);
        Subsystem->OnKeywordDetected.AddDynamic(this, &ASTTTestActor::OnKeywordDetected);
        Subsystem->OnConnectionStatusChanged.AddDynamic(this, &ASTTTestActor::OnConnectionStatusChanged);
        Subsystem->OnSTTError.AddDynamic(this, &ASTTTestActor::OnErrorOccurred);

        PRINTLOG(TEXT("[STT Test] 이벤트 바인딩 완료"));

        // UI 자동 생성
        if (bAutoCreateUI)
        {
            CreateStatusUI();
        }

        // 레벨 로드 시 마이크 스트리밍 자동 시작
        Subsystem->StartTranscribing();
        PRINTLOG(TEXT("[STT Test] STT 스트리밍 시작"));
    }
    else
    {
        PRINTLOG(TEXT("[STT Test] 경고: STT 서브시스템을 찾을 수 없음"));
    }
}

void ASTTTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // UI 제거
    RemoveStatusUI();

    if (USTTSubsystem* Subsystem = GetSTTSubsystem())
    {
        PRINTLOG(TEXT("[STT Test] 스트리밍 종료"));
        Subsystem->StopTranscribing();

        // 바인딩 해제
        Subsystem->OnSTTTextStream.RemoveAll(this);
        Subsystem->OnSTTCommandReceived.RemoveAll(this);
        Subsystem->OnKeywordDetected.RemoveAll(this);
        Subsystem->OnConnectionStatusChanged.RemoveAll(this);
        Subsystem->OnSTTError.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

// ================================================================================================
// UI 관리
// ================================================================================================

void ASTTTestActor::CreateStatusUI()
{
    if (StatusWidget)
    {
        PRINTLOG(TEXT("[STT Test] UI가 이미 존재함"));
        return;
    }

    if (!StatusWidgetClass)
    {
        PRINTLOG(TEXT("[STT Test] 경고: StatusWidgetClass가 지정되지 않음"));
        return;
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        StatusWidget = CreateWidget<USTTStatusWidget>(PC, StatusWidgetClass);
        if (StatusWidget)
        {
            StatusWidget->AddToViewport(10);  // 높은 ZOrder로 항상 위에 표시
            PRINTLOG(TEXT("[STT Test] UI 위젯 생성 완료"));
        }
    }
}

void ASTTTestActor::RemoveStatusUI()
{
    if (StatusWidget)
    {
        StatusWidget->RemoveFromParent();
        StatusWidget = nullptr;
        PRINTLOG(TEXT("[STT Test] UI 위젯 제거"));
    }
}

// ================================================================================================
// 이벤트 핸들러
// ================================================================================================

void ASTTTestActor::OnTextStreamReceived(const FSTTResponse& Response)
{
    FString Status = Response.bIsFinal ? TEXT("[확정]") : TEXT("[진행중]");
    PRINTLOG(TEXT("[STT Test] 텍스트 수신 %s: %s"), *Status, *Response.TranscribedText);
}

void ASTTTestActor::OnCommandReceived(ESTTCommandType CommandType, const FString& OriginalText)
{
    PRINTLOG(TEXT("[STT Test] 🎯 명령어 수신: %s - \"%s\""),
        *USTTSubsystem::GetCommandDisplayName(CommandType),
        *OriginalText);

    // 명령어별 처리
    switch (CommandType)
    {
    case ESTTCommandType::PlayTrack:
        if (bHandlePlayCommand)
        {
            PRINTLOG(TEXT("[STT Test] ▶️ 재생 명령 실행"));
            OnPlayCommand(OriginalText);
        }
        break;

    case ESTTCommandType::StopTrack:
        if (bHandleStopCommand)
        {
            PRINTLOG(TEXT("[STT Test] ⏹️ 정지 명령 실행"));
            OnStopCommand(OriginalText);
        }
        break;

    case ESTTCommandType::NextTrack:
        if (bHandleNextCommand)
        {
            PRINTLOG(TEXT("[STT Test] ⏭️ 다음 곡 명령 실행"));
            OnNextCommand(OriginalText);
        }
        break;

    default:
        break;
    }
}

void ASTTTestActor::OnKeywordDetected(FGameplayTag Tag, const FString& FullText)
{
    PRINTLOG(TEXT("[STT Test] 키워드 발동 - 태그: [%s], 원문: %s"), *Tag.ToString(), *FullText);
}

void ASTTTestActor::OnConnectionStatusChanged(bool bConnected)
{
    if (bConnected)
    {
        PRINTLOG(TEXT("[STT Test] ✅ 서버 연결됨 - 마이크 스트림 활성화"));
    }
    else
    {
        PRINTLOG(TEXT("[STT Test] ❌ 서버 연결 끊김"));
    }
}

void ASTTTestActor::OnErrorOccurred(const FString& ErrorMsg)
{
    PRINTLOG(TEXT("[STT Test] ⚠️ 오류: %s"), *ErrorMsg);
}

USTTSubsystem* ASTTTestActor::GetSTTSubsystem() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetSubsystem<USTTSubsystem>();
    }
    return nullptr;
}