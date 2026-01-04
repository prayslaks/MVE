
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h" // FConcertInfo에 대한 의존성을 위해 추가
#include "Data/RoomInfo.h" // FConcertInfo를 위해 추가
#include "MVE_AUD_WidgetClass_StationLevel_AudienceStation.generated.h"


class UButton;
class UMVE_AUD_WidgetClass_ConcertList;
class UMVE_AUD_WidgetClass_SearchConcert;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_StationLevel_AudienceStation : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CustomButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ThrowCustomButton;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_AUD_WidgetClass_SearchConcert> SearchConcertWidget;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_AUD_WidgetClass_ConcertList> ConcertListWidget;
	
private:

	UFUNCTION()
	void OnCustomButtonClicked();

	UFUNCTION()
	void OnThrowCustomButtonClicked();

    // SearchConcert의 델리게이트를 처리할 함수 선언
    UFUNCTION()
    void HandleSearchCommitted(const FString& SearchText);

    // GetConcertList API의 콜백 함수 선언
    void OnGetConcertListComplete(bool bSuccess, const struct FGetConcertListData& ResponseData, const FString& ErrorCode);

    // 전체 콘서트 목록을 저장할 변수
    UPROPERTY()
    TArray<FConcertInfo> AllConcertList;
};
