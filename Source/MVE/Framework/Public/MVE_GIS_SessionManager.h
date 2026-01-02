
#pragma once

#include "CoreMinimal.h"
#include "MVE_API_ResponseData.h"
#include "Data/RoomInfo.h"
#include "StageLevel/Data/MVE_EffectSequenceData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MVE_GIS_SessionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionsFound, bool, bSuccess, int32, NumSessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBroadcastStateChanged, bool, bIsOpen);

UCLASS()
class MVE_API UMVE_GIS_SessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void CreateSession(const FConcertInfo& ConcertInfo);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void FindSessions();
    
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void JoinSession(int32 Index);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void JoinSessionByRoomId(const FString& RoomId);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void DestroySession();
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    const TArray<FConcertInfo>& GetConcertList() const { return ConcertList; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    bool GetConcertInfoAtIndex(int32 Index, FConcertInfo& OutConcertInfo) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    FString GetCurrentSessionName();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    FString GetCurrentRoomID();

    // ===== 콘서트 재생목록 관리 =====

    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void SetPendingPlaylist(const TArray<FAudioFile>& Playlist);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    TArray<FAudioFile> GetPendingPlaylist() const { return PendingConcertPlaylist; }

    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void ClearPendingPlaylist();

    // ===== 곡별 이펙트 시퀀스 관리 =====

    /**
     * 특정 곡의 이펙트 시퀀스 저장
     * @param AudioId 오디오 파일 ID
     * @param SequenceData 이펙트 시퀀스 데이터
     */
    UFUNCTION(BlueprintCallable, Category = "MVE|Session|EffectSequence")
    void SetEffectSequenceForAudio(int32 AudioId, const TArray<FEffectSequenceData>& SequenceData);

    /**
     * 특정 곡의 이펙트 시퀀스 가져오기
     * @param AudioId 오디오 파일 ID
     * @return 이펙트 시퀀스 데이터 (없으면 빈 배열)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session|EffectSequence")
    TArray<FEffectSequenceData> GetEffectSequenceForAudio(int32 AudioId) const;

    /**
     * 특정 곡의 이펙트 시퀀스가 있는지 확인
     * @param AudioId 오디오 파일 ID
     * @return 이펙트 시퀀스 존재 여부
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session|EffectSequence")
    bool HasEffectSequenceForAudio(int32 AudioId) const;

    /**
     * 모든 이펙트 시퀀스 데이터 초기화
     */
    UFUNCTION(BlueprintCallable, Category = "MVE|Session|EffectSequence")
    void ClearAllEffectSequences();

    UPROPERTY(BlueprintAssignable, Category = "MVE|Session")
    FOnSessionCreated OnSessionCreated;
    
    UPROPERTY(BlueprintAssignable, Category = "MVE|Session")
    FOnSessionsFound OnSessionsFound;
    
    UPROPERTY(BlueprintAssignable, Category = "MVE|Session")
    FOnSessionJoined OnSessionJoined;

    // ===== 방송 시작/종료 =====
    
    // 방송 시작 (bIsOpen = true)
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void StartBroadcast();
    
    // 방송 종료 (bIsOpen = false, 검색 불가능하지만 세션은 유지)
    UFUNCTION(BlueprintCallable, Category = "MVE|Session")
    void StopBroadcast();
    
    // 현재 방송 중인지 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Session")
    bool IsBroadcasting() const { return bIsBroadcasting; }
    
    // 델리게이트 추가
    UPROPERTY(BlueprintAssignable, Category = "MVE|Session")
    FOnBroadcastStateChanged OnBroadcastStateChanged;

private:
    void CreateConcertInternal(const FConcertInfo& ConcertInfo);
    void OnCreateConcertComplete(bool bSuccess, const FConcertCreationData& Data, const FString& ErrorCode);
    void RegisterListenServer();
    void OnRegisterListenServerComplete(bool bSuccess, const FRegisterListenServerData& Data, const FString& ErrorCode);
    void OpenConcert();
    void OnToggleConcertOpenComplete(bool bSuccess, const FString& MessageOrError);
    
    void OnGetConcertListComplete(bool bSuccess, const FGetConcertListData& Data, const FString& ErrorCode);
    
    void JoinConcertInternal(const FString& RoomId);
    void OnGetConcertInfoForJoin(bool bSuccess, const FGetConcertInfoResponseData& Data, const FString& ErrorCode);
    void OnJoinConcertComplete(bool bSuccess, const FString& MessageOrError);
    
    void OnLeaveConcertComplete(bool bSuccess, const FString& MessageOrError);
    void OnCloseConcertComplete(bool bSuccess, const FString& MessageOrError);
    
    int32 GetUniqueClientId() const;

    UPROPERTY()
    FString CurrentRoomId;

    // 세션 생성한 방 정보
    UPROPERTY()
    FConcertInfo PendingConcertInfo;

    // 세션 참여한 방 정보
    UPROPERTY()
    FString JoinedConcertName;

    UPROPERTY()
    TArray<FConcertInfo> ConcertList;

    // 콘서트 생성 시 사용할 재생목록
    UPROPERTY()
    TArray<FAudioFile> PendingConcertPlaylist;

    // 곡별 이펙트 시퀀스 저장소 (AudioFileId → EffectSequenceData)
    UPROPERTY()
    TMap<int32, FEffectSequenceDataArray> AudioEffectSequenceMap;


    UPROPERTY()
    bool bIsHost;
    
    UPROPERTY()
    FString PendingTravelIP;
    
    UPROPERTY()
    int32 PendingTravelPort;

    UPROPERTY()
    bool bIsBroadcasting; // 현재 방송 주 ㅇ여부
};

