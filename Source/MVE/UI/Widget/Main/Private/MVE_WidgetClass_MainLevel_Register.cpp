#include "UI/Widget/Main/Public/MVE_WidgetClass_MainLevel_Register.h"

#include "MVE.h"
#include "API/Public/MVE_API_Helper.h"
#include "API/Public/MVE_GIS_API.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UMVE_WidgetClass_MainLevel_Register::NativeConstruct()
{
	Super::NativeConstruct();

	// 로그인 화면으로 돌아가는 버튼 바인딩
	if (MoveLoginButton)
	{
		MoveLoginButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnMoveLoginButtonClicked);
	}
	
	// 회원가입 시도 버튼 바인딩
	if (TryRegisterButton)
	{
		TryRegisterButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnTryRegisterButtonClicked);
	}
	
	// 인증번호 전송 버튼 바인딩
	if (SendVerificationCodeButton)
	{
		SendVerificationCodeButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeButtonClicked);
	}
	
	// 유저 이메일 입력창 바인딩
	if (UserEmailEditableBox)
	{
		UserEmailEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnUserEmailEditableBoxCommitted);
	}
	
	// 인증 코드 입력창 바인딩
	if (VerificationCodeEditableBox)
	{
		VerificationCodeEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnVerificationCodeEditableBoxCommitted);
	}
	
	// 인증 번호 확인 바인딩
	if (TryConfirmVerificationCodeButton)
	{
		TryConfirmVerificationCodeButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnTryConfirmVerificationCodeButtonClicked);
	}
	
	// 유저 암호 입력창 바인딩
	if (UserPasswordEditableBox)
	{
		UserPasswordEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnUserPasswordEditableBoxCommitted);
	}
	
	// 암호 재확인 입력창 바인딩
	if (VerificationPasswordEditableBox)
	{
		VerificationPasswordEditableBox->OnTextCommitted.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnVerificationPasswordEditableBoxCommitted);
	}
	
	// 회원가입 피드백 위젯 초기 상태 설정
	if(RegisterFeedbackTextBlock)
	{
		RegisterFeedbackTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 이메일 확인 피드백 위젯 초기 상태 설정
	if(EmailConfirmFeedbackTextBlock)
	{
		EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 암호 확인 피드백 위젯 초기 상태 설정
	if(PasswordConfirmFeedbackTextBlock)
	{
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	UpdateRegisterButtonState();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Register::OnMoveLoginButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Login);
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnUserEmailEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedUserEmail = Text.ToString();
		bIsEmailVerified = false; // 이메일이 변경되었으므로 인증 상태 초기화
		UpdateRegisterButtonState();
	
		// 이메일 중복 확인 API 호출
		FOnCheckEmailComplete OnResult;
		OnResult.BindUObject(this, &UMVE_WidgetClass_MainLevel_Register::OnCheckEmailResult);
		UMVE_API_Helper::CheckEmail(CommittedUserEmail, OnResult);
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnVerificationCodeEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedVerificationCode = Text.ToString();
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnUserPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedUserPassword = Text.ToString();
		UpdatePasswordConfirmationUI();
		UpdateRegisterButtonState();
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnVerificationPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter || CommitMethod == ETextCommit::Type::OnUserMovedFocus)
	{
		CommittedVerificationPassword = Text.ToString();
		UpdatePasswordConfirmationUI();
		UpdateRegisterButtonState();
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnTryConfirmVerificationCodeButtonClicked()
{
	CommittedVerificationCode = VerificationCodeEditableBox->GetText().ToString();
	
	// 인증 번호 입력 여부 확인
	if (CommittedVerificationCode.IsEmpty())
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("이메일을 확인후 인증 번호를 입력해주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	
	// 인증 번호 유효 확인
	if (CommittedVerificationCode.IsNumeric() == false)
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("숫자로 이루어진 인증 번호를 입력해주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	
	// 인증 번호 유효 확인
	if (CommittedVerificationCode.Len() != 6)
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("6자리 인증 번호를 입력해주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	
	// 인증번호 발송 API 호출
	FOnGenericApiComplete OnResult;
	OnResult.BindUObject(this, &UMVE_WidgetClass_MainLevel_Register::OnTryConfirmVerifyCodeResult);
	UMVE_API_Helper::TryConfirmVerifyCode(CommittedUserEmail, CommittedVerificationCode, OnResult);
}

void UMVE_WidgetClass_MainLevel_Register::UpdatePasswordConfirmationUI()
{
	if (!PasswordConfirmFeedbackTextBlock) return;

	// 최종 입력 값 가져오기
	CommittedUserPassword = UserPasswordEditableBox->GetText().ToString();
	CommittedVerificationPassword = VerificationPasswordEditableBox->GetText().ToString();

	if (CommittedUserPassword.IsEmpty() && CommittedVerificationPassword.IsEmpty())
	{
		PasswordConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("")));
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	if (CommittedUserPassword.IsEmpty())
	{
		PasswordConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("비밀번호가 입력되지 않았습니다!")));
		PasswordConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	if (CommittedVerificationPassword.IsEmpty())
	{
		PasswordConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("2차 확인이 완료되지 않았습니다!")));
		PasswordConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		return;
	}

	if (CommittedUserPassword == CommittedVerificationPassword)
	{
		PasswordConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("비밀번호가 일치합니다.")));
		PasswordConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Green);
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		PasswordConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("비밀번호가 일치하지 않습니다!")));
		PasswordConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
		PasswordConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeButtonClicked()
{
	CommittedUserEmail = UserEmailEditableBox->GetText().ToString();
	
	// 이메일 입력 여부 확인
	if (CommittedUserEmail.IsEmpty())
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("이메일을 먼저 입력해주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	
	// 인증번호 발송 API 호출
	FOnGenericApiComplete OnResult;
	OnResult.BindUObject(this, &UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeResult);
	UMVE_API_Helper::SendVerificationCode(CommittedUserEmail, OnResult);
}

void UMVE_WidgetClass_MainLevel_Register::OnTryRegisterButtonClicked()
{
	// 최종 값 업데이트
	CommittedUserEmail = UserEmailEditableBox->GetText().ToString();
	CommittedUserPassword = UserPasswordEditableBox->GetText().ToString();
	CommittedVerificationPassword = VerificationPasswordEditableBox->GetText().ToString();
	
	// 유효성 검사
	if(CommittedUserPassword != CommittedVerificationPassword)
	{
		if(RegisterFeedbackTextBlock)
		{
			RegisterFeedbackTextBlock->SetText(FText::FromString(TEXT("비밀번호와 비밀번호 확인이 일치하지 않습니다.")));
			RegisterFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	if(!bIsEmailVerified)
	{
		if(RegisterFeedbackTextBlock)
		{
			RegisterFeedbackTextBlock->SetText(FText::FromString(TEXT("이메일 인증을 먼저 완료해야 합니다.")));
			RegisterFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	
	// 회원가입 API 호출
	FOnSignUpComplete OnResult;
	OnResult.BindUObject(this, &UMVE_WidgetClass_MainLevel_Register::OnSignUpResult);
	UMVE_API_Helper::SignUp(CommittedUserEmail, CommittedUserPassword, CommittedVerificationCode, OnResult);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Register::OnCheckEmailResult(const bool bSuccess, const FCheckEmailResponseData& ResponseData, const FString& ErrorCode)
{
	if (bSuccess)
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("사용 가능한 이메일입니다. '인증번호 발송'을 눌러주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Green);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		FText TranslatedErrorMessage;
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UMVE_GIS_API* Subsystem = GameInstance->GetSubsystem<UMVE_GIS_API>())
			{
				TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}
		
		if (EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(TranslatedErrorMessage);
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeResult(const bool bSuccess, const FString& ErrorCode)
{
	if (bSuccess)
	{
		if(EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("인증 메일이 전송되었습니다! 메일을 확인하여 인증번호를 입력해주세요.")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Green);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		FText TranslatedErrorMessage;
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UMVE_GIS_API* Subsystem = GameInstance->GetSubsystem<UMVE_GIS_API>())
			{
				TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}

		if (EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(TranslatedErrorMessage);
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnTryConfirmVerifyCodeResult(const bool bSuccess, const FString& ErrorCode)
{
	if (bSuccess)
	{
		bIsEmailVerified = true;
		if (EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(FText::FromString(TEXT("이메일 인증에 성공했습니다!")));
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Green);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		bIsEmailVerified = false;
		FText TranslatedErrorMessage;
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UMVE_GIS_API* Subsystem = GameInstance->GetSubsystem<UMVE_GIS_API>())
			{
				TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}

		if (EmailConfirmFeedbackTextBlock)
		{
			EmailConfirmFeedbackTextBlock->SetText(TranslatedErrorMessage);
			EmailConfirmFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			EmailConfirmFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
	UpdateRegisterButtonState();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_Register::OnSignUpResult(const bool bSuccess, const FSignUpResponseData& ResponseData, const FString& ErrorCode)
{
	if (bSuccess)
	{
		if(RegisterFeedbackTextBlock)
		{
			RegisterFeedbackTextBlock->SetText(FText::FromString(TEXT("회원가입에 성공했습니다! 잠시 후 로그인 화면으로 이동합니다.")));
			RegisterFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Green);
			RegisterFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}

		// 2초 후 로그인 화면으로 전환
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
			{
				UIManager->ShowScreen(EUIScreen::Login);
			}
		}, 2.0f, false);
	}
	else
	{
		FText TranslatedErrorMessage;
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UMVE_GIS_API* Subsystem = GameInstance->GetSubsystem<UMVE_GIS_API>())
			{
				TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}

		if (RegisterFeedbackTextBlock)
		{
			RegisterFeedbackTextBlock->SetText(TranslatedErrorMessage);
			RegisterFeedbackTextBlock->SetColorAndOpacity(FLinearColor::Red);
			RegisterFeedbackTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UMVE_WidgetClass_MainLevel_Register::UpdateRegisterButtonState()
{
	// 최종 값 가져오기
	CommittedUserEmail = UserEmailEditableBox->GetText().ToString();
	CommittedUserPassword = UserPasswordEditableBox->GetText().ToString();
	CommittedVerificationPassword = VerificationPasswordEditableBox->GetText().ToString();
	
	// 조건 확인
	const bool bPasswordsMatch = (!CommittedUserPassword.IsEmpty()) && (CommittedUserPassword == CommittedVerificationPassword);
	const bool bCanAttemptRegister = bIsEmailVerified && bPasswordsMatch && !CommittedUserEmail.IsEmpty();
	
	// 결과 반영
	if (TryRegisterButton)
	{
		TryRegisterButton->SetIsEnabled(bCanAttemptRegister);
	}
}