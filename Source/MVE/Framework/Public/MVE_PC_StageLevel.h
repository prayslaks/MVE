
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MVE_PC_StageLevel.generated.h"

UCLASS()
class MVE_API AMVE_PC_StageLevel : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay();

private:
	/**
	 * 호스트 UI 표시
	 */
	void ShowStudioUI();

	/**
	 * 클라이언트 UI 표시
	 */
	void ShowAudienceUI();

	/**
	 * 이 PC가 리슨 서버 호스트인지 확인
	 */
	bool IsListenServerHost() const;
};
