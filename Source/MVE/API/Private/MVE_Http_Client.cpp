#include "API/Public/MVE_HTTP_Client.h"
#include "HttpModule.h"
#include "MVE.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMVE_HTTP_Client::SendGetRequest(const FString& URL, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", "application/json");

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::SendPostRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonBody);

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::SendPutRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("PUT");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonBody);

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::SendDeleteRequest(const FString& URL, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("DELETE");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", "application/json");

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::SendDeleteRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("DELETE");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonBody);

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::SendMultipartRequest(const FString& URL, const FString& FileFieldName, const TArray<uint8>& FileData, const FString& FileName,
    const TMap<FString, FString>& FormFields, const FString& AuthToken, FOnHttpResponse OnComplete)
{
    const FString Boundary = FString::Printf(TEXT("----UnrealBoundary%d"), FMath::RandRange(100000, 999999));

    TArray<uint8> CombinedContent;

    // Form 필드 추가
    for (const TPair<FString, FString>& Field : FormFields)
    {
        FString FieldHeader = FString::Printf(
            TEXT("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n"),
            *Boundary, *Field.Key, *Field.Value
        );
        FTCHARToUTF8 FieldConverter(*FieldHeader);
        CombinedContent.Append((uint8*)FieldConverter.Get(), FieldConverter.Length());
    }

    // 파일 필드 추가 (파일 데이터가 있는 경우)
    if(FileData.Num() > 0)
    {
        // 파일 확장자에 따라 적절한 Content-Type 설정
        FString FileExtension = FPaths::GetExtension(FileName).ToLower();
        FString ContentType = TEXT("application/octet-stream");  // 기본값

        if (FileExtension == TEXT("png"))
        {
            ContentType = TEXT("image/png");
        }
        else if (FileExtension == TEXT("jpg") || FileExtension == TEXT("jpeg"))
        {
            ContentType = TEXT("image/jpeg");
        }
        else if (FileExtension == TEXT("gif"))
        {
            ContentType = TEXT("image/gif");
        }
        else if (FileExtension == TEXT("webp"))
        {
            ContentType = TEXT("image/webp");
        }
        else if (FileExtension == TEXT("glb"))
        {
            ContentType = TEXT("model/gltf-binary");
        }
        else if (FileExtension == TEXT("gltf"))
        {
            ContentType = TEXT("model/gltf+json");
        }

        PRINTLOG(TEXT("📤 Multipart Upload: File=%s, Size=%d bytes, ContentType=%s"),
            *FileName, FileData.Num(), *ContentType);

        FString FileHeader = FString::Printf(
            TEXT("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n"),
            *Boundary, *FileFieldName, *FileName, *ContentType
        );
        FTCHARToUTF8 FileConverter(*FileHeader);
        CombinedContent.Append((uint8*)FileConverter.Get(), FileConverter.Length());
        CombinedContent.Append(FileData);
        CombinedContent.Append((uint8*)TCHAR_TO_UTF8(TEXT("\r\n")), 2);
    }

    // 종료 Boundary
    FString EndBoundary = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
    FTCHARToUTF8 EndConverter(*EndBoundary);
    CombinedContent.Append((uint8*)EndConverter.Get(), EndConverter.Length());

    PRINTLOG(TEXT("📤 Total request size: %d bytes"), CombinedContent.Num());

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL(URL);
    Request->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
    Request->SetContent(CombinedContent);

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }

    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::DownloadFile(const FString& URL, const FString& AuthToken, FOnHttpDownloadResult OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL(URL);

    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }

    Request->OnProcessRequestComplete().BindStatic(&FMVE_HTTP_Client::OnDownloadResponseReceived, OnComplete);
    Request->ProcessRequest();
}

void FMVE_HTTP_Client::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, const bool bWasSuccessful, FOnHttpResponse Callback)
{
    if (bWasSuccessful && Response.IsValid())
    {
        const int32 StatusCode = Response->GetResponseCode();
        const FString ResponseBody = Response->GetContentAsString();
        
        const bool bSuccess = (StatusCode >= 200 && StatusCode < 500);
        Callback.ExecuteIfBound(bSuccess, ResponseBody);
    }
    else
    {
        Callback.ExecuteIfBound(false, TEXT("{\"success\": false, \"error\": \"NETWORK_ERROR\", \"message\": \"Network request failed\"}"));
    }
}

void FMVE_HTTP_Client::OnDownloadResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpDownloadResult Callback)
{
    if (bWasSuccessful && Response.IsValid())
    {
        const int32 StatusCode = Response->GetResponseCode();
        if (StatusCode >= 200 && StatusCode < 500)
        {
            Callback.ExecuteIfBound(true, Response->GetContent(), TEXT(""));
        }
        else
        {
            FString ErrorMessage = FString::Printf(TEXT("HTTP Error: %d. Body: %s"), StatusCode, *Response->GetContentAsString());
            Callback.ExecuteIfBound(false, TArray<uint8>(), ErrorMessage);
        }
    }
    else
    {
        Callback.ExecuteIfBound(false, TArray<uint8>(), TEXT("Network request failed."));
    }
}