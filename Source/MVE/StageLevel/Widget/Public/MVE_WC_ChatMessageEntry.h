
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WC_ChatMessageEntry.generated.h"

class UMVE_ChatMessageEntry;
class UTextBlock;
class UHorizontalBox;

/**
 * ListView Entry 위젯
 * - IUserObjectListEntry 인터페이스 구현
 * - 메시지 데이터를 UI에 표시
 */

UCLASS()
class MVE_API UMVE_WC_ChatMessageEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	//~ UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	//~ IUserObjectListEntry Interface
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	/**
	 * UI 위젯 생성 (C++에서 동적 생성)
	 */
	void CreateWidgets();

	/**
	 * 메시지 데이터로 UI 업데이트
	 * @param Entry 메시지 엔트리 오브젝트
	 */
	void UpdateUI(UMVE_ChatMessageEntry* Entry);

protected:
	/** 현재 표시 중인 메시지 엔트리 */
	UPROPERTY()
	TObjectPtr<UMVE_ChatMessageEntry> CurrentEntry;

	/** 루트 컨테이너 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> RootHorizontalBox;

	/** 발신자 이름 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> SenderNameText;

	/** 메시지 내용 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MessageContentText;

	/** 타임스탬프 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TimestampText;

	/** 위젯 생성 완료 여부 */
	bool bWidgetsCreated;

};
