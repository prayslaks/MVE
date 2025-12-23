// STTTestActor.h - 수정된 버전 (UI 연동 + 명령어 처리)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STTSubsystem.h"
#include "STTStatusWidget.h"
#include "STTTestActor.generated.h"

UCLASS()
class MVE_API ASTTTestActor : public AActor
{
    GENERATED_BODY()

public:
    ASTTTestActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ================================================================================================
    // UI 설정
    // ================================================================================================

    // UI 위젯 클래스 (블루프린트에서 지정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|UI")
    TSubclassOf<USTTStatusWidget> StatusWidgetClass;

    // 생성된 UI 인스턴스
    UPROPERTY(BlueprintReadOnly, Category = "STT|UI")
    TObjectPtr<USTTStatusWidget> StatusWidget;

    // 자동으로 UI 생성 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|UI")
    bool bAutoCreateUI = true;

    // ================================================================================================
    // 명령어 처리 설정
    // ================================================================================================

    // 명령어별 처리 활성화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Commands")
    bool bHandlePlayCommand = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Commands")
    bool bHandleStopCommand = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STT|Commands")
    bool bHandleNextCommand = true;

    // ================================================================================================
    // 블루프린트 구현 가능 이벤트
    // ================================================================================================

    // 재생 명령어 수신 시
    UFUNCTION(BlueprintImplementableEvent, Category = "STT|Commands")
    void OnPlayCommand(const FString& OriginalText);

    // 정지 명령어 수신 시
    UFUNCTION(BlueprintImplementableEvent, Category = "STT|Commands")
    void OnStopCommand(const FString& OriginalText);

    // 다음 곡 명령어 수신 시
    UFUNCTION(BlueprintImplementableEvent, Category = "STT|Commands")
    void OnNextCommand(const FString& OriginalText);

    // ================================================================================================
    // 블루프린트 호출 가능 함수
    // ================================================================================================

    UFUNCTION(BlueprintCallable, Category = "STT")
    void CreateStatusUI();

    UFUNCTION(BlueprintCallable, Category = "STT")
    void RemoveStatusUI();

private:
    // 서브시스템 이벤트 수신 함수들
    UFUNCTION()
    void OnTextStreamReceived(const FSTTResponse& Response);

    UFUNCTION()
    void OnCommandReceived(ESTTCommandType CommandType, const FString& OriginalText);

    UFUNCTION()
    void OnKeywordDetected(FGameplayTag Tag, const FString& FullText);

    UFUNCTION()
    void OnConnectionStatusChanged(bool bConnected);

    UFUNCTION()
    void OnErrorOccurred(const FString& ErrorMsg);

    USTTSubsystem* GetSTTSubsystem() const;
};