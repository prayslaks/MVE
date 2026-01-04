#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVE_LoadingScreenSubsystem.generated.h"

/**
 * 로딩 화면 서브시스템 (MoviePlayer 사용)
 * 모든 레벨 전환 시 로딩 영상을 자동으로 재생
 *
 * 사용 예시:
 * auto LoadingScreen = GetGameInstance()->GetSubsystem<UMVE_LoadingScreenSubsystem>();
 * LoadingScreen->LoadLevelWithLoadingScreen("StageLevel");
 */
UCLASS(Config=Game)
class MVE_API UMVE_LoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 로딩 화면을 표시하고 레벨 전환
	 * @param LevelName 전환할 레벨 이름 (예: "StageLevel", "/Game/Maps/StageLevel")
	 * @param bUseServerTravel true면 ServerTravel 사용, false면 OpenLevel 사용
	 * @param Options 추가 옵션 (예: "?listen")
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void LoadLevelWithLoadingScreen(const FString& LevelName, bool bUseServerTravel = false, const FString& Options = "");

protected:
	/** 로딩 영상 파일 이름 (확장자 제외, Content/Movies/ 기준) */
	UPROPERTY(Config)
	FString LoadingMovieName;

	/** 최소 로딩 화면 표시 시간 (초) */
	UPROPERTY(Config)
	float MinimumLoadingScreenDisplayTime;

private:
	/** 레벨 로드 전 호출 */
	void OnPreLoadMap(const FString& MapName);

	/** 레벨 로드 전 호출 (WorldContext 포함) */
	void OnPreLoadMapWithContext(const FWorldContext& WorldContext, const FString& MapName);
};
