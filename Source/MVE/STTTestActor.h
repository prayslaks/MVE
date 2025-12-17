#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STTSubsystem.h"
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

private:
	// 서브시스템 이벤트 수신 함수들
	UFUNCTION()
	void OnTextStreamReceived(const FSTTResponse& Response);

	UFUNCTION()
	void OnKeywordDetected(FGameplayTag Tag, const FString& FullText);

	UFUNCTION()
	void OnConnectionStatusChanged(bool bConnected);

	UFUNCTION()
	void OnErrorOccurred(const FString& ErrorMsg);

	USTTSubsystem* GetSTTSubsystem() const;
};