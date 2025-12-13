
#include "../Public/MVE_WidgetClass_Dropdown.h"
#include "MVE.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UMVE_WidgetClass_Dropdown::SetDropdownPosition(const FVector2D& InPosition, float InButtonHeight, EDropdownAnchorPosition AnchorPosition)
{
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
    
    if (!RootCanvas || RootCanvas->GetChildrenCount() == 0)
    {
        PRINTLOG(TEXT("Invalid root canvas"));
        return;
    }

    // 첫 번째 자식 (Size Box)
    UWidget* SizeBoxWidget = RootCanvas->GetChildAt(0);
    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SizeBoxWidget->Slot);
    
    if (!CanvasSlot)
    {
        PRINTLOG(TEXT("No CanvasPanelSlot"));
        return;
    }

    // 기본 설정
    CanvasSlot->SetAutoSize(true);
    CanvasSlot->SetZOrder(1);
    
    // 레이아웃 계산 (드롭다운 크기 알아내기)
    SizeBoxWidget->ForceLayoutPrepass();
    FVector2D DropdownSize = SizeBoxWidget->GetDesiredSize();
    
    // 화면 크기 가져오기
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    
    // 앵커와 정렬, 위치 계산
    FAnchors Anchors;
    FVector2D Alignment;
    FVector2D FinalPosition;
    
    switch (AnchorPosition)
    {
    case EDropdownAnchorPosition::TopLeft:
        {
            // 앵커: 좌상단 (0, 0)
            Anchors = FAnchors(0.f, 0.f, 0.f, 0.f);
            Alignment = FVector2D(0.f, 0.f);
            
            // InPosition은 버튼의 왼쪽 상단
            // 드롭다운은 버튼 아래에 표시
            FinalPosition = InPosition + FVector2D(0, InButtonHeight+DropdownVerticalOffset);
            
            // 오른쪽 오버플로우 체크
            if (FinalPosition.X + DropdownSize.X > ViewportSize.X)
            {
                FinalPosition.X = ViewportSize.X - DropdownSize.X;
            }
            
            // 아래쪽 오버플로우 체크
            if (FinalPosition.Y + DropdownSize.Y > ViewportSize.Y)
            {
                // 버튼 위로 표시
                FinalPosition.Y = InPosition.Y - DropdownSize.Y;
            }
        }
        break;
        
    case EDropdownAnchorPosition::TopRight:
        {
            // 앵커: 우상단 (1, 0)
            Anchors = FAnchors(1.f, 0.f, 1.f, 0.f);
            Alignment = FVector2D(1.f, 0.f); // 오른쪽 정렬
            
            // InPosition은 버튼의 오른쪽 상단
            // 우상단 앵커 기준 좌표로 변환
            FinalPosition.X = InPosition.X - ViewportSize.X;
            FinalPosition.Y = InPosition.Y + InButtonHeight + DropdownVerticalOffset;
            
            // 왼쪽 오버플로우 체크
            if (InPosition.X - DropdownSize.X < 0)
            {
                FinalPosition.X = -ViewportSize.X + DropdownSize.X;
            }
            
            // 아래쪽 오버플로우 체크
            if (FinalPosition.Y + DropdownSize.Y > ViewportSize.Y)
            {
                FinalPosition.Y = InPosition.Y - DropdownSize.Y;
            }
        }
        break;
        
    case EDropdownAnchorPosition::MiddleLeft:
        {
            // 앵커: 좌측 중앙 (0, 0.5)
            Anchors = FAnchors(0.f, 0.5f, 0.f, 0.5f);
            Alignment = FVector2D(0.f, 0.5f);
            
            // InPosition은 버튼의 왼쪽 중앙
            FinalPosition.X = InPosition.X;
            FinalPosition.Y = InPosition.Y - (ViewportSize.Y * 0.5f) + DropdownVerticalOffset;
            
            // 오른쪽 오버플로우 체크
            if (FinalPosition.X + DropdownSize.X > ViewportSize.X)
            {
                FinalPosition.X = ViewportSize.X - DropdownSize.X;
            }
        }
        break;
        
    case EDropdownAnchorPosition::MiddleRight:
        {
            // 앵커: 우측 중앙 (1, 0.5)
            Anchors = FAnchors(1.f, 0.5f, 1.f, 0.5f);
            Alignment = FVector2D(1.f, 0.5f);
            
            // InPosition은 버튼의 오른쪽 중앙
            FinalPosition.X = InPosition.X - ViewportSize.X;
            FinalPosition.Y = InPosition.Y - (ViewportSize.Y * 0.5f) + DropdownVerticalOffset;
            
            // 왼쪽 오버플로우 체크
            if (InPosition.X - DropdownSize.X < 0)
            {
                FinalPosition.X = -ViewportSize.X + DropdownSize.X;
            }
        }
        break;
        
    case EDropdownAnchorPosition::BottomLeft:
        {
            // 앵커: 좌하단 (0, 1)
            Anchors = FAnchors(0.f, 1.f, 0.f, 1.f);
            Alignment = FVector2D(0.f, 1.f);
            
            // InPosition은 버튼의 왼쪽 하단
            FinalPosition.X = InPosition.X;
            FinalPosition.Y = InPosition.Y + InButtonHeight - ViewportSize.Y + DropdownVerticalOffset;
            
            // 오른쪽 오버플로우 체크
            if (FinalPosition.X + DropdownSize.X > ViewportSize.X)
            {
                FinalPosition.X = ViewportSize.X - DropdownSize.X;
            }
        }
        break;
        
    case EDropdownAnchorPosition::BottomRight:
        {
            // 앵커: 우하단 (1, 1)
            Anchors = FAnchors(1.f, 1.f, 1.f, 1.f);
            Alignment = FVector2D(1.f, 1.f);
            
            // InPosition은 버튼의 오른쪽 하단
            FinalPosition.X = InPosition.X - ViewportSize.X;
            FinalPosition.Y = InPosition.Y + InButtonHeight - ViewportSize.Y + DropdownVerticalOffset;
            
            // 왼쪽 오버플로우 체크
            if (InPosition.X - DropdownSize.X < 0)
            {
                FinalPosition.X = -ViewportSize.X + DropdownSize.X;
            }
        }
        break;
    }
    
    // 설정 적용
    CanvasSlot->SetAnchors(Anchors);
    CanvasSlot->SetAlignment(Alignment);
    CanvasSlot->SetPosition(FinalPosition);
    
    PRINTLOG(TEXT("Dropdown positioned - Anchor: %d, Position: %s, Alignment: %s"), 
        (int32)AnchorPosition, *FinalPosition.ToString(), *Alignment.ToString());
}

void UMVE_WidgetClass_Dropdown::CloseDropdown()
{
	RemoveFromParent();
}
