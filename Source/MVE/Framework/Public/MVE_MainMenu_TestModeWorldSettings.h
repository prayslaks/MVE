

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "MVE_MainMenu_TestModeWorldSettings.generated.h"

/**
 * 커스텀 WorldSettings 클래스
 * 개발 중 테스트를 위한 바이패스 설정 제공
 */

UCLASS()
class MVE_API AMVE_MainMenu_TestModeWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	AMVE_MainMenu_TestModeWorldSettings();
	
	// ========================================
	// 개발용 스킵 설정
	// ========================================

	/** 인트로 화면 스킵 여부 */
	UPROPERTY(EditAnywhere, Category = "Dev | Skip Settings", meta = (DisplayName = "Skip Intro"))
	bool bSkipIntro = false;

	/** 로그인 화면 스킵 여부 (자동 로그인) */
	UPROPERTY(EditAnywhere, Category = "Dev | Skip Settings", meta = (DisplayName = "Skip Login"))
	bool bSkipLogin = false;

	/** 자동 로그인 설정 (bSkipLogin이 true일 때만 사용) */
	UPROPERTY(EditAnywhere, Category = "Dev | Skip Settings | Auto Login", meta = (DisplayName = "Dev Account ID", EditCondition = "bSkipLogin"))
	FString DevAccountID = TEXT("test@move.com");

	/** 자동 로그인 비밀번호 (bSkipLogin이 true일 때만 사용) */
	UPROPERTY(EditAnywhere, Category = "Dev | Skip Settings | Auto Login", meta = (DisplayName = "Dev Password", EditCondition = "bSkipLogin", PasswordField = true))
	FString DevPassword = TEXT("test1234");
	

	// ========================================
	// 헬퍼 함수들
	// ========================================

	/** WorldSettings에서 스킵 설정 가져오기 (안전한 접근) */
	static bool ShouldSkipIntro(const UWorld* World);
	static bool ShouldSkipLogin(const UWorld* World);
	static void GetDevLoginCredentials(const UWorld* World, FString& OutID, FString& OutPassword);

private:
	/** 커맨드라인에서 로그인 정보 파싱 */
	static bool ParseDevLoginFromCommandLine(FString& OutID, FString& OutPassword);
};
