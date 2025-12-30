#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_Register.generated.h"

struct FSignUpResponseData;
struct FCheckEmailResponseData;
class UTextBlock;
class UEditableTextBox;
class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_Register : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveLoginButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryRegisterButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserEmailEditableBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SendVerificationCodeButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> VerificationCodeEditableBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryConfirmVerificationCodeButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserPasswordEditableBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> VerificationPasswordEditableBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EmailConfirmFeedbackTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PasswordConfirmFeedbackTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RegisterFeedbackTextBlock;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnSignUpSuccessBIE();

private:	
	UFUNCTION()
	void OnUserEmailEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnVerificationCodeEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnUserPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnVerificationPasswordEditableBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnTryConfirmVerificationCodeButtonClicked();
	
	UFUNCTION()
	void UpdatePasswordConfirmationUI();
	
	UFUNCTION()
	void OnSendVerificationCodeButtonClicked();
	
	UFUNCTION()
	void OnTryRegisterButtonClicked();

	UFUNCTION()
	void OnCheckEmailResult(const bool bSuccess, const FCheckEmailResponseData& ResponseData, const FString& ErrorCode);
	
	UFUNCTION()
	void OnSendVerificationCodeResult(bool bSuccess, const FString& ErrorCode);
	
	UFUNCTION()
	void OnTryConfirmVerifyCodeResult(const bool bSuccess, const FString& ErrorCode);
	
	UFUNCTION()
	void OnSignUpResult(const bool bSuccess, const FSignUpResponseData& ResponseData, const FString& ErrorCode);
	
	UFUNCTION()
	void UpdateRegisterButtonState();
	
	UPROPERTY()
	bool bIsEmailVerified = false;
	
	UPROPERTY()
	FString CommittedUserEmail;
	
	UPROPERTY()
	FString CommittedVerificationCode;
	
	UPROPERTY()
	FString CommittedUserPassword;
	
	UPROPERTY()
	FString CommittedVerificationPassword;
};
