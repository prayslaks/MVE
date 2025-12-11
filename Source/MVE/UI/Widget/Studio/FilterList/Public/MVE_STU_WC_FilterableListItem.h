#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STU_WC_FilterableListItem.generated.h"


class UButton;
class UTextBlock;

/**
 * 필터링 리스트의 개별 항목을 나타내는 추상 베이스 위젯
 * 호버, 클릭, 드롭다운 트리거 등 공통 인터랙션 처리
 */

UCLASS()
class MVE_API UMVE_STU_WC_FilterableListItem : public UUserWidget
{
	GENERATED_BODY()

public:
	// 항목 데이터 설정 (자식 클래스에서 구현)
	virtual void SetItemData(const FString& Data);

	// 항목 인덱스 설정
	void SetItemIndex(int32 Index);

	// 항목 인덱스 가져오기
	int32 GetItemIndex() const { return ItemIndex; }

	// 항목 데이터 가져오기
	FString GetItemData() const { return ItemData; }

protected:
	virtual void NativeConstruct() override;

	// 호버 이벤트
	UFUNCTION()
	virtual void OnItemHovered();

	UFUNCTION()
	virtual void OnItemUnhovered();

	// 클릭 이벤트 (드롭다운 트리거)
	UFUNCTION()
	virtual void OnItemClicked();

	// Blueprint에서 바인딩할 위젯들
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InteractionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayText;

	// 항목 인덱스 (부모 리스트에서 관리)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemIndex;

	// 항목 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FString ItemData;

public:
	// 수정/삭제 액션을 부모 위젯에 알리기 위한 델리게이트
	DECLARE_DELEGATE_OneParam(FOnItemActionRequested, int32 /* ItemIndex */);
	
	FOnItemActionRequested OnEditRequested;
	FOnItemActionRequested OnDeleteRequested;

	// 드롭다운에서 수정 선택 시 호출
	UFUNCTION(BlueprintCallable, Category = "Item")
	void RequestEdit();

	// 드롭다운에서 삭제 선택 시 호출
	UFUNCTION(BlueprintCallable, Category = "Item")
	void RequestDelete();
};
