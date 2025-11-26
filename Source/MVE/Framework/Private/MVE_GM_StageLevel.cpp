
#include "../Public/MVE_GM_StageLevel.h"

#include "MVE.h"
#include "MVE_AUD_PC_StageLevel.h"
#include "MVE_PC_StageLevel.h"
#include "MVE_STU_PC_StageLevel.h"

AMVE_GM_StageLevel::AMVE_GM_StageLevel()
{
	// OpenLevel로 시작하므로 Seamless Travel 불필요
	bUseSeamlessTravel = false;

	// 기본은 Audience PC (클라이언트용)
	PlayerControllerClass = AMVE_PC_StageLevel::StaticClass();
}

void AMVE_GM_StageLevel::BeginPlay()
{
	Super::BeginPlay();
}
