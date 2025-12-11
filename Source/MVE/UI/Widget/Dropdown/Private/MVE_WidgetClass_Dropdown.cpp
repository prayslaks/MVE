
#include "../Public/MVE_WidgetClass_Dropdown.h"
#include "MVE.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UMVE_WidgetClass_Dropdown::SetDropdownPosition(const FVector2D& InPosition)
{
	// 위젯 루트가 Canvas Panel이라고 가정
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());

	PRINTLOG(TEXT("SetDropdownPosition - RootCanvas: %s, Position: %s"),
		RootCanvas ? TEXT("Valid") : TEXT("Null"), *InPosition.ToString());

	if (RootCanvas && RootCanvas->GetChildrenCount() > 0)
	{
		PRINTLOG(TEXT("Canvas Children Count: %d"), RootCanvas->GetChildrenCount());

		// 첫 번째 자식이 Size Box라고 가정
		UWidget* SizeBoxWidget = RootCanvas->GetChildAt(0);
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SizeBoxWidget->Slot))
		{
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetZOrder(1); // Canvas Panel 내에서 최상위

			// 화면 크기 가져오기
			FVector2D ViewportSize;
			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->GetViewportSize(ViewportSize);
			}

			// 현재 앵커 가져오기
			FAnchors CurrentAnchors = CanvasSlot->GetAnchors();

			PRINTLOG(TEXT("Viewport Size: %s, InPosition: %s, Anchors: Min(%f,%f) Max(%f,%f)"),
				*ViewportSize.ToString(), *InPosition.ToString(),
				CurrentAnchors.Minimum.X, CurrentAnchors.Minimum.Y,
				CurrentAnchors.Maximum.X, CurrentAnchors.Maximum.Y);

			// 최종 위치 계산
			FVector2D FinalPosition = InPosition;

			// 앵커가 오른쪽 상단 (1, 0)인 경우
			if (FMath::IsNearlyEqual(CurrentAnchors.Minimum.X, 1.0f) &&
				FMath::IsNearlyEqual(CurrentAnchors.Maximum.X, 1.0f))
			{
				// Alignment를 왼쪽 상단 (0, 0)으로 설정
				// 이렇게 하면 Position이 드롭다운의 왼쪽 상단 모서리를 가리킴
				// → 드롭다운의 왼쪽 끝이 버튼의 왼쪽 끝에 정렬됨
				FVector2D BeforeAlignment = CanvasSlot->GetAlignment();
				CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
				FVector2D AfterAlignment = CanvasSlot->GetAlignment();

				PRINTLOG(TEXT("Alignment changed: Before(%f,%f) → After(%f,%f)"),
					BeforeAlignment.X, BeforeAlignment.Y, AfterAlignment.X, AfterAlignment.Y);

				// InPosition은 버튼의 왼쪽 끝 아래쪽 (화면 절대 좌표)
				// 오른쪽 앵커 기준 좌표로 변환: 절대 좌표 - 화면 너비
				FinalPosition.X = InPosition.X - ViewportSize.X;
				FinalPosition.Y = InPosition.Y; // Y는 위쪽 앵커(0)라 그대로

				PRINTLOG(TEXT("Right-anchored left-aligned dropdown: ButtonLeftX=%f, ViewportWidth=%f, FinalX=%f"),
					InPosition.X, ViewportSize.X, FinalPosition.X);
			}
			else
			{
				// 왼쪽 앵커인 경우 Alignment (0, 0)
				CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));

				// 레이아웃 강제 계산
				TSharedPtr<SWidget> SlateWidget = SizeBoxWidget->GetCachedWidget();
				if (SlateWidget.IsValid())
				{
					SlateWidget->SlatePrepass(1.0f);
				}

				FVector2D DropdownSize = SizeBoxWidget->GetDesiredSize();

				// 오른쪽으로 벗어나는지 확인
				if (DropdownSize.X > 0 && FinalPosition.X + DropdownSize.X > ViewportSize.X)
				{
					FinalPosition.X = ViewportSize.X - DropdownSize.X;
					PRINTLOG(TEXT("Dropdown adjusted to prevent right overflow: X=%f"), FinalPosition.X);
				}
			}

			// 위치 설정
			CanvasSlot->SetPosition(FinalPosition);

			// 레이아웃 강제 갱신
			if (SizeBoxWidget)
			{
				SizeBoxWidget->ForceLayoutPrepass();
			}

			// 최종 확인
			FVector2D FinalAlignment = CanvasSlot->GetAlignment();
			FVector2D FinalPos = CanvasSlot->GetPosition();
			PRINTLOG(TEXT("Final state - Position: %s, Alignment: (%f,%f)"),
				*FinalPos.ToString(), FinalAlignment.X, FinalAlignment.Y);

			PRINTLOG(TEXT("Dropdown positioned successfully at %s"), *FinalPosition.ToString());
		}
		else
		{
			PRINTLOG(TEXT("Failed to cast to CanvasPanelSlot! Widget: %s, Slot: %s"),
				*SizeBoxWidget->GetName(), SizeBoxWidget->Slot ? *SizeBoxWidget->Slot->GetName() : TEXT("Null"));
		}
	}
	else
	{
		PRINTLOG(TEXT("RootCanvas is null or has no children"));
	}
}

void UMVE_WidgetClass_Dropdown::CloseDropdown()
{
	RemoveFromParent();
}
