
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MVE_STU_PC_StageLevel.generated.h"


class UUserWidget;

UCLASS()
class MVE_API AMVE_STU_PC_StageLevel : public APlayerController
{
	GENERATED_BODY()

public:
	AMVE_STU_PC_StageLevel();

protected:
	virtual void BeginPlay() override;

	// 호스트 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StudioUIClass;

private:
	// 호스트 UI 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> StudioUIWidget;

	/**
	 * 호스트 UI 표시
	 */
	void ShowStudioUI();
};
