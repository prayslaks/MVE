#include "STTTestActor.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"

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
        Subsystem->OnKeywordDetected.AddDynamic(this, &ASTTTestActor::OnKeywordDetected);
        Subsystem->OnConnectionStatusChanged.AddDynamic(this, &ASTTTestActor::OnConnectionStatusChanged);
        Subsystem->OnSTTError.AddDynamic(this, &ASTTTestActor::OnErrorOccurred);

        PRINTLOG(TEXT("STT 테스트 액터 준비 완료. 자동 스트리밍 시작."));
        
        // 레벨 로드 시 마이크 스트리밍 자동 시작
        Subsystem->StartTranscribing(); 
    }
}

void ASTTTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (USTTSubsystem* Subsystem = GetSTTSubsystem())
    {
        PRINTLOG(TEXT("[TEST] 자동 스트리밍 종료 요청"));
        Subsystem->StopTranscribing();
        
        // 바인딩 해제
        Subsystem->OnSTTTextStream.RemoveAll(this);
        Subsystem->OnKeywordDetected.RemoveAll(this);
        Subsystem->OnConnectionStatusChanged.RemoveAll(this);
        Subsystem->OnSTTError.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

// ------------------------------------ 이벤트 핸들러 ------------------------------------

void ASTTTestActor::OnTextStreamReceived(const FSTTResponse& Response)
{
    FString Status = Response.bIsFinal ? TEXT("[확정]") : TEXT("[진행중]");
    PRINTLOG(TEXT("STT 수신 %s: %s"), *Status, *Response.TranscribedText);
}

void ASTTTestActor::OnKeywordDetected(FGameplayTag Tag, const FString& FullText)
{
    PRINTLOG(TEXT("!!! 키워드 발동 !!! 태그: [%s], 원문: %s"), *Tag.ToString(), *FullText);
}

void ASTTTestActor::OnConnectionStatusChanged(bool bConnected)
{
    if (bConnected)
    {
        PRINTLOG(TEXT("서버 연결됨 - 마이크 스트림이 시작되었습니다."));
    }
    else
    {
        PRINTLOG(TEXT("서버 연결 끊김 / 스트림 종료"));
    }
}

void ASTTTestActor::OnErrorOccurred(const FString& ErrorMsg)
{
    PRINTLOG(TEXT("[오류 발생] %s"), *ErrorMsg);
}

USTTSubsystem* ASTTTestActor::GetSTTSubsystem() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetSubsystem<USTTSubsystem>();
    }
    return nullptr;
}