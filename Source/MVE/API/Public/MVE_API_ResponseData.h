#pragma once

#include "CoreMinimal.h"
#include "MVE_API_ResponseData.generated.h"
/*
 *********************************************************************************
 * [공용] 자주 사용되는 데이터 구조체
 *********************************************************************************
 */

USTRUCT(BlueprintType)
struct FAudioFile
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString title;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString artist;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString file_path;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    int64 file_size = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    int32 duration = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString format;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString created_at;
};

USTRUCT(BlueprintType)
struct FModelInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString model_name;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString file_path;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    int64 file_size = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString thumbnail_path;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    bool is_ai_generated = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString created_at;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Model")
    FString updated_at;
};

/*
 *********************************************************************************
 * [인증 서버] API 응답 구조체 및 델리게이트
 *********************************************************************************
 */

USTRUCT(BlueprintType)
struct FCheckEmailResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool exists = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
};
DECLARE_DELEGATE_ThreeParams(FOnCheckEmailComplete, bool, const FCheckEmailResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnCheckEmailCompleteBP, bool, bSuccess, const FCheckEmailResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FSignUpUser
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString email;
};
USTRUCT(BlueprintType)
struct FSignUpResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FSignUpUser user;
};
DECLARE_DELEGATE_ThreeParams(FOnSignUpComplete, bool, const FSignUpResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnSignUpCompleteBP, bool, bSuccess, const FSignUpResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FLoginUser
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString email;
};
USTRUCT(BlueprintType)
struct FLoginResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString token;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FLoginUser user;
};
DECLARE_DELEGATE_ThreeParams(FMVEOnLoginComplete, bool, const FLoginResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnLoginCompleteBP, bool, bSuccess, const FLoginResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FProfileResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString email;
    // Add other profile fields as needed
};
DECLARE_DELEGATE_ThreeParams(FOnGetProfileComplete, bool, const FProfileResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetProfileCompleteBP, bool, bSuccess, const FProfileResponseData&, ResponseData, const FString&, ErrorCode);

/*
 *********************************************************************************
 * [리소스 서버] API 응답 구조체 및 델리게이트
 *********************************************************************************
 */

USTRUCT(BlueprintType)
struct FSavePresetResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
};
DECLARE_DELEGATE_ThreeParams(FOnSavePresetComplete, bool, const FSavePresetResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnSavePresetCompleteBP, bool, bSuccess, const FSavePresetResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FAccessoryPreset
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    int32 id = 0;
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    FString presetName;
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    FString accessories;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    FString description;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    bool isPublic = false;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    FString created_at;
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Preset")
    FString updated_at;
};

USTRUCT(BlueprintType)
struct FGetPresetListResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 count = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    TArray<FAccessoryPreset> presets;
};
DECLARE_DELEGATE_ThreeParams(FOnGetPresetListComplete, bool, const FGetPresetListResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetPresetListCompleteBP, bool, bSuccess, const FGetPresetListResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FGetPresetDetailResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FAccessoryPreset preset;
};
DECLARE_DELEGATE_ThreeParams(FOnGetPresetDetailComplete, bool, const FGetPresetDetailResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetPresetDetailCompleteBP, bool, bSuccess, const FGetPresetDetailResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FAudioListResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 count = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    TArray<FAudioFile> audio_files;
};
DECLARE_DELEGATE_ThreeParams(FOnGetAudioListComplete, bool, const FAudioListResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetAudioListCompleteBP, bool, bSuccess, const FAudioListResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FStreamAudioFile
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString title;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString format;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int64 file_size = 0;
};

USTRUCT(BlueprintType)
struct FStreamAudioResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString stream_url;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FStreamAudioFile audio_file;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 expires_in = 0;
};
DECLARE_DELEGATE_ThreeParams(FOnStreamAudioComplete, bool, const FStreamAudioResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnStreamAudioCompleteBP, bool, bSuccess, const FStreamAudioResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FUploadAudioResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FAudioFile audio_file;
};
DECLARE_DELEGATE_ThreeParams(FOnUploadAudioComplete, bool, const FUploadAudioResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnUploadAudioCompleteBP, bool, bSuccess, const FUploadAudioResponseData&, ResponseData, const FString&, ErrorCode);

DECLARE_DELEGATE_ThreeParams(FOnSearchAudioComplete, bool, const FAudioListResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnSearchAudioCompleteBP, bool, bSuccess, const FAudioListResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FGetAudioDetailResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FAudioFile audio_file;
};
DECLARE_DELEGATE_ThreeParams(FOnGetAudioDetailComplete, bool, const FGetAudioDetailResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetAudioDetailCompleteBP, bool, bSuccess, const FGetAudioDetailResponseData&, ResponseData, const FString&, ErrorCode);

// 콘서트 세션 생성
USTRUCT(BlueprintType)
struct FConcertCreationData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
	bool success = false;
	
	UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
	FString roomId;

	UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
	int32 expiresIn = 0;
};
DECLARE_DELEGATE_ThreeParams(FOnCreateConcertComplete, bool, const FConcertCreationData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnCreateConcertCompleteBP, bool, bSuccess, const FConcertCreationData&, SuccessData, const FString&, ErrorCode);

// 리슨 서버 정보 등록
USTRUCT(BlueprintType)
struct FListenServerDetails
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString localIP;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 port = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString publicIP;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 publicPort = 0;
};
USTRUCT(BlueprintType)
struct FRegisterListenServerData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FListenServerDetails listenServer;
};
DECLARE_DELEGATE_ThreeParams(FOnRegisterListenServerComplete, bool, const FRegisterListenServerData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnRegisterListenServerCompleteBP, bool, bSuccess, const FRegisterListenServerData&, SuccessData, const FString&, ErrorMes);

// 콘서트 정보 획득
USTRUCT(BlueprintType)
struct FConcertSong
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 songNum = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 audioId = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString streamUrl;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 stageDirectionId = 0;
};
USTRUCT(BlueprintType)
struct FConcertAccessory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString socketName;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FVector relativeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FRotator relativeRotation = FRotator::ZeroRotator;
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    float relativeScale = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString modelUrl;
};
USTRUCT(BlueprintType)
struct FListenServerInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString localIP;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 port = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString publicIP;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 publicPort = 0;
};
USTRUCT(BlueprintType)
struct FConcertInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString roomId; // Added

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 studioUserId = 0; // Changed from FString to int32

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString studioName;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FString concertName;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    TArray<FConcertSong> songs;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    TArray<FConcertAccessory> accessories;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 maxAudience = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int64 createdAt = 0; // Changed from FString to int64

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    FListenServerInfo listenServer;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    bool isOpen = false;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 currentSong = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response|Concert")
    int32 currentAudience = 0;
};
USTRUCT(BlueprintType)
struct FGetConcertInfoResponseData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FConcertInfo concert;
};
DECLARE_DELEGATE_ThreeParams(FOnGetConcertInfoComplete, bool, const FGetConcertInfoResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetConcertInfoCompleteBP, bool, bSuccess, const FGetConcertInfoResponseData&, ResponseData, const FString&, ErrorCode);

// 콘서트 리스트 획득
USTRUCT(BlueprintType)
struct FGetConcertListData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
	
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    TArray<FConcertInfo> concerts; // Now uses FConcertInfo
};
DECLARE_DELEGATE_ThreeParams(FOnGetConcertListComplete, bool, const FGetConcertListData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetConcertListCompleteBP, bool, bSuccess, const FGetConcertListData&, ResponseData, const FString&, ErrorCode);






USTRUCT(BlueprintType)
struct FGenerateModelResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString job_id;
};
DECLARE_DELEGATE_ThreeParams(FOnGenerateModelComplete, bool, const FGenerateModelResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGenerateModelCompleteBP, bool, bSuccess, const FGenerateModelResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FAIJobStatus
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString job_id;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString status;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString prompt;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString created_at;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString completed_at;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 model_id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString download_url;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString error_message;
};

USTRUCT(BlueprintType)
struct FGetJobStatusResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FAIJobStatus data;
};
DECLARE_DELEGATE_ThreeParams(FOnGetJobStatusComplete, bool, const FGetJobStatusResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetJobStatusCompleteBP, bool, bSuccess, const FGetJobStatusResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FModelListResponseData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 count = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    TArray<FModelInfo> models;
};
DECLARE_DELEGATE_ThreeParams(FOnGetModelListComplete, bool, const FModelListResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetModelListCompleteBP, bool, bSuccess, const FModelListResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FUploadModelResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FModelInfo model;
};
DECLARE_DELEGATE_ThreeParams(FOnUploadModelComplete, bool, const FUploadModelResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnUploadModelCompleteBP, bool, bSuccess, const FUploadModelResponseData&, ResponseData, const FString&, ErrorCode);

USTRUCT(BlueprintType)
struct FDeletedModelInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    int32 id = 0;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString model_name;
};

USTRUCT(BlueprintType)
struct FDeleteModelResponseData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    bool success = false;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FString message;
    UPROPERTY(BlueprintReadOnly, Category="MVE|API Response")
    FDeletedModelInfo deleted_model;
};
DECLARE_DELEGATE_ThreeParams(FOnDeleteModelComplete, bool, const FDeleteModelResponseData&, const FString&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDeleteModelCompleteBP, bool, bSuccess, const FDeleteModelResponseData&, ResponseData, const FString&, ErrorCode);

/*
 *********************************************************************************
 * [일반] 범용 델리게이트
 *********************************************************************************
 */

// C++용: 대부분의 간단한 POST, DELETE 요청은 성공 여부와 메시지만 받으므로 공용으로 사용
DECLARE_DELEGATE_TwoParams(FOnGenericApiComplete, bool /*bSuccess*/, const FString& /*ErrorCode*/);

// 블루프린트용
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGenericApiCompleteBP, bool, bSuccess, const FString&, MessageOrError);

// 기존 FOnApiResponse 는 FOnGenericApiComplete 로 대체될 예정이나, 하위 호환성을 위해 남겨둡니다.
DECLARE_DELEGATE_FourParams(FOnApiResponse, const bool, const FString&, const FString&, const FString&);