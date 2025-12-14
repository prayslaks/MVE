// RadialMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WC_StageLevel_AudRadialMenu.generated.h"

class UMVE_WC_StageLevel_AudRadialSector;
class UImage;

UCLASS()
class MVE_API UMVE_WC_StageLevel_AudRadialMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 현재 선택된 섹터 인덱스를 반환합니다. 선택된 섹터가 없으면 -1을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	int32 GetCurrentSectorIndex() const;

protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_0;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_1;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_2;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_3;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_4;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category = "Radial Menu")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialSector> Sector_5;
	
	UPROPERTY(VisibleAnywhere, Category = "Radial Menu")
	TArray<TObjectPtr<UMVE_WC_StageLevel_AudRadialSector>> Sectors;
	
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	FORCEINLINE int32 GetNumSectors() const { return Sectors.Num(); }

	// 현재 선택된 섹터 인덱스 (0 ~ 5)
	UPROPERTY(BlueprintReadOnly, Category = "Radial Menu")
	int32 CurrentSectorIndex = -1;

	// 메뉴 중앙 데드존 (이 안에 마우스가 있으면 선택 안 함)
	UPROPERTY(EditAnywhere, Category = "Radial Menu")
	float CenterDeadZoneRadius = 50.0f;

	// 0번 인덱스의 시작 각도 보정 (기본적으로 3시 방향이 0도이므로, 12시를 0번으로 하려면 조정 필요)
	UPROPERTY(EditAnywhere, Category = "Radial Menu")
	float AngleOffset = 90.0f;

	// 섹터가 변경되었을 때 호출되어 하이라이트를 변경
	UFUNCTION(Category = "Radial Menu")
	void OnSectorChanged(int32 NewIndex);
	
private:
	UPROPERTY(EditAnywhere, Category = "Radial Menu", meta=(AllowPrivateAccess = true))
	FSlateColor SelectedSlateColor;
	
	UPROPERTY(EditAnywhere, Category = "Radial Menu", meta=(AllowPrivateAccess = true))
	FSlateColor UnselectedSlateColor;
};