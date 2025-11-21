
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveMainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserEmailEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserPasswordEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoginValidationTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryLoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveRegisterButton;
	
	FString CommittedUserEmail;
	FString CommittedUserPassword;
	
private:
	void OnUserEmailEditableBoxCommitted();
	void OnUserPasswordEditableBoxCommitted();
	bool ValidLoginForm();


	// 버튼 콜백 함수
	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION()
	void OnRegisterButtonClicked();

	UFUNCTION()
	void OnMoveMainMenuButtonClicked();
};
