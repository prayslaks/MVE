
#include "../Public/MVE_STU_WC_BannedWordList.h"

#include "UI/Widget/Studio/FilterList/Public/MVE_STU_WC_BannedWordItem.h"
#include "UI/Widget/Studio/FilterList/Public/MVE_STU_WC_FilterableListItem.h"

void UMVE_STU_WC_BannedWordList::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMVE_STU_WC_BannedWordList::AddItem(const FString& ItemData)
{
	// 부모 클래스의 AddItem 호출 (검증 및 위젯 생성)
	Super::AddItem(ItemData);

	// 금지어 데이터에 추가
	if (!BannedWords.Contains(ItemData))
	{
		BannedWords.Add(ItemData);
		UE_LOG(LogTemp, Log, TEXT("Banned word added to data: %s"), *ItemData);
	}
}

bool UMVE_STU_WC_BannedWordList::IsWordBanned(const FString& Word) const
{
	return BannedWords.Contains(Word);
}

void UMVE_STU_WC_BannedWordList::LoadBannedWords(const TArray<FString>& Words)
{
	// 기존 데이터 초기화
	ClearAllItems();
	BannedWords.Empty();

	// 새 데이터 로드
	for (const FString& Word : Words)
	{
		AddItem(Word);
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded %d banned words"), Words.Num());
}

UMVE_STU_WC_FilterableListItem* UMVE_STU_WC_BannedWordList::CreateItemWidget()
{
	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemWidgetClass is not set!"));
		return nullptr;
	}

	// 금지어 항목 위젯 생성
	UMVE_STU_WC_FilterableListItem* NewWidget = CreateWidget<UMVE_STU_WC_FilterableListItem>(GetOwningPlayer(), ItemWidgetClass);
	return NewWidget;
}

bool UMVE_STU_WC_BannedWordList::ValidateInput(const FString& Input) const
{
	// 부모 클래스 검증 먼저 수행
	if (!Super::ValidateInput(Input))
	{
		return false;
	}

	FString TrimmedInput = Input.TrimStartAndEnd();

	// 길이 검증
	if (TrimmedInput.Len() < MinWordLength || TrimmedInput.Len() > MaxWordLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid word length: %d (Min: %d, Max: %d)"), 
			TrimmedInput.Len(), MinWordLength, MaxWordLength);
		return false;
	}

	// 추가 검증 가능
	// 예: 특수문자만으로 구성된 경우 거부
	// 예: 숫자만으로 구성된 경우 거부 등

	return true;
}

void UMVE_STU_WC_BannedWordList::HandleUpdateItem(int32 Index, const FString& NewData)
{
	Super::HandleUpdateItem(Index, NewData);

	// 데이터 배열도 동기화
	if (BannedWords.IsValidIndex(Index))
	{
		BannedWords[Index] = NewData;
		UE_LOG(LogTemp, Log, TEXT("[BannedWordList] Updated data array at index %d: %s"), Index, *NewData);
	}
}
