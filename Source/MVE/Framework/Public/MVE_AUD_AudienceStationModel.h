
#pragma once

#include "CoreMinimal.h"
#include "Data/RoomInfo.h"
#include "UObject/Object.h"
#include "MVE_AUD_AudienceStationModel.generated.h"

class UMVE_GIS_SessionManager;

UCLASS()
class MVE_API UMVE_AUD_AudienceStationModel : public UObject
{
	GENERATED_BODY()

	
private:
	// SessionManager 참조
	UPROPERTY()
	TObjectPtr<UMVE_GIS_SessionManager> SessionManager;

	// 전체 방 목록 (SessionManager로부터 받음)
	UPROPERTY()
	TArray<FRoomInfo> AllRooms;

	// 필터링된 방 목록
	UPROPERTY()
	TArray<FRoomInfo> SearchedRooms;

	// 검색/필터 상태
	FString CurrentSearchQuery;

public:
	/**
	 * 초기화
	 */
	void Initialize(UMVE_GIS_SessionManager* InSessionManager);

	/**
	 * 세션 목록 새로고침 요청
	 */
	void RefreshRoomList();

	/**
	 * 검색
	 */
	void SearchRooms(const FString& SearchQuery);

	/**
     * 검색된 방 목록 가져오기
     */
    const TArray<FRoomInfo>& GetSearchedRooms() const { return SearchedRooms; }

	/**
	* 전체 방 목록 가져오기
	*/
	const TArray<FRoomInfo>& GetAllRooms() const { return AllRooms; }

	// 방 목록 업데이트 델리게이트
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomListUpdated, const TArray<FRoomInfo>&);
	FOnRoomListUpdated OnRoomListUpdated;

private:
	/**
	 * SessionManager 이벤트 바인딩
	 */
	void BindSessionManagerEvents();

	/**
	 * 세션 찾기 완료 핸들러
	 */
	//UFUNCTION()
	void OnSessionsFound(bool bSuccess, const TArray<FRoomInfo>& Sessions);

	/**
	 * 필터 적용
	 */
	void PerformSearch();
	
};
