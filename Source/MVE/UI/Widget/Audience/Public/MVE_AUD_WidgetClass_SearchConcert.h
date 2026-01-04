
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_SearchConcert.generated.h"

class UEditableTextBox;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchCommitted, const FString&, SearchText);

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_SearchConcert : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRefreshClicked);
	FOnRefreshClicked OnRefreshClicked;
	
	FOnSearchCommitted OnSearchCommitted; // 추가

protected:
	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* ConcertSearchEditableTextBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SearchButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RefreshListButton;

private:
	UFUNCTION()
	void OnRefreshButtonClicked();

    UFUNCTION()
    void OnEditableTextBoxTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION()
	void OnSearchButtonClicked(); // 추가
};
