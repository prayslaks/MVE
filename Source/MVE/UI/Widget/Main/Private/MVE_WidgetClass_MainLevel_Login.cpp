#include "UI/Widget/Main/Public/MVE_WidgetClass_MainLevel_Login.h"
#include "MVE.h"
#include "MVE_GIS_API.h"
#include "API/Public/MVE_API_Helper.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UMVE_WidgetClass_MainLevel_Login::NativeConstruct()
{
	Super::NativeConstruct();

	// 유저 닉네임 바인딩
	if (UserEmailEditableBox)
	{
		UserEmailEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnUserEmailEditableBoxCommitted);
	}

	// 유저 비밀번호 바인딩
	if (UserPasswordEditableBox)
	{
		UserPasswordEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnUserPasswordEditableBoxCommitted);
	}

	// 메인 메뉴 이동 바인딩
	if (MoveMainButton)
	{
		MoveMainButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnMoveMainMenuButtonClicked);
	}

	// 로그인 시도 바인딩
	if (TryLoginButton)
	{
		TryLoginButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnLoginButtonClicked);
	}

	// 회원가입 이동 바인딩
	if (MoveRegisterButton)
	{
		MoveRegisterButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnRegisterButtonClicked);
	}

	// 로그인 검증 텍스트 블록
	if (LoginValidationTextBlock)
	{
		LoginValidationTextBlock->SetText(FText::GetEmpty());
		LoginValidationTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnUserEmailEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedUserEmail = Text.ToString();
		PRINTLOG(TEXT("로그인 : 유저 닉네임이 제출됐습니다! {%s}"), *CommittedUserEmail);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnUserPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedUserPassword = Text.ToString();
		PRINTLOG(TEXT("로그인 : 유저 비밀번호가 제출됐습니다! {%s}"), *CommittedUserPassword);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnLoginButtonClicked()
{
	// API 요청 직전 최종 변수 업데이트
	CommittedUserEmail = UserEmailEditableBox->GetText().ToString();
	CommittedUserPassword = UserPasswordEditableBox->GetText().ToString();
	
	// 로그인 API 요청
	FMVEOnLoginComplete OnResult;
	OnResult.BindUObject(this, &UMVE_WidgetClass_MainLevel_Login::OnLoginResultReceived);
	UMVE_API_Helper::Login(CommittedUserEmail, CommittedUserPassword, OnResult);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Login::OnLoginResultReceived(const bool bSuccess, const FLoginResponseData& ResponseData, const FString& ErrorCode)
{
	if (bSuccess)
	{
		// JWT 토큰 설정
		UMVE_API_Helper::SetAuthToken(ResponseData.token);
		
		if (LoginValidationTextBlock)
		{
			LoginValidationTextBlock->SetVisibility(ESlateVisibility::Hidden);
		}

		// 요청이 성공했으므로 이동
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UIManager->ShowScreen(EUIScreen::ModeSelect);
		}
	}
	else
	{
		FText TranslatedErrorMessage;
		if (const UMVE_GIS_API* Subsystem = UMVE_GIS_API::Get(this))
		{
			TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
		}
		
		if (LoginValidationTextBlock)
		{
			LoginValidationTextBlock->SetText(TranslatedErrorMessage);
			LoginValidationTextBlock->SetColorAndOpacity(FLinearColor::Red);
			LoginValidationTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Login::OnRegisterButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Register);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Login::OnMoveMainMenuButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
}