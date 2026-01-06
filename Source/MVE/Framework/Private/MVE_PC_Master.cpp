
#include "../Public/MVE_PC_Master.h"

#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_MainMenu_TestModeWorldSettings.h"
#include "MVE_WidgetClass_MainLevel.h"
#include "NetworkMessage.h"
#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

void AMVE_PC_Master::BeginPlay()
{
	Super::BeginPlay();

	// Input Mode, Mouse Cursor 보이기 설정
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI M;
	M.SetHideCursorDuringCapture(false);
	SetInputMode(M);

	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (!UIManager)
	{
		return;
	}

	// 커맨드라인에서 개발용 로그인 정보 파싱 (배치 파일 실행 시)
	FString CmdLineDevEmail, CmdLineDevPassword;
	bool bHasCmdLineCredentials = false;

	if (FParse::Value(FCommandLine::Get(), TEXT("DevLoginID="), CmdLineDevEmail) &&
		FParse::Value(FCommandLine::Get(), TEXT("DevLoginPW="), CmdLineDevPassword))
	{
		bHasCmdLineCredentials = true;
		PRINTLOG(TEXT("[Dev Mode] 커맨드라인에서 로그인 정보 감지: %s"), *CmdLineDevEmail);
	}

	// 개발 모드 체크: Intro 스킵
	const bool bSkipIntro = AMVE_MainMenu_TestModeWorldSettings::ShouldSkipIntro(GetWorld());
	const bool bSkipLogin = AMVE_MainMenu_TestModeWorldSettings::ShouldSkipLogin(GetWorld());

	// 커맨드라인에 로그인 정보가 있거나 WorldSettings에서 스킵 설정이 되어있는 경우
	if (bHasCmdLineCredentials || bSkipLogin)
	{
		PRINTLOG(TEXT("[Dev Mode] Auto-login 시도중..."));

		// 개발용 계정 정보 가져오기
		FString DevEmail, DevPassword;

		// 커맨드라인 인자가 우선순위
		if (bHasCmdLineCredentials)
		{
			DevEmail = CmdLineDevEmail;
			DevPassword = CmdLineDevPassword;
			PRINTLOG(TEXT("[Dev Mode] 커맨드라인 인자 사용"));
		}
		else
		{
			AMVE_MainMenu_TestModeWorldSettings::GetDevLoginCredentials(GetWorld(), DevEmail, DevPassword);
			PRINTLOG(TEXT("[Dev Mode] WorldSettings 값 사용"));
		}

		if (DevEmail.IsEmpty() || DevPassword.IsEmpty())
		{
			PRINTLOG(TEXT("[Dev Mode] 개발 계정 정보가 비어있습니다. Login 화면으로 이동합니다."));
			UIManager->ShowScreen(EUIScreen::Login);
			return;
		}

		// 자동 로그인 시도
		FMVEOnLoginComplete OnResult;
		OnResult.BindUObject(this, &AMVE_PC_Master::OnAutoLoginComplete);
		UMVE_API_Helper::Login(DevEmail, DevPassword, OnResult);
	}
	// Intro만 스킵하는 경우 - Login 화면으로
	else if (bSkipIntro)
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
	// 정상 플로우 - Intro부터 시작
	else
	{
		UIManager->ShowScreen(EUIScreen::Intro);
	}
}

void AMVE_PC_Master::OnAutoLoginComplete(bool bSuccess, const FLoginResponseData& ResponseData, const FString& ErrorCode)
{
	if (bSuccess)
	{
		PRINTLOG(TEXT("[Dev Mode] Auto-login 성공! ModeSelect로 이동합니다."));

		// JWT 토큰 설정
		UMVE_API_Helper::SetAuthToken(ResponseData.Token);

		// ModeSelect 화면으로 이동
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UIManager->ShowScreen(EUIScreen::ModeSelect);
		}
	}
	else
	{
		PRINTLOG(TEXT("[Dev Mode] Auto-login 실패! (ErrorCode: %s) Login 화면으로 폴백합니다."), *ErrorCode);

		// 실패 시 Login 화면으로 폴백
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UIManager->ShowScreen(EUIScreen::Login);
		}
	}
}
