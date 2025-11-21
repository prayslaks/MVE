
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_Register.generated.h"

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
	TObjectPtr<UButton> SwitchLoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryRegisterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserEmailEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SendVerificationCodeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> VerificationCodeEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EmailConfirmStateTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserPasswordEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> VerificationPasswordEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PasswordConfirmStateTextBlock;

public:
	void OnUserPasswordEditableBoxCommitted();
	void OnVerificationPasswordEditableBoxCommitted();
	void ValidPasswordCondition();

private:
	UFUNCTION()
	void OnSendVerificationCodeButtonClicked();

	UFUNCTION()
	void OnRegisterButtonClicked();

	UFUNCTION()
	void OnMoveLoginScreenButtonClicked();
};
