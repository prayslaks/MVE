
#pragma once

#include "CoreMinimal.h"
#include "MVE_STU_WC_FilterableList.h"
#include "MVE_STU_WC_BannedWordList.generated.h"

/**
 * 금지어 관리 리스트 위젯
 */

UCLASS()
class MVE_API UMVE_STU_WC_BannedWordList : public UMVE_STU_WC_FilterableList
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	// 금지어 추가
	virtual void AddItem(const FString& ItemData) override;

	// 모든 금지어 가져오기
	UFUNCTION(BlueprintPure, Category = "Banned Words")
	TArray<FString> GetAllBannedWords() const { return BannedWords; }

	// 특정 단어가 금지어인지 확인
	UFUNCTION(BlueprintPure, Category = "Banned Words")
	bool IsWordBanned(const FString& Word) const;

	// 금지어 데이터를 배열로 로드
	UFUNCTION(BlueprintCallable, Category = "Banned Words")
	void LoadBannedWords(const TArray<FString>& Words);

protected:
	virtual UMVE_STU_WC_FilterableListItem* CreateItemWidget() override;
	virtual bool ValidateInput(const FString& Input) const override;

	// 금지어 데이터 저장소
	UPROPERTY(BlueprintReadOnly, Category = "Banned Words")
	TArray<FString> BannedWords;

	// 최소/최대 금지어 길이
	UPROPERTY(EditAnywhere, Category = "Banned Words", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MinWordLength = 1;

	UPROPERTY(EditAnywhere, Category = "Banned Words", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxWordLength = 30;
	
};
