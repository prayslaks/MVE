#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_Login.generated.h"

class UTextBlock;
class UEditableTextBox;
class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_Login : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// 메인 메뉴 이동 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveMainButton;

	// 유저 이름 작성 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserEmailEditableBox;

	// 유저 암호 작성 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserPasswordEditableBox;

	// 로그인 검증 텍스트 블록
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoginValidationTextBlock;

	// 로그인 시도 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryLoginButton;

	// 회원가입 이동 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveRegisterButton;
	
	// 작성된 유저 이름
	UPROPERTY(VisibleAnywhere)
	FString CommittedUserEmail;
	
	// 작성된 유저 비밀번호
	UPROPERTY(VisibleAnywhere)
	FString CommittedUserPassword;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoginSuccessBIE();
	
private:
	// 유저 네임 작성 콜백
	UFUNCTION()
	void OnUserEmailEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	// 유저 암호 작성 콜백
	UFUNCTION()
	void OnUserPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	// 로그인 버튼 콜백
	UFUNCTION()
	void OnLoginButtonClicked();

	// 로그인 API 응답 콜백
	UFUNCTION()
	void OnLoginResultReceived(const bool bSuccess, const FLoginResponseData& ResponseData, const FString& Code);
};