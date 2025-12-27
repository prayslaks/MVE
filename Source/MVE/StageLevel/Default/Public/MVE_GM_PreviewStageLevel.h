#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_GM_PreviewStageLevel.generated.h"

/**
 * PreviewStageLevel GameMode
 * - FinalCheckSettings UI를 표시
 * - EffectSequencePreview로 무대 프리뷰 제공
 * - StartConcertButton 클릭 시 StageLevel로 전환
 */
UCLASS()
class MVE_API AMVE_GM_PreviewStageLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMVE_GM_PreviewStageLevel();

protected:
	virtual void BeginPlay() override;
};
