
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STU_WC_FilterableList.generated.h"

class UScrollBox;
class UEditableTextBox;
class UMVE_STU_WC_FilterableListItem;

/**
 * 필터링 가능한 항목 리스트를 관리하는 추상 베이스 위젯
 * 금지어 리스트, 차단 유저 리스트 등의 공통 기능 제공
 */

UCLASS()
class MVE_API UMVE_STU_WC_FilterableList : public UUserWidget
{
	GENERATED_BODY()

public:
	// 항목 추가 (자식 클래스에서 구현)
	UFUNCTION(BlueprintCallable, Category = "Filterable List")
	virtual void AddItem(const FString& ItemData);

	// 항목 삭제
	UFUNCTION(BlueprintCallable, Category = "Filterable List")
	virtual void RemoveItem(int32 Index);

	// 항목 수정
	UFUNCTION(BlueprintCallable, Category = "Filterable List")
	virtual void UpdateItem(int32 Index, const FString& NewData);

	// 모든 항목 삭제
	UFUNCTION(BlueprintCallable, Category = "Filterable List")
	virtual void ClearAllItems();

	// 항목 개수 반환
	UFUNCTION(BlueprintPure, Category = "Filterable List")
	int32 GetItemCount() const { return ItemWidgets.Num(); }

protected:
	virtual void NativeConstruct() override;

	// 항목 위젯 생성 (팩토리 메서드 - 자식 클래스에서 구현)
	virtual UMVE_STU_WC_FilterableListItem* CreateItemWidget();

	// 입력 검증 (자식 클래스에서 오버라이드 가능)
	virtual bool ValidateInput(const FString& Input) const;

	// 입력 필드 이벤트
	UFUNCTION()
	void OnInputCommitted(const FText& Text, ETextCommit::Type CommitType);

	// 항목 액션 핸들러
	void HandleItemEdit(int32 ItemIndex);
	void HandleItemDelete(int32 ItemIndex);

	// Blueprint에서 바인딩할 위젯들
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ItemScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> InputField;

	// 항목 위젯 클래스 (자식 클래스에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Filterable List")
	TSubclassOf<UMVE_STU_WC_FilterableListItem> ItemWidgetClass;

	// 생성된 항목 위젯들
	UPROPERTY()
	TArray<TObjectPtr<UMVE_STU_WC_FilterableListItem>> ItemWidgets;

	// 최대 항목 개수 (옵션)
	UPROPERTY(EditAnywhere, Category = "Filterable List", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxItemCount = 100;

	// 중복 허용 여부
	UPROPERTY(EditAnywhere, Category = "Filterable List")
	bool bAllowDuplicates = false;
};
