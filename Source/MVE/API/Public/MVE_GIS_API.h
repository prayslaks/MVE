#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVE_GIS_API.generated.h"

UCLASS()
class MVE_API UMVE_GIS_API : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief 주어진 MVE 리스폰스 코드에 대해 번역된 텍스트를 반환.
	 * @param Code MVE API 리스폰스에서 파싱해서 얻는 코드.
	 * @return LOCTEXT에 의해 로컬라이징된 FText. 코드와 매핑되는 값이 없다면 기본값 반환.
	 */
	FText GetTranslatedTextFromResponseCode(const FString& Code) const;
	
	// 외부에서 게임 인스턴스 API 서브시스템 획득
	static UMVE_GIS_API* Get(const UObject* WorldContextObject);
	
	// 현재 참가 중인 콘서트 룸 ID
	UPROPERTY(BlueprintReadWrite)
	FString CurrentRoomId;

protected:
	// MVE API 게임인스턴스 서브시스템 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	// 입력값 (리스폰스 코드)에 대해 출력값 (로컬라이징 텍스트)를 반환하는 맵
	UPROPERTY()
	TMap<FString, FText> ResponseCodeToTextMap;

	// ApiSpecs에 명시된 (리스폰스 코드)를 (로컬라이징 텍스트)로 매핑 
	void MapResponseCodeToText();
};