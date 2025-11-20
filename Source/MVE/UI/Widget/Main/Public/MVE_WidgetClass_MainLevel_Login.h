
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GoMainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserEmailEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserPasswordEditableBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoginValidationTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TryLoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GoRegisterButton;
	
	FString CommittedUserEmail;
	FString CommittedUserPassword;
	
public:
	void OnUserEmailEditableBoxCommitted();
	void OnUserPasswordEditableBoxCommitted();
	bool ValidLoginForm();
};
