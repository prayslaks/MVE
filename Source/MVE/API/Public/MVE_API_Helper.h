#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Json.h"
#include "MVE_API_ResponseData.h"
#include "MVE_API_Helper.generated.h"

UCLASS()
class MVE_API UMVE_API_Helper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    // 초기화
    static void Initialize();
    
    // 로컬 IP 주소와 포트 번호를 획득
    static bool GetHostLocalIPAndPort(const UObject* WorldContextObject, FString& OutLocalIP, int32& OutPort);
    
    // 리스폰스에서 특정 필드 추출
    static bool GetJsonFieldString(const FString& InResponseBody, const FString& InField, FString& OutFieldValue);
    
    // 에러 메시지 파싱
    static bool ParseError(const FString& ResponseBody, FString& OutErrorCode, FString& OutErrorMessage);
    
    // --- C++ Native API Delegates ---
    
    // 서버 헬스 체크 API
    static void LoginHealthCheck(const FOnGenericApiComplete& OnResult);
    static void ResourceHealthCheck(const FOnGenericApiComplete& OnResult);

    // 로그인 서버 API
    static void CheckEmail(const FString& Email, const FOnCheckEmailComplete& OnResult);
    static void SendVerificationCode(const FString& Email, const FOnGenericApiComplete& OnResult);
    static void TryConfirmVerifyCode(const FString& Email, const FString& Code, const FOnGenericApiComplete& OnResult);
    static void SignUp(const FString& Email, const FString& Password, const FString& Code, const FOnSignUpComplete& OnResult);
    static void Login(const FString& Email, const FString& Password, const FMVEOnLoginComplete& OnResult);
    static void Logout(const FOnGenericApiComplete& OnResult);
    static void Withdraw(const FString& Password, const FOnGenericApiComplete& OnResult);
    static void GetProfile(const FOnGetProfileComplete& OnResult);

    // 리소스 서버 API
    static void SaveAccessoryPreset(const FString& PresetName, const TArray<TSharedPtr<FJsonValue>>& Accessories, const FString& Description, bool bIsPublic, const FOnSavePresetComplete& OnResult);
    static void GetPresetList(bool bIncludePublic, const FOnGetPresetListComplete& OnResult);
    static void GetPresetDetail(int32 PresetId, const FOnGetPresetDetailComplete& OnResult);
    static void UpdatePreset(int32 PresetId, const FString& PresetName, const TArray<TSharedPtr<FJsonValue>>& Accessories, const FString& Description, bool bIsPublic, const FOnGenericApiComplete& OnResult);
    static void DeletePreset(int32 PresetId, const FOnGenericApiComplete& OnResult);
    
    static void GetAudioList(const FOnGetAudioListComplete& OnResult);
    static void StreamAudio(int32 AudioId, const FOnStreamAudioComplete& OnResult);
    static void UploadAudio(const FString& FilePath, const FString& Title, const FString& Artist, float Duration, const FOnUploadAudioComplete& OnResult);
    static void SearchAudio(const FString& Query, const FOnSearchAudioComplete& OnResult);
    static void GetAudioDetail(int32 AudioId, const FOnGetAudioDetailComplete& OnResult);
    static void DownloadAudioFile(int32 AudioId, const FString& SavePath, const FOnGenericApiComplete& OnResult);
    
	static void CreateConcert(const FString& ConcertName, const TArray<FConcertSong>& Songs, const TArray<FAccessory>& Accessories, int32 MaxAudience, const FOnCreateConcertComplete& OnResult);
    static void GetConcertInfo(const FString& RoomId, const FOnGetConcertInfoComplete& OnResult);
    static void GetConcertList(const FOnGetConcertListComplete& OnResult);
    static void JoinConcert(const FString& RoomId, int32 ClientId, const FOnGenericApiComplete& OnResult);
    static void LeaveConcert(const FString& RoomId, int32 ClientId, const FOnGenericApiComplete& OnResult);
    
	static void RegisterListenServer(const FString& RoomId, const FString& LocalIP, int32 Port, const FString& PublicIP, int32 PublicPort, const FOnRegisterListenServerComplete& OnResult);
    static void ToggleConcertOpen(const FString& RoomId, bool bIsOpen, const FOnGenericApiComplete& OnResult);
    
	static void GetModelList(const FString& RoomId, const FOnGetModelListComplete& OnResult);
    static void GenerateModel(const FString& Prompt, const FString& ImagePath, const FOnGenerateModelComplete& OnResult);
    static void GetModelGenerationStatus(const FString& JobId, const FOnGetJobStatusComplete& OnResult);
    static void UploadModel(const FString& ModelPath, const FString& ThumbnailPath, const FString& ModelName, const FOnUploadModelComplete& OnResult);
    //static void DownloadModel(int32 ModelId, const FString& SavePath, const FOnGenericApiComplete& OnResult);
    static void GetModelDownloadUrl(int32 ModelId, const FOnGetModelDownloadUrlComplete& OnResult);
    static void DeleteModel(int32 ModelId, const FOnDeleteModelComplete& OnResult);

    // TODO: Not Implemented
    // static void AddSongToConcert(const FString& RoomId, int32 SongNum, int32 AudioId, const FString& StreamUrl, int32 StageDirectionId, const FOnGenericApiComplete& OnResult);
    // static void RemoveSongFromConcert(const FString& RoomId, int32 SongNum, const FOnGenericApiComplete& OnResult);
    // static void ChangeConcertSong(const FString& RoomId, int32 SongNum, const FOnGenericApiComplete& OnResult);
    // static void GetCurrentConcertSong(const FString& RoomId, const FOnGetCurrentConcertSongComplete& OnResult);
    // static void AddConcertAccessory(const FString& RoomId, const FString& SocketName, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FString& ModelUrl, const FOnGenericApiComplete& OnResult);
    // static void RemoveConcertAccessory(const FString& RoomId, int32 AccessoryIndex, const FOnGenericApiComplete& OnResult);
    // static void ReplaceConcertAccessories(const FString& RoomId, const TArray<FAccessoryInfo>& Accessories, const FOnGenericApiComplete& OnResult);
    // static void ExpireAllConcerts(const FOnGenericApiComplete& OnResult);
    // static void UploadModelFromAI(const FString& ModelPath, const FString& ThumbnailPath, const FString& JobId, const FString& JobSecret, const FOnUploadModelComplete& OnResult);

    // --- Blueprint Callable API Wrappers ---
	
	UFUNCTION(BlueprintCallable, Category = "MVE|API", meta=(WorldContext="WorldContextObject"), BlueprintPure)
	static bool GetHostLocalIPAndPortBP(const UObject* WorldContextObject, FString& OutLocalIP, int32& OutPort);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API", meta=(WorldContext="WorldContextObject"))
    static void LoginHealthCheckBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API", meta=(WorldContext="WorldContextObject"))
    static void ResourceHealthCheckBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void CheckEmailBP(UObject* WorldContextObject, const FString& Email, const FOnCheckEmailCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void SendVerificationCodeBP(UObject* WorldContextObject, const FString& Email, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void TryConfirmVerifyCodeBP(UObject* WorldContextObject, const FString& Email, const FString& Code, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void SignUpBP(UObject* WorldContextObject, const FString& Email, const FString& Password, const FString& Code, const FOnSignUpCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void LoginBP(UObject* WorldContextObject, const FString& Email, const FString& Password, const FOnLoginCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void LogoutBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void WithdrawBP(UObject* WorldContextObject, const FString& Password, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Auth", meta=(WorldContext="WorldContextObject"))
    static void GetProfileBP(UObject* WorldContextObject, const FOnGetProfileCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void SaveAccessoryPresetBP(UObject* WorldContextObject, const FString& PresetName, const FString& AccessoriesJson, const FString& Description, bool bIsPublic, const FOnSavePresetCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void GetPresetListBP(UObject* WorldContextObject, bool bIncludePublic, const FOnGetPresetListCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void GetPresetDetailBP(UObject* WorldContextObject, int32 PresetId, const FOnGetPresetDetailCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo"))
    static void UpdatePresetBP(UObject* WorldContextObject, int32 PresetId, const FString& PresetName, const FString& AccessoriesJson, const FString& Description, bool bIsPublic, const FOnGenericApiCompleteBP& OnResult, FLatentActionInfo LatentInfo);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void DeletePresetBP(UObject* WorldContextObject, int32 PresetId, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GetAudioListBP(UObject* WorldContextObject, const FOnGetAudioListCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void StreamAudioBP(UObject* WorldContextObject, int32 AudioId, const FOnStreamAudioCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void UploadAudioBP(UObject* WorldContextObject, const FString& FilePath, const FString& Title, const FString& Artist, float Duration, const FOnUploadAudioCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void SearchAudioBP(UObject* WorldContextObject, const FString& Query, const FOnSearchAudioCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void GetAudioDetailBP(UObject* WorldContextObject, int32 AudioId, const FOnGetAudioDetailCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void DownloadAudioFileBP(UObject* WorldContextObject, int32 AudioId, const FString& SavePath, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void CreateConcertBP(UObject* WorldContextObject, const FString& ConcertName, const TArray<FConcertSong>& Songs, const TArray<FAccessory>& Accessories, int32 MaxAudience, const FOnCreateConcertCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GetConcertInfoBP(UObject* WorldContextObject, const FString& RoomId, const FOnGetConcertInfoCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void GetConcertListBP(UObject* WorldContextObject, const FOnGetConcertListCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void JoinConcertBP(UObject* WorldContextObject, const FString& RoomId, int32 ClientId, const FOnGenericApiCompleteBP& OnResult);

    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta = (WorldContext = "WorldContextObject"))
    static void LeaveConcertBP(UObject* WorldContextObject, const FString& RoomId, int32 ClientId, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void RegisterListenServerBP(UObject* WorldContextObject, const FString& RoomId, const FString& LocalIP, int32 Port, const FString& PublicIP, int32 PublicPort, const FOnRegisterListenServerCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void ToggleConcertOpenBP(UObject* WorldContextObject, const FString& RoomId, bool bIsOpen, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GetModelListBP(UObject* WorldContextObject, const FString& RoomId, const FOnGetModelListCompleteBP& OnResultBP);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GenerateModelBP(UObject* WorldContextObject, const FString& Prompt, const FString& ImagePath, const FOnGenerateModelCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GetModelGenerationStatusBP(UObject* WorldContextObject, const FString& JobId, const FOnGetJobStatusCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void UploadModelBP(UObject* WorldContextObject, const FString& ModelPath, const FString& ThumbnailPath, const FString& ModelName, const FOnUploadModelCompleteBP& OnResult);
    
    // UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    // static void DownloadModelBP(UObject* WorldContextObject, int32 ModelId, const FString& SavePath, const FOnGenericApiCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void GetModelDownloadUrlBP(UObject* WorldContextObject, int32 ModelId, const FOnGetModelDownloadUrlCompleteBP& OnResult);
    
    UFUNCTION(BlueprintCallable, Category = "MVE|API|Resource", meta=(WorldContext="WorldContextObject"))
    static void DeleteModelBP(UObject* WorldContextObject, int32 ModelId, const FOnDeleteModelCompleteBP& OnResult);

    // 인증 JWT 토큰
    UFUNCTION(BlueprintCallable, Category = "MVE|API")
    static FString GetAuthToken();
	
    UFUNCTION(BlueprintCallable, Category = "MVE|API")
    static void SetAuthToken(const FString& Token);

private:
    inline static FString LoginServerURL;
    inline static FString ResourceServerURL;
    inline static FString GlobalAuthToken;
};
