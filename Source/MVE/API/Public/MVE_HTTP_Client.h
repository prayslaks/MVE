#pragma once

#include "CoreMinimal.h"
#include "Http.h"

DECLARE_DELEGATE_TwoParams(FOnHttpResponse, bool /*bSuccess*/, const FString& /*ResponseBody*/);
DECLARE_DELEGATE_ThreeParams(FOnHttpDownloadResult, bool /*bSuccess*/, const TArray<uint8>& /*FileData*/, const FString& /*ErrorMessage*/);

class MVE_API FMVE_HTTP_Client
{
public:
    // GET 요청
    static void SendGetRequest(const FString& URL, const FString& AuthToken, FOnHttpResponse OnComplete);

    // POST 요청 (JSON)
    static void SendPostRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete);

    // PUT 요청 (JSON)
    static void SendPutRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete);

    // DELETE 요청
    static void SendDeleteRequest(const FString& URL, const FString& AuthToken, FOnHttpResponse OnComplete);
    // DELETE 요청 (JSON Body)
    static void SendDeleteRequest(const FString& URL, const FString& JsonBody, const FString& AuthToken, FOnHttpResponse OnComplete);

    // POST 요청 (Multipart Form Data - 파일 업로드)
    static void SendMultipartRequest(const FString& URL, const FString& FileFieldName, const TArray<uint8>& FileData, const FString& FileName,
                                      const TMap<FString, FString>& FormFields, const FString& AuthToken, FOnHttpResponse OnComplete);

    // GET 요청 (파일 다운로드)
    static void DownloadFile(const FString& URL, const FString& AuthToken, FOnHttpDownloadResult OnComplete);

private:
    static void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpResponse Callback);
    static void OnDownloadResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpDownloadResult Callback);
};
