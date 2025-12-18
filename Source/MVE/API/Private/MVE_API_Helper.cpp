#include "API/Public/MVE_API_Helper.h"
#include "API/Public/MVE_HTTP_Client.h"
#include "API/Public/MVE_API_ResponseData.h"
#include "MVE.h"
#include "JsonUtilities.h"
#include "SocketSubsystem.h" // For ISocketSubsystem
#include "IPAddress.h"     // For FInternetAddr
#include "Engine/World.h"   // For UWorld

#pragma region 네이티브 & 블루프린트 리스폰스 핸들러 매크로 

// 새로운 USTRUCT 기반 응답 핸들러 매크로
#define HANDLE_RESPONSE_STRUCT(StructType, OnResult) \
    FOnHttpResponse::CreateLambda([OnResult](bool bSuccess, const FString& ResponseBody) \
    { \
        PRINTLOG(TEXT("Response: %s"), *ResponseBody); \
        if (bSuccess) \
        { \
            StructType ParsedData; \
            if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseBody, &ParsedData, 0, 0)) \
            { \
                OnResult.ExecuteIfBound(true, ParsedData, TEXT("")); \
            } \
            else \
            { \
                OnResult.ExecuteIfBound(false, StructType(), TEXT("JSON_PARSE_ERROR")); \
            } \
        } \
        else \
        { \
            FString ErrorCode, ErrorMessage; \
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage); \
            OnResult.ExecuteIfBound(false, StructType(), ErrorCode); \
        } \
    })

// 간단한 성공/실패 응답을 위한 핸들러 매크로
#define HANDLE_GENERIC_RESPONSE(OnResult) \
    FOnHttpResponse::CreateLambda([OnResult](bool bSuccess, const FString& ResponseBody) \
    { \
        PRINTLOG(TEXT("Response: %s"), *ResponseBody); \
        FString ErrorCode, ErrorMessage; \
        if (bSuccess) \
        { \
            if (UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage)) \
            { \
                /* Server responded 200 OK, but with a business logic error */ \
                OnResult.ExecuteIfBound(false, ErrorCode); \
            } \
            else \
            { \
                FString Message = TEXT("Success"); \
                UMVE_API_Helper::GetJsonFieldString(ResponseBody, TEXT("message"), Message); \
                OnResult.ExecuteIfBound(true, Message); \
            } \
        } \
        else \
        { \
            /* HTTP request itself failed */ \
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage); \
            OnResult.ExecuteIfBound(false, ErrorCode); \
        } \
    })

// 블루프린트 래퍼 매크로
#define WRAP_DELEGATE(DelegateType, OnResultBP) \
    F##DelegateType OnResult; \
    OnResult.BindLambda([OnResultBP](auto... Args) { \
    OnResultBP.ExecuteIfBound(Args...); \
    });

#pragma endregion

// MVE Server URL - AWS EC2 인스턴스
#define SERVER_URL TEXT("http://ec2-13-125-244-186.ap-northeast-2.compute.amazonaws.com")

// 인증 JWT 토큰 획득
FString UMVE_API_Helper::GetAuthToken()
{
    return GlobalAuthToken;
}

// 인증 JWT 토큰 설정
void UMVE_API_Helper::SetAuthToken(const FString& Token)
{
    GlobalAuthToken = Token;
}

// 초기화
void UMVE_API_Helper::Initialize()
{
    LoginServerURL = FString::Printf(TEXT("%s"), SERVER_URL);
    ResourceServerURL = FString::Printf(TEXT("%s"), SERVER_URL);
    
#if UE_BUILD_DEVELOPMENT
    // 개발 빌드에서는 미리 정의된 개발용 토큰을 사용하여 로그인 과정을 생략합니다.
    // 중요: 이 토큰은 서버의 개발 환경과 동일하게 설정되어야 합니다.
    GlobalAuthToken = TEXT("MVE_DEV_AUTH_TOKEN_2024_A");
    PRINTLOG(TEXT("개발용 인증 토큰이 설정되었습니다. 로그인 과정을 생략합니다."));
#endif
    
    // 로그인 서버 헬스 체크
    {
        FOnGenericApiComplete OnResult;
        OnResult.BindLambda([](const bool bSuccess, const FString& MessageOrError)
        {
            PRINTLOG(TEXT("MVE 로그인 서버 헬스 체크 %s: %s"), bSuccess ? TEXT("정상") : TEXT("비정상"), *MessageOrError);
        });
        LoginHealthCheck(OnResult);   
    }
    
    // 리소스 서버 헬스 체크
    {
        FOnGenericApiComplete OnResult;
        OnResult.BindLambda([](const bool bSuccess, const FString& MessageOrError)
        {
            PRINTLOG(TEXT("MVE 리소스 서버 헬스 체크 %s: %s"), bSuccess ? TEXT("정상") : TEXT("비정상"), *MessageOrError);
        });
        ResourceHealthCheck(OnResult);   
    }
}

// 로스트 IP와 포트 획득
bool UMVE_API_Helper::GetHostLocalIPAndPort(const UObject* WorldContextObject, FString& OutLocalIP, int32& OutPort) 
{
    if (!WorldContextObject || !WorldContextObject->GetWorld())
    {
        PRINTLOG(TEXT("GetHostLocalIPAndPort: Invalid WorldContextObject or World."));
        return false;
    }

    // 1. 로컬 IP 가져오기 (SocketSubsystem 이용)
    bool bCanBindAll;
    TSharedPtr<class FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);

    if (LocalIp.IsValid())
    {
        // 사람이 읽을 수 있는 문자열로 변환 (예: "192.168.0.10")
        OutLocalIP = LocalIp->ToString(false); 
        PRINTLOG(TEXT("Local IP: %s"), *OutLocalIP);
    }
    else
    {
        PRINTLOG(TEXT("GetHostLocalIPAndPort: Failed to get local IP address."));
        return false; // IP를 가져올 수 없음
    }

    // 2. 현재 월드의 포트 가져오기
    OutPort = WorldContextObject->GetWorld()->URL.Port;
    if (OutPort == 0) // Default port is usually 7777 if not specified in URL
    {
        // Fallback to default port if not set in URL, common for listen servers
        OutPort = 7777; 
        PRINTLOG(TEXT("GetHostLocalIPAndPort: Port not found in URL, defaulting to 7777."));
    }
    PRINTLOG(TEXT("Port: %d"), OutPort);
    
    return true;
}

// JSON 리스폰스에서 특정 필드의 값을 파싱
bool UMVE_API_Helper::GetJsonFieldString(const FString& InResponseBody, const FString& InField, FString& OutFieldValue)
{
    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InResponseBody);
    if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
    {
        return JsonResponse->TryGetStringField(InField, OutFieldValue);
    }
    return false;
}

bool UMVE_API_Helper::ParseError(const FString& ResponseBody, FString& OutErrorCode, FString& OutErrorMessage)
{
    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

    if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
    {
        if(JsonResponse->HasField(TEXT("error")) && !JsonResponse->GetStringField(TEXT("error")).IsEmpty())
        {
            JsonResponse->TryGetStringField(TEXT("error"), OutErrorCode);
            JsonResponse->TryGetStringField(TEXT("message"), OutErrorMessage);
            return true;
        }
    }
    
    // If parsing fails or message is not found, return the raw body as message
    OutErrorCode = TEXT("UNKNOWN");
    if (!ResponseBody.IsEmpty())
    {
        OutErrorMessage = ResponseBody;
    }
    else
    {
        OutErrorMessage = TEXT("An unknown error occurred.");
    }
    return false;
}

// 서버 헬스 체크
void UMVE_API_Helper::LoginHealthCheck(const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/health/login"), *LoginServerURL);
    FMVE_HTTP_Client::SendGetRequest(URL, "", HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::ResourceHealthCheck(const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/health/resource"), *ResourceServerURL);
    FMVE_HTTP_Client::SendGetRequest(URL, "", HANDLE_GENERIC_RESPONSE(OnResult));
}

// 로그인 서버 API
void UMVE_API_Helper::CheckEmail(const FString& Email, const FOnCheckEmailComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/check-email"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), Email);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, "", HANDLE_RESPONSE_STRUCT(FCheckEmailResponseData, OnResult));
}

void UMVE_API_Helper::SendVerificationCode(const FString& Email, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/send-verification"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), Email);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, "", HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::TryConfirmVerifyCode(const FString& Email, const FString& Code, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/verify-code"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), Email);
    JsonObject->SetStringField(TEXT("code"), Code);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, "", HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::SignUp(const FString& Email, const FString& Password, const FString& Code, const FOnSignUpComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/signup"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), Email);
    JsonObject->SetStringField(TEXT("password"), Password);
    JsonObject->SetStringField(TEXT("code"), Code);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, "", HANDLE_RESPONSE_STRUCT(FSignUpResponseData, OnResult));
}

void UMVE_API_Helper::Login(const FString& Email, const FString& Password, const FMVEOnLoginComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/login"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), Email);
    JsonObject->SetStringField(TEXT("password"), Password);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // auto HandleResponseLambda = [OnResult](const bool bSuccess, const FString& ResponseBody)
    // {
    //     PRINTLOG(TEXT("Response: %s"), *ResponseBody);
    //     if (bSuccess)
    //     {
    //         FLoginResponseData ParsedData;
    //         if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseBody, &ParsedData, 0, 0) && ParsedData.Success)
    //         {
    //             GlobalAuthToken = ParsedData.Token;
    //             PRINTLOG(TEXT("로그인 성공. 인증 토큰 : %s"), *GlobalAuthToken);
    //             OnResult.ExecuteIfBound(true, ParsedData, TEXT(""));
    //         }
    //         else
    //         {
    //             FString ErrorCode, ErrorMessage;
    //             UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
    //             OnResult.ExecuteIfBound(false, FLoginResponseData(), ErrorCode);
    //         }
    //     }
    //     else
    //     {
    //         FString ErrorCode, ErrorMessage;
    //         UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
    //         OnResult.ExecuteIfBound(false, FLoginResponseData(), ErrorCode);
    //     }
    // };
    
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, "", HANDLE_RESPONSE_STRUCT(FLoginResponseData, OnResult));
}

void UMVE_API_Helper::Logout(const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/logout"), *LoginServerURL);
    
    auto HandleResponseLambda = [OnResult](bool bSuccess, const FString& ResponseBody)
    {
        PRINTLOG(TEXT("Response: %s"), *ResponseBody);
        if (bSuccess)
        {
            GlobalAuthToken.Empty();
            OnResult.ExecuteIfBound(true, TEXT("Logged out successfully."));
        }
        else
        {
            FString ErrorCode, ErrorMessage;
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
            OnResult.ExecuteIfBound(false, ErrorCode);
        }
    };
    
    FMVE_HTTP_Client::SendPostRequest(URL, "", GlobalAuthToken, FOnHttpResponse::CreateLambda(HandleResponseLambda));
}

void UMVE_API_Helper::Withdraw(const FString& Password, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/withdraw"), *LoginServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("password"), Password);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    auto HandleResponseLambda = [OnResult](bool bSuccess, const FString& ResponseBody)
    {
        PRINTLOG(TEXT("Response: %s"), *ResponseBody);
        if (bSuccess)
        {
            GlobalAuthToken.Empty();
            OnResult.ExecuteIfBound(true, TEXT("Withdrawal successful."));
        }
        else
        {
            FString ErrorCode, ErrorMessage;
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
            OnResult.ExecuteIfBound(false, ErrorCode);
        }
    };

    FMVE_HTTP_Client::SendDeleteRequest(URL, JsonBody, GlobalAuthToken, FOnHttpResponse::CreateLambda(HandleResponseLambda));
}

void UMVE_API_Helper::GetProfile(const FOnGetProfileComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/auth/profile"), *LoginServerURL);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FProfileResponseData, OnResult));
}

// 리소스 서버 관련 API
void UMVE_API_Helper::SaveAccessoryPreset(const FString& PresetName, const TArray<TSharedPtr<FJsonValue>>& Accessories, const FString& Description, const bool bIsPublic, const FOnSavePresetComplete& OnResult)

{

    const FString URL = FString::Printf(TEXT("%s/api/presets/save"), *ResourceServerURL);

    

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    JsonObject->SetStringField(TEXT("presetName"), PresetName);

    JsonObject->SetArrayField(TEXT("accessories"), Accessories);

    if (!Description.IsEmpty())

    {

        JsonObject->SetStringField(TEXT("description"), Description);

    }

    JsonObject->SetBoolField(TEXT("isPublic"), bIsPublic);

    

    FString JsonBody;

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);

    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);



    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FSavePresetResponseData, OnResult));

}

void UMVE_API_Helper::GetPresetList(const bool bIncludePublic, const FOnGetPresetListComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/presets/list?includePublic=%s"), *ResourceServerURL, bIncludePublic ? TEXT("true") : TEXT("false"));
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetPresetListResponseData, OnResult));
}

void UMVE_API_Helper::GetPresetDetail(const int32 PresetId, const FOnGetPresetDetailComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/presets/%d"), *ResourceServerURL, PresetId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetPresetDetailResponseData, OnResult));
}

void UMVE_API_Helper::UpdatePreset(const int32 PresetId, const FString& PresetName, const TArray<TSharedPtr<FJsonValue>>& Accessories, const FString& Description, const bool bIsPublic, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/presets/%d"), *ResourceServerURL, PresetId);
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    if(!PresetName.IsEmpty()) JsonObject->SetStringField(TEXT("presetName"), PresetName);
    if(Accessories.Num() > 0) JsonObject->SetArrayField(TEXT("accessories"), Accessories);
    if(!Description.IsEmpty()) JsonObject->SetStringField(TEXT("description"), Description);
    JsonObject->SetBoolField(TEXT("isPublic"), bIsPublic);
    
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    FMVE_HTTP_Client::SendPutRequest(URL, JsonBody, GlobalAuthToken, HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::DeletePreset(const int32 PresetId, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/presets/%d"), *ResourceServerURL, PresetId);
    FMVE_HTTP_Client::SendDeleteRequest(URL, GlobalAuthToken, HANDLE_GENERIC_RESPONSE(OnResult));
}

// 음원 관련 API
void UMVE_API_Helper::GetAudioList(const FOnGetAudioListComplete& OnResult)

{

    const FString URL = FString::Printf(TEXT("%s/api/audio/list"), *ResourceServerURL);

    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FAudioListResponseData, OnResult));

}

void UMVE_API_Helper::StreamAudio(const int32 AudioId, const FOnStreamAudioComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/audio/stream/%d"), *ResourceServerURL, AudioId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FStreamAudioResponseData, OnResult));
}

void UMVE_API_Helper::UploadAudio(const FString& FilePath, const FString& Title, const FString& Artist, const float Duration, const FOnUploadAudioComplete& OnResult)

{

    const FString URL = FString::Printf(TEXT("%s/api/audio/upload"), *ResourceServerURL);

    

    TMap<FString, FString> FormFields;

    FormFields.Add(TEXT("title"), Title);

    if (!Artist.IsEmpty())

    {

        FormFields.Add(TEXT("artist"), Artist);

    }

    if (Duration > 0)

    {

        FormFields.Add(TEXT("duration"), FString::SanitizeFloat(Duration));

    }



    TArray<uint8> FileData;

    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))

    {

        OnResult.ExecuteIfBound(false, FUploadAudioResponseData(), TEXT("FILE_READ_ERROR"));

        return;

    }

    

    const FString FileName = FPaths::GetCleanFilename(FilePath);

    FMVE_HTTP_Client::SendMultipartRequest(URL, TEXT("audio"), FileData, FileName, FormFields, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FUploadAudioResponseData, OnResult));

}

void UMVE_API_Helper::SearchAudio(const FString& Query, const FOnSearchAudioComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/audio/search/%s"), *ResourceServerURL, *Query);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FAudioListResponseData, OnResult));
}

void UMVE_API_Helper::GetAudioDetail(const int32 AudioId, const FOnGetAudioDetailComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/audio/%d"), *ResourceServerURL, AudioId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetAudioDetailResponseData, OnResult));
}

void UMVE_API_Helper::DownloadAudioFile(int32 AudioId, const FString& SavePath, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/audio/file/%d"), *ResourceServerURL, AudioId);

    auto HandleDownloadResponse = FOnHttpDownloadResult::CreateLambda([SavePath, AudioId, OnResult](bool bSuccess, const TArray<uint8>& FileData, const FString& ErrorMessage)
    {
        PRINTLOG(TEXT("Download Response: Success=%d, ErrorMessage=%s"), bSuccess, *ErrorMessage);

        if (bSuccess && FileData.Num() > 0)
        {
            FString FinalPath = SavePath;
            if (FinalPath.IsEmpty())
            {
                FinalPath = FPaths::ProjectSavedDir() / TEXT("DownloadedAudio");
                IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
                if (!PlatformFile.DirectoryExists(*FinalPath))
                {
                    PlatformFile.CreateDirectoryTree(*FinalPath);
                }
                // We don't know the extension from this endpoint, so we can't add one.
                // The caller should provide a full path if they know the file type.
                FinalPath /= FString::Printf(TEXT("Audio_%d"), AudioId);
            }

            if (FFileHelper::SaveArrayToFile(FileData, *FinalPath))
            {
                OnResult.ExecuteIfBound(true, FinalPath);
            }
            else
            {
                OnResult.ExecuteIfBound(false, TEXT("FILE_SAVE_ERROR"));
            }
        }
        else
        {
            FString ErrorCode, _;
            UMVE_API_Helper::ParseError(ErrorMessage, ErrorCode, _);
            OnResult.ExecuteIfBound(false, ErrorCode);
        }
    });

    FMVE_HTTP_Client::DownloadFile(URL, GlobalAuthToken, HandleDownloadResponse);
}

// 콘서트 관련 API
void UMVE_API_Helper::CreateConcert(const FString& ConcertName, const TArray<FConcertSong>& Songs, const TArray<FAccessory>& Accessories, const int32 MaxAudience, const FOnCreateConcertComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/create"), *ResourceServerURL);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("concertName"), ConcertName);
    JsonObject->SetNumberField(TEXT("maxAudience"), MaxAudience);

    // Serialize Songs
    TArray<TSharedPtr<FJsonValue>> SongsArray;
    for (const auto& Song : Songs)
    {
        TSharedPtr<FJsonObject> SongObject = FJsonObjectConverter::UStructToJsonObject(Song);
        SongsArray.Add(MakeShareable(new FJsonValueObject(SongObject)));
    }
    JsonObject->SetArrayField(TEXT("songs"), SongsArray);

    // Serialize Accessories
    TArray<TSharedPtr<FJsonValue>> AccessoriesArray;
    for (const auto& Accessory : Accessories)
    {
        TSharedPtr<FJsonObject> AccessoryObject = FJsonObjectConverter::UStructToJsonObject(Accessory);
        AccessoriesArray.Add(MakeShareable(new FJsonValueObject(AccessoryObject)));
    }
    JsonObject->SetArrayField(TEXT("accessories"), AccessoriesArray);
    
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    auto HandleResponseLambda = [OnResult](const bool bSuccess, const FString& ResponseBody)
    {
        PRINTLOG(TEXT("CreateConcert Response: %s"), *ResponseBody);
        if (bSuccess)
        {
            FConcertCreationData ParsedData;
            if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseBody, &ParsedData, 0, 0))
            {
                if(ParsedData.Success)
                {
                    PRINTLOG(TEXT("%s"), *ParsedData.RoomId);
                    OnResult.ExecuteIfBound(true, ParsedData, TEXT(""));
                }
                else
                {
                    OnResult.ExecuteIfBound(false, FConcertCreationData(), TEXT("CREATE_FAILED"));
                }
            }
            else
            {
                OnResult.ExecuteIfBound(false, FConcertCreationData(), TEXT("JSON_PARSE_ERROR"));
            }
        }
        else
        {
            FString ErrorCode, ErrorMessage;
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
            OnResult.ExecuteIfBound(false, FConcertCreationData(), ErrorCode);
        }
    };
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, FOnHttpResponse::CreateLambda(HandleResponseLambda));
}

void UMVE_API_Helper::GetConcertInfo(const FString& RoomId, const FOnGetConcertInfoComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/%s/info"), *ResourceServerURL, *RoomId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetConcertInfoResponseData, OnResult));
}

void UMVE_API_Helper::GetConcertList(const FOnGetConcertListComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/list"), *ResourceServerURL);
    auto HandleResponseLambda = [OnResult](const bool bSuccess, const FString& ResponseBody)
    {
        PRINTLOG(TEXT("GetConcertList Response: %s"), *ResponseBody);
        if (bSuccess)
        {
            FGetConcertListData ParsedData;
            if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseBody, &ParsedData, 0, 0))
            {
                if(ParsedData.Success)
                {
                    OnResult.ExecuteIfBound(true, ParsedData, TEXT(""));
                }
                else
                {
                    OnResult.ExecuteIfBound(false, FGetConcertListData(), TEXT("GET_CONCERT_LIST_FAILED"));
                }
            }
            else
            {
                OnResult.ExecuteIfBound(false, FGetConcertListData(), TEXT("JSON_PARSE_ERROR"));
            }
        }
        else
        {
            FString ErrorCode, ErrorMessage;
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
            OnResult.ExecuteIfBound(false, FGetConcertListData(), ErrorCode);
        }
    };
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, FOnHttpResponse::CreateLambda(HandleResponseLambda));
}

void UMVE_API_Helper::JoinConcert(const FString& RoomId, const int32 ClientId, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/%s/join"), *ResourceServerURL, *RoomId);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("clientId"), ClientId);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::LeaveConcert(const FString& RoomId, const int32 ClientId, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/%s/leave"), *ResourceServerURL, *RoomId);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("clientId"), ClientId);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, HANDLE_GENERIC_RESPONSE(OnResult));
}

void UMVE_API_Helper::RegisterListenServer(const FString& RoomId, const FString& LocalIP, const int32 Port, const FString& PublicIP, const int32 PublicPort, const FOnRegisterListenServerComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/%s/listen-server"), *ResourceServerURL, *RoomId);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("localIP"), LocalIP);
    JsonObject->SetNumberField(TEXT("port"), Port);
    if (PublicIP.IsEmpty() == false)
    {
        JsonObject->SetStringField(TEXT("publicIP"), PublicIP);
    }
    if (PublicPort > 0)
    {
        JsonObject->SetNumberField(TEXT("publicPort"), PublicPort);
    }
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    auto HandleResponseLambda = [OnResult](bool bSuccess, const FString& ResponseBody)
    {
        PRINTLOG(TEXT("RegisterListenServer Response: %s"), *ResponseBody);
        if (bSuccess)
        {
            FRegisterListenServerData ParsedData;
            if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseBody, &ParsedData, 0, 0))
            {
                if(ParsedData.Success)
                {
                    OnResult.ExecuteIfBound(true, ParsedData, TEXT(""));
                }
                else
                {
                    OnResult.ExecuteIfBound(false, FRegisterListenServerData(), TEXT("REGISTER_LISTEN_SERVER_FAILED"));
                }
            }
            else
            {
                OnResult.ExecuteIfBound(false, FRegisterListenServerData(), TEXT("JSON_PARSE_ERROR"));
            }
        }
        else
        {
            FString ErrorCode, ErrorMessage;
            UMVE_API_Helper::ParseError(ResponseBody, ErrorCode, ErrorMessage);
            OnResult.ExecuteIfBound(false, FRegisterListenServerData(), ErrorCode);
        }
    };
    
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, FOnHttpResponse::CreateLambda(HandleResponseLambda));
}

void UMVE_API_Helper::ToggleConcertOpen(const FString& RoomId, const bool bIsOpen, const FOnGenericApiComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/concert/%s/toggle-open"), *ResourceServerURL, *RoomId);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetBoolField(TEXT("isOpen"), bIsOpen);
    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    FMVE_HTTP_Client::SendPostRequest(URL, JsonBody, GlobalAuthToken, HANDLE_GENERIC_RESPONSE(OnResult));
}

// AI 모델 생성 및 관리 API
void UMVE_API_Helper::GetModelList(const FString& RoomId, const FOnGetModelListComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/list"), *ResourceServerURL);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FModelListResponseData, OnResult));
}

void UMVE_API_Helper::GenerateModel(const FString& Prompt, const FString& ImagePath, const FOnGenerateModelComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/generate"), *ResourceServerURL);
    TMap<FString, FString> FormFields;
    FormFields.Add(TEXT("prompt"), Prompt);
    TArray<uint8> FileData;
    FString FileName;
    if (ImagePath.IsEmpty() == false)
    {
        if (FFileHelper::LoadFileToArray(FileData, *ImagePath) == false)
        {
            OnResult.ExecuteIfBound(false, FGenerateModelResponseData(), TEXT("FILE_READ_ERROR"));
            return;
        }
        FileName = FPaths::GetCleanFilename(ImagePath);
        //FileName = FPaths::GetCleanFilename(ImagePath) + FDateTime::Now().ToString();
    }
    FMVE_HTTP_Client::SendMultipartRequest(URL, TEXT("image"), FileData, FileName, FormFields, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGenerateModelResponseData, OnResult));
}

void UMVE_API_Helper::GetModelGenerationStatus(const FString& JobId, const FOnGetJobStatusComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/jobs/%s"), *ResourceServerURL, *JobId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetJobStatusResponseData, OnResult));
}

void UMVE_API_Helper::UploadModel(const FString& ModelPath, const FString& ThumbnailPath, const FString& ModelName, const FOnUploadModelComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/upload"), *ResourceServerURL);
    TMap<FString, FString> FormFields;
    if (!ModelName.IsEmpty())
    {
        FormFields.Add(TEXT("model_name"), ModelName);
    }
    TArray<uint8> ModelData;
    if (!FFileHelper::LoadFileToArray(ModelData, *ModelPath))
    {
        OnResult.ExecuteIfBound(false, FUploadModelResponseData(), TEXT("FILE_READ_ERROR"));
        return;
    }
    // 현재 HTTP 클라이언트는 단일 파일만 지원하므로 모델 파일만 전송합니다.
    // 만약 다중 파일 업로드가 필요하면 MVE_HTTP_Client 수정이 필요합니다.
    const FString FileName = FPaths::GetCleanFilename(ModelPath);
    FMVE_HTTP_Client::SendMultipartRequest(URL, TEXT("model"), ModelData, FileName, FormFields, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FUploadModelResponseData, OnResult));
}

// 경고 : 이거 안씀
// void UMVE_API_Helper::DownloadModel(int32 ModelId, const FString& SavePath, const FOnGenericApiComplete& OnResult)
// {
//     const FString URL = FString::Printf(TEXT("%s/api/models/%d/download"), *ResourceServerURL, ModelId);
//     auto HandleDownloadResponse = FOnHttpDownloadResult::CreateLambda([SavePath, ModelId, OnResult](bool bSuccess, const TArray<uint8>& FileData, const FString& ErrorMessage)
//
//     {
//
//         PRINTLOG(TEXT("Download Response: Success=%d, ErrorMessage=%s"), bSuccess, *ErrorMessage);
//
//         if (bSuccess && FileData.Num() > 0)
//
//         {
//
//             FString FinalPath = SavePath;
//
//             if (FinalPath.IsEmpty())
//
//             {
//
//                 FinalPath = FPaths::ProjectSavedDir() / TEXT("DownloadedModels");
//
//                 IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
//
//                 if (!PlatformFile.DirectoryExists(*FinalPath))
//
//                 {
//
//                     PlatformFile.CreateDirectoryTree(*FinalPath);
//
//                 }
//
//                 FinalPath /= FString::Printf(TEXT("Model_%d.glb"), ModelId);
//
//             }
//
//
//
//             if (FFileHelper::SaveArrayToFile(FileData, *FinalPath))
//
//             {
//
//                 OnResult.ExecuteIfBound(true, FinalPath);
//
//             }
//
//             else
//
//             {
//
//                 OnResult.ExecuteIfBound(false, TEXT("FILE_SAVE_ERROR"));
//
//             }
//
//         }
//
//         else
//
//         {
//             FString ErrorCode, _;
//             UMVE_API_Helper::ParseError(ErrorMessage, ErrorCode, _);
//             OnResult.ExecuteIfBound(false, ErrorCode);
//         }
//
//     });
//     FMVE_HTTP_Client::DownloadFile(URL, GlobalAuthToken, HandleDownloadResponse);
// }

// 이거를 써요
void UMVE_API_Helper::GetModelDownloadUrl(const int32 ModelId, const FOnGetModelDownloadUrlComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/%d/download-url"), *ResourceServerURL, ModelId);
    FMVE_HTTP_Client::SendGetRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FGetModelDownloadUrlResponseData, OnResult));
}

void UMVE_API_Helper::DeleteModel(const int32 ModelId, const FOnDeleteModelComplete& OnResult)
{
    const FString URL = FString::Printf(TEXT("%s/api/models/%d"), *ResourceServerURL, ModelId);
    FMVE_HTTP_Client::SendDeleteRequest(URL, GlobalAuthToken, HANDLE_RESPONSE_STRUCT(FDeleteModelResponseData, OnResult));
}

// 블루프린트 래핑

bool UMVE_API_Helper::GetHostLocalIPAndPortBP(const UObject* WorldContextObject, FString& OutLocalIP, int32& OutPort)
{
    return GetHostLocalIPAndPort(WorldContextObject, OutLocalIP, OutPort);
}

void UMVE_API_Helper::LoginHealthCheckBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    LoginHealthCheck(OnResult);
}

void UMVE_API_Helper::ResourceHealthCheckBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    ResourceHealthCheck(OnResult);
}

void UMVE_API_Helper::CheckEmailBP(UObject* WorldContextObject, const FString& Email, const FOnCheckEmailCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnCheckEmailComplete, OnResultBP);
    CheckEmail(Email, OnResult);
}

void UMVE_API_Helper::SendVerificationCodeBP(UObject* WorldContextObject, const FString& Email, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    SendVerificationCode(Email, OnResult);
}

void UMVE_API_Helper::TryConfirmVerifyCodeBP(UObject* WorldContextObject, const FString& Email, const FString& Code, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    TryConfirmVerifyCode(Email, Code, OnResult);
}

void UMVE_API_Helper::SignUpBP(UObject* WorldContextObject, const FString& Email, const FString& Password, const FString& Code, const FOnSignUpCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnSignUpComplete, OnResultBP);
    SignUp(Email, Password, Code, OnResult);
}

void UMVE_API_Helper::LoginBP(UObject* WorldContextObject, const FString& Email, const FString& Password, const FOnLoginCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(MVEOnLoginComplete, OnResultBP);
    Login(Email, Password, OnResult);
}

void UMVE_API_Helper::LogoutBP(UObject* WorldContextObject, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    Logout(OnResult);
}

void UMVE_API_Helper::WithdrawBP(UObject* WorldContextObject, const FString& Password, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    Withdraw(Password, OnResult);
}

void UMVE_API_Helper::GetProfileBP(UObject* WorldContextObject, const FOnGetProfileCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetProfileComplete, OnResultBP);
    GetProfile(OnResult);
}


void UMVE_API_Helper::SaveAccessoryPresetBP(UObject* WorldContextObject, const FString& PresetName, const FString& AccessoriesJson, const FString& Description, const bool bIsPublic, const FOnSavePresetCompleteBP& OnResultBP)
{
    TArray<TSharedPtr<FJsonValue>> Accessories;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(AccessoriesJson);
    FJsonSerializer::Deserialize(Reader, Accessories);
    
    WRAP_DELEGATE(OnSavePresetComplete, OnResultBP);
    SaveAccessoryPreset(PresetName, Accessories, Description, bIsPublic, OnResult);
}

void UMVE_API_Helper::GetPresetListBP(UObject* WorldContextObject, const bool bIncludePublic, const FOnGetPresetListCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetPresetListComplete, OnResultBP);
    GetPresetList(bIncludePublic, OnResult);
}

void UMVE_API_Helper::GetPresetDetailBP(UObject* WorldContextObject, const int32 PresetId, const FOnGetPresetDetailCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetPresetDetailComplete, OnResultBP);
    GetPresetDetail(PresetId, OnResult);
}

void UMVE_API_Helper::UpdatePresetBP(UObject* WorldContextObject, const int32 PresetId, const FString& PresetName, const FString& AccessoriesJson, const FString& Description, const bool bIsPublic, const FOnGenericApiCompleteBP& OnResultBP, FLatentActionInfo LatentInfo)
{
    TArray<TSharedPtr<FJsonValue>> Accessories;
    if(!AccessoriesJson.IsEmpty())
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(AccessoriesJson);
        FJsonSerializer::Deserialize(Reader, Accessories);
    }
    
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    UpdatePreset(PresetId, PresetName, Accessories, Description, bIsPublic, OnResult);
}

void UMVE_API_Helper::DeletePresetBP(UObject* WorldContextObject, const int32 PresetId, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    DeletePreset(PresetId, OnResult);
}


void UMVE_API_Helper::GetAudioListBP(UObject* WorldContextObject, const FOnGetAudioListCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetAudioListComplete, OnResultBP);
    GetAudioList(OnResult);
}

void UMVE_API_Helper::StreamAudioBP(UObject* WorldContextObject, const int32 AudioId, const FOnStreamAudioCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnStreamAudioComplete, OnResultBP);
    StreamAudio(AudioId, OnResult);
}

void UMVE_API_Helper::UploadAudioBP(UObject* WorldContextObject, const FString& FilePath, const FString& Title, const FString& Artist, const float Duration, const FOnUploadAudioCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnUploadAudioComplete, OnResultBP);
    UploadAudio(FilePath, Title, Artist, Duration, OnResult);
}

void UMVE_API_Helper::SearchAudioBP(UObject* WorldContextObject, const FString& Query, const FOnSearchAudioCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnSearchAudioComplete, OnResultBP);
    SearchAudio(Query, OnResult);
}

void UMVE_API_Helper::GetAudioDetailBP(UObject* WorldContextObject, const int32 AudioId, const FOnGetAudioDetailCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetAudioDetailComplete, OnResultBP);
    GetAudioDetail(AudioId, OnResult);
}

void UMVE_API_Helper::DownloadAudioFileBP(UObject* WorldContextObject, const int32 AudioId, const FString& SavePath, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    DownloadAudioFile(AudioId, SavePath, OnResult);
}


void UMVE_API_Helper::CreateConcertBP(UObject* WorldContextObject, const FString& ConcertName, const TArray<FConcertSong>& Songs, const TArray<FAccessory>& Accessories, const int32 MaxAudience, const FOnCreateConcertCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnCreateConcertComplete, OnResultBP);
    CreateConcert(ConcertName, Songs, Accessories, MaxAudience, OnResult);
}

void UMVE_API_Helper::GetConcertInfoBP(UObject* WorldContextObject, const FString& RoomId, const FOnGetConcertInfoCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetConcertInfoComplete, OnResultBP);
    GetConcertInfo(RoomId, OnResult);
}

void UMVE_API_Helper::GetConcertListBP(UObject* WorldContextObject, const FOnGetConcertListCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetConcertListComplete, OnResultBP);
    GetConcertList(OnResult);
}

void UMVE_API_Helper::JoinConcertBP(UObject* WorldContextObject, const FString& RoomId, const int32 ClientId, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    JoinConcert(RoomId, ClientId, OnResult);
}

void UMVE_API_Helper::LeaveConcertBP(UObject* WorldContextObject, const FString& RoomId, const int32 ClientId, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    LeaveConcert(RoomId, ClientId, OnResult);
}

void UMVE_API_Helper::RegisterListenServerBP(UObject* WorldContextObject, const FString& RoomId, const FString& LocalIP, const int32 Port, const FString& PublicIP, const int32 PublicPort, const FOnRegisterListenServerCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnRegisterListenServerComplete, OnResultBP);
    RegisterListenServer(RoomId, LocalIP, Port, PublicIP, PublicPort, OnResult);
}

void UMVE_API_Helper::ToggleConcertOpenBP(UObject* WorldContextObject, const FString& RoomId, const bool bIsOpen, const FOnGenericApiCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
    ToggleConcertOpen(RoomId, bIsOpen, OnResult);
}


void UMVE_API_Helper::GetModelListBP(UObject* WorldContextObject, const FString& RoomId, const FOnGetModelListCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetModelListComplete, OnResultBP);
    GetModelList(RoomId, OnResult);
}

void UMVE_API_Helper::GenerateModelBP(UObject* WorldContextObject, const FString& Prompt, const FString& ImagePath, const FOnGenerateModelCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGenerateModelComplete, OnResultBP);
    GenerateModel(Prompt, ImagePath, OnResult);
}

void UMVE_API_Helper::GetModelGenerationStatusBP(UObject* WorldContextObject, const FString& JobId, const FOnGetJobStatusCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetJobStatusComplete, OnResultBP);
    GetModelGenerationStatus(JobId, OnResult);
}

void UMVE_API_Helper::UploadModelBP(UObject* WorldContextObject, const FString& ModelPath, const FString& ThumbnailPath, const FString& ModelName, const FOnUploadModelCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnUploadModelComplete, OnResultBP);
    UploadModel(ModelPath, ThumbnailPath, ModelName, OnResult);
}

// void UMVE_API_Helper::DownloadModelBP(UObject* WorldContextObject, const int32 ModelId, const FString& SavePath, const FOnGenericApiCompleteBP& OnResultBP)
// {
//     WRAP_DELEGATE(OnGenericApiComplete, OnResultBP);
//     DownloadModel(ModelId, SavePath, OnResult);
// }

void UMVE_API_Helper::GetModelDownloadUrlBP(UObject* WorldContextObject, const int32 ModelId, const FOnGetModelDownloadUrlCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnGetModelDownloadUrlComplete, OnResultBP);
    GetModelDownloadUrl(ModelId, OnResult);
}

void UMVE_API_Helper::DeleteModelBP(UObject* WorldContextObject, const int32 ModelId, const FOnDeleteModelCompleteBP& OnResultBP)
{
    WRAP_DELEGATE(OnDeleteModelComplete, OnResultBP);
    DeleteModel(ModelId, OnResult);
}
