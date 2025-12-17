#include "StageLevel/Widget/Public/MVE_WC_StageLevel_AudRadialMenu.h"

#include "MVE_StageLevel_AudCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "StageLevel/Widget/Public/MVE_WC_StageLevel_AudRadialSector.h"

void UMVE_WC_StageLevel_AudRadialMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 배열에 추가
	Sectors.Add(Sector_0);
	Sectors.Add(Sector_1);
	Sectors.Add(Sector_2);
	Sectors.Add(Sector_3);
	Sectors.Add(Sector_4);
	Sectors.Add(Sector_5);
	
	// 열거형 텍스트를 이용하여 초기화
	for (int32 i = 0; i < GetNumSectors(); i++)
	{
		if (i == static_cast<int32>(EAudienceControlMode::WidgetSelection))
		{
			// 비가시
			Sectors[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			// 초기화
			const FText DisplayName = StaticEnum<EAudienceControlMode>()->GetDisplayNameTextByValue(i);	
			Sectors[i]->SectorTitleTextBlock->SetText(DisplayName);	
		}
	}
}

void UMVE_WC_StageLevel_AudRadialMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	const APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		return;
	}
	
	// 1. 마우스 위치 가져오기
	float MouseX, MouseY;
	if (PC->GetMousePosition(MouseX, MouseY) == false)
	{
		return;
	}
	const FVector2D MousePos(MouseX, MouseY);
	
	// 2. 화면(뷰포트) 중앙 좌표 계산
	FVector2D ViewportSize;
	if (GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
	}
	const FVector2D CenterPos = ViewportSize * 0.5f;
	
	// 3. 방향 벡터 계산
	const FVector2D Direction = MousePos - CenterPos;
	
	// 데드존 체크 (마우스가 중앙에 너무 가까우면 계산 안 함)
	if (Direction.Size() < CenterDeadZoneRadius)
	{
		if (CurrentSectorIndex != -1)
		{
			CurrentSectorIndex = -1;
			OnSectorChanged(-1); // 선택 해제 상태
		}
		return;
	}
	
	// 4. 각도 계산 (Atan2 반환값: -180 ~ 180도)
	float Angle = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
	
	// 5. 각도 보정
	
	// A. 오프셋 적용 (예: 12시 방향을 0도로 맞추기 위해)
	Angle += AngleOffset; 
	
	// B. 음수 각도를 0~360 양수로 변환
	if (Angle < 0.0f)
	{
		Angle += 360.0f;
	}
	
	// C. 360도를 넘어가는 경우 모듈러 연산
	Angle = FMath::Fmod(Angle, 360.0f);
	
	// 6. 섹터 인덱스 산출
	const float SectorSize = 360.0f / static_cast<float>(GetNumSectors());
	int32 NewIndex = FMath::FloorToInt(Angle / SectorSize);
	
	// 인덱스 안전장치 (0 ~ NumSectors-1)
	NewIndex = FMath::Clamp(NewIndex, 0, GetNumSectors() - 1);
	
	// 7. 변경사항이 있을 때만 이벤트 호출
	if (NewIndex != CurrentSectorIndex)
	{
		CurrentSectorIndex = NewIndex;
		OnSectorChanged(CurrentSectorIndex);
	}
}

int32 UMVE_WC_StageLevel_AudRadialMenu::GetCurrentSectorIndex() const
{
	return CurrentSectorIndex;
}

void UMVE_WC_StageLevel_AudRadialMenu::OnSectorChanged(const int32 NewIndex)
{
	for (int32 i = 0; i < GetNumSectors(); i++)
	{
		const FSlateColor CurrentSlateColor = i == NewIndex ? SelectedSlateColor : UnselectedSlateColor; 
		Sectors[i]->SectorImage->SetBrushTintColor(CurrentSlateColor);
	}
}