#include "MVE_LoadingScreenSubsystem.h"

#include "MoviePlayer.h"
#include "MVE.h"
#include "MoviePlayer.h"
#include "Kismet/GameplayStatics.h"

void UMVE_LoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Config 로드
	LoadConfig();

	// 기본값 설정
	if (LoadingMovieName.IsEmpty())
	{
		LoadingMovieName = TEXT("Loading");
	}

	if (MinimumLoadingScreenDisplayTime <= 0.0f)
	{
		MinimumLoadingScreenDisplayTime = 2.0f;
	}

	// 레벨 로드 전 델리게이트 바인딩
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMVE_LoadingScreenSubsystem::OnPreLoadMap);
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &UMVE_LoadingScreenSubsystem::OnPreLoadMapWithContext);

	PRINTLOG(TEXT("✅ LoadingScreenSubsystem 초기화 완료 (MoviePlayer)"));
	PRINTLOG(TEXT("  - LoadingMovieName: '%s'"), *LoadingMovieName);
	PRINTLOG(TEXT("  - MinimumDisplayTime: %.1f초"), MinimumLoadingScreenDisplayTime);
}

void UMVE_LoadingScreenSubsystem::Deinitialize()
{
	// 델리게이트 해제
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);

	Super::Deinitialize();
}

void UMVE_LoadingScreenSubsystem::LoadLevelWithLoadingScreen(const FString& LevelName, bool bUseServerTravel, const FString& Options)
{
	PRINTLOG(TEXT("🎬 레벨 전환 시작: %s (ServerTravel: %s, Options: %s)"),
		*LevelName,
		bUseServerTravel ? TEXT("Yes") : TEXT("No"),
		*Options);

	// 레벨 전환 (PreLoadMap 델리게이트에서 자동으로 로딩 화면 표시)
	if (bUseServerTravel)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FString TravelURL = LevelName + Options;
			World->ServerTravel(TravelURL);
			PRINTLOG(TEXT("ServerTravel: %s"), *TravelURL);
		}
	}
	else
	{
		FString LevelPath = LevelName + Options;
		UGameplayStatics::OpenLevel(this, FName(*LevelPath));
		PRINTLOG(TEXT("OpenLevel: %s"), *LevelPath);
	}
}

void UMVE_LoadingScreenSubsystem::OnPreLoadMap(const FString& MapName)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	PRINTLOG(TEXT("🎥 로딩 영상 재생 시작: %s"), *LoadingMovieName);

	// MoviePlayer 설정
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true; // 로딩 완료 시 자동 종료
	LoadingScreen.bWaitForManualStop = false;
	LoadingScreen.MinimumLoadingScreenDisplayTime = MinimumLoadingScreenDisplayTime;
	LoadingScreen.MoviePaths.Add(LoadingMovieName); // 확장자 제외 ("Loading" → "Loading.mp4" 자동 인식)

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	GetMoviePlayer()->PlayMovie();

	PRINTLOG(TEXT("✅ 로딩 영상 재생 시작됨"));
}

void UMVE_LoadingScreenSubsystem::OnPreLoadMapWithContext(const FWorldContext& WorldContext, const FString& MapName)
{
	OnPreLoadMap(MapName);
}
