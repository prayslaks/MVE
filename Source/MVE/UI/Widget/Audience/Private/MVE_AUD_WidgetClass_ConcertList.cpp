
#include "../Public/MVE_AUD_WidgetClass_ConcertList.h"
#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "Components/TileView.h"

void UMVE_AUD_WidgetClass_ConcertList::NativeConstruct()
{
	Super::NativeConstruct();

	if (RoomTileView)
	{
		// 이벤트 바인딩
		//RoomTileView->OnItemSelectionChanged().AddUObject(this, &UMVE_AUD_WidgetClass_ConcertList::OnRoomItemSelected);
		RoomTileView->OnItemClicked().AddUObject(this, &UMVE_AUD_WidgetClass_ConcertList::OnRoomItemClicked);

		PRINTLOG(TEXT("ListView initialized."));

		if (!TempRoomSessionTableAsset.IsNull())
		{
			UDataTable* DT = TempRoomSessionTableAsset.LoadSynchronous();
			TArray<FDTRoomInfo*> AllRowsPtr;
			DT->GetAllRows(TEXT("LoadAllRooms"), AllRowsPtr);

			TArray<FRoomInfo> RoomInfoArray;
			RoomInfoArray.Reserve(AllRowsPtr.Num());

			for (FDTRoomInfo* RowPtr : AllRowsPtr)
			{
				if (RowPtr)
				{
					RoomInfoArray.Add(RowPtr->RoomInfo);
				}
			}

			UpdateRoomList(RoomInfoArray);
		}
	}
}

void UMVE_AUD_WidgetClass_ConcertList::UpdateRoomList(const TArray<FRoomInfo>& Rooms)
{
	if (!RoomTileView)
	{
		PRINTLOG(TEXT("RoomTileView is nullptr!"));
		return;
	}

	// 기존 데이터 초기화
	ClearRoomList();

	// FRoomInfo → URoomInfoData 변환하여 ListView에 추가
	for (const FRoomInfo& RoomInfo : Rooms)
	{
		// UObject 래퍼 생성
		URoomInfoData* RoomData = NewObject<URoomInfoData>(this);
        
		// 구조체 데이터를 통째로 복사
		RoomData->SetRoomInfo(RoomInfo);

		// TileView에 추가
		RoomTileView->AddItem(RoomData);
	}
}

void UMVE_AUD_WidgetClass_ConcertList::ClearRoomList()
{
	if (RoomTileView)
	{
		RoomTileView->ClearListItems();
	}
}

void UMVE_AUD_WidgetClass_ConcertList::OnRoomItemSelected(UObject* SelectedItem)
{
	// 선택 이벤트. 지금은 안 씀
	URoomInfoData* RoomData = Cast<URoomInfoData>(SelectedItem);
    
	if (RoomData)
	{
		PRINTLOG(TEXT("Room selected: %s"), *RoomData->RoomInfo.RoomTitle);
		OnRoomSelected.Broadcast(RoomData);
	}
}

void UMVE_AUD_WidgetClass_ConcertList::OnRoomItemClicked(UObject* ClickedItem)
{
	URoomInfoData* RoomData = Cast<URoomInfoData>(ClickedItem);
	if (!RoomData)
	{
		PRINTLOG(TEXT("ClickedItem is not URoomInfoData!"));
		return;
	}

	const FRoomInfo& RoomInfo = RoomData->GetRoomInfo();
	PRINTLOG(TEXT("Room clicked: %s (ID: %s)"), 
			 *RoomInfo.RoomTitle, *RoomInfo.RoomID);
	
}
