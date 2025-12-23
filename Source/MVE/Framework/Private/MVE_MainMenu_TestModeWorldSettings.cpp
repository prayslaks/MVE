

#include "../Public/MVE_MainMenu_TestModeWorldSettings.h"

AMVE_MainMenu_TestModeWorldSettings::AMVE_MainMenu_TestModeWorldSettings()
{
}

bool AMVE_MainMenu_TestModeWorldSettings::ShouldSkipIntro(const UWorld* World)
{
	if (World)
	{
		if (AMVE_MainMenu_TestModeWorldSettings* MVESettings = Cast<AMVE_MainMenu_TestModeWorldSettings>(World->GetWorldSettings()))
		{
			return MVESettings->bSkipIntro;
		}
	}
	return false;
}

bool AMVE_MainMenu_TestModeWorldSettings::ShouldSkipLogin(const UWorld* World)
{
	if (World)
	{
		if (AMVE_MainMenu_TestModeWorldSettings* MVESettings = Cast<AMVE_MainMenu_TestModeWorldSettings>(World->GetWorldSettings()))
		{
			return MVESettings->bSkipLogin;
		}
	}
	return false;
}

void AMVE_MainMenu_TestModeWorldSettings::GetDevLoginCredentials(const UWorld* World, FString& OutID,
	FString& OutPassword)
{
	OutID.Empty();
	OutPassword.Empty();
	
	if (World)
	{
		if (AMVE_MainMenu_TestModeWorldSettings* MVESettings = Cast<AMVE_MainMenu_TestModeWorldSettings>(World->GetWorldSettings()))
		{
			OutID = MVESettings->DevAccountID;
			OutPassword = MVESettings->DevPassword;
		}
	}
}

bool AMVE_MainMenu_TestModeWorldSettings::ParseDevLoginFromCommandLine(FString& OutID, FString& OutPassword)
{
	// 커맨드라인 파라미터 읽기
	// 예: MyProject.uproject -DevLoginID="test@move.com" -DevLoginPW="test1234"
	
	const TCHAR* CmdLine = FCommandLine::Get();
	
	bool bFoundID = FParse::Value(CmdLine, TEXT("DevLoginID="), OutID);
	bool bFoundPW = FParse::Value(CmdLine, TEXT("DevLoginPW="), OutPassword);
	
	// 둘 다 있어야 유효
	if (bFoundID && bFoundPW)
	{
		return true;
	}
	
	// 하나라도 없으면 초기화하고 false 반환
	OutID.Empty();
	OutPassword.Empty();
	return false;
}
