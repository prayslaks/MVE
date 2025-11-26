
#pragma once

#include "CoreMinimal.h"
#include "Data/RoomInfo.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MVE_GIS_SessionManager.generated.h"

UCLASS()
class MVE_API UMVE_GIS_SessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 세션(방) 생성
	 * @param RoomInfo 방 정보
	 * @param RequestingPlayer 세션 생성을 요청한 PlayerController (이동시킬 플레이어)
	 */
	UFUNCTION(BlueprintCallable)
	void CreateSession(const FRoomInfo& RoomInfo, APlayerController* RequestingPlayer = nullptr);

	/**
	 * 세션 목록 조회 (Find Sessions)
	 */
	UFUNCTION(BlueprintCallable)
	void FindSessions();

	/**
	 * 세션 참가
	 */
	UFUNCTION(BlueprintCallable)
	void JoinSession(const FString& SessionId);

	/**
	 * 세션 나가기
	 */
	UFUNCTION(BlueprintCallable)
	void LeaveSession();

	/**
	 * 세션 삭제
	 */
	UFUNCTION(BlueprintCallable)
	void DestroySession();

	// === 델리게이트 (이벤트 알림) ===

	// 세션 생성 완료
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool /* bSuccess */);
	FOnSessionCreated OnSessionCreated;

	// 세션 목록 조회 완료
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionsFound, bool /* bSuccess */, const TArray<FRoomInfo>& /* Sessions */);
	FOnSessionsFound OnSessionsFound;

	// 세션 참가 완료
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool /* bSuccess */);
	FOnSessionJoined OnSessionJoined;

private:

	FString StringBase64Encode(FString str);
	FString StringBase64Decode(FString str);
	
	IOnlineSessionPtr SessionInterface;

	// 세션 검색 설정
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 콜백 함수들
	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnLeaveSessionComplete(FName SessionName, bool bSuccess);

	// 델리게이트 핸들
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle LeaveSessionDelegateHandle;

	/**
	 * OnlineSessionSearchResult → FRoomInfo 변환
	 */
	FRoomInfo ConvertSearchResultToRoomInfo(const FOnlineSessionSearchResult& SearchResult);

	// 세션 생성 대기 중인 방 정보
	FRoomInfo PendingRoomInfo;

	// 세션 생성 대기 플래그
	bool bPendingCreateSession = false;

	// 세션 생성을 요청한 PlayerController
	UPROPERTY()
	TObjectPtr<APlayerController> RequestingPlayerController;

	// 실제 세션 생성 로직
	void CreateSessionInternal(const FRoomInfo& RoomInfo, APlayerController* RequestingPlayer);

};

