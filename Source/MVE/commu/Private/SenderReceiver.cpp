// Source/MVE/SenderReceiver.cpp

#include "../Public/SenderReceiver.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "MVE.h"
#include "Engine/StaticMesh.h"

// ============================================================================
//                          정적 멤버 초기화
// ============================================================================

const TMap<EAssetType, FString> USenderReceiver::AssetTypeExtensions = {
    { EAssetType::MESH,    TEXT("glb") },
    { EAssetType::IMAGE,   TEXT("png") },
    { EAssetType::AUDIO,   TEXT("wav") },
    { EAssetType::VIDEO,   TEXT("mp4") },
    { EAssetType::GENERIC, TEXT("dat") }
};

// ============================================================================
//                          유틸리티 함수
// ============================================================================

FString USenderReceiver::GetFileExtension(EAssetType AssetType)
{
    if (const FString* Ext = AssetTypeExtensions.Find(AssetType))
    {
        return *Ext;
    }
    return TEXT("dat");
}

FString USenderReceiver::GetSaveDirectory()
{
    // 저장 경로: {프로젝트}/Saved/GenAIAssets/
    FString SaveDir = FPaths::ProjectSavedDir() / LocalSaveFolder;
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDir))
    {
        if (PlatformFile.CreateDirectoryTree(*SaveDir))
        {
            UE_LOG(LogMVE, Log, TEXT("[Utility] 저장 디렉토리 생성: %s"), *SaveDir);
        }
        else
        {
            UE_LOG(LogMVE, Error, TEXT("[Utility] 디렉토리 생성 실패: %s"), *SaveDir);
        }
    }
    return SaveDir;
}

FDateTime USenderReceiver::ParseDateString(const FString& DateStr)
{
    // AI 서버 날짜 형식: "YYYY-MM-DD HH:MM:SS"
    FDateTime Result = FDateTime::Now();
    
    if (DateStr.IsEmpty())
    {
        return Result;
    }

    TArray<FString> Parts;
    DateStr.ParseIntoArray(Parts, TEXT(" "));
    
    if (Parts.Num() >= 2)
    {
        TArray<FString> DateParts, TimeParts;
        Parts[0].ParseIntoArray(DateParts, TEXT("-"));
        Parts[1].ParseIntoArray(TimeParts, TEXT(":"));
        
        if (DateParts.Num() >= 3 && TimeParts.Num() >= 3)
        {
            Result = FDateTime(
                FCString::Atoi(*DateParts[0]),  // Year
                FCString::Atoi(*DateParts[1]),  // Month
                FCString::Atoi(*DateParts[2]),  // Day
                FCString::Atoi(*TimeParts[0]),  // Hour
                FCString::Atoi(*TimeParts[1]),  // Minute
                FCString::Atoi(*TimeParts[2])   // Second
            );
        }
    }
    
    return Result;
}

// ============================================================================
//                          송신부: Unreal → AI 서버
// ============================================================================

void USenderReceiver::SendGenerationRequest(
    const FString& Prompt,
    const FString& UserEmail,
    const FString& ImagePath)
{
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogMVE, Log, TEXT("║              송신: AI 서버(Form/File) 생성 요청              ║"));
    UE_LOG(LogMVE, Log, TEXT("╚════════════════════════════════════════════════════════════╝"));
    UE_LOG(LogMVE, Log, TEXT(""));

    // 설정 확인
    UE_LOG(LogMVE, Warning, TEXT("🔍 설정 확인:"));
    UE_LOG(LogMVE, Warning, TEXT("  ServerURL = %s"), *ServerURL);
    UE_LOG(LogMVE, Warning, TEXT("  GenerateEndpoint = %s"), *GenerateEndpoint);

    // ------------------------------------------------------------------------
    // HTTP 요청 생성
    // ------------------------------------------------------------------------
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest =
        FHttpModule::Get().CreateRequest();

    FString FullURL = ServerURL + GenerateEndpoint;
    UE_LOG(LogMVE, Warning, TEXT("  FullURL = %s"), *FullURL);

    HttpRequest->SetURL(FullURL);
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetTimeout(600.0f);
    
    UE_LOG(LogMVE, Log, TEXT("  [요청 URL] %s"), *FullURL);

    // ------------------------------------------------------------------------
    // boundary 설정
    // ------------------------------------------------------------------------
    FString Boundary = FString::Printf(TEXT("----UEBoundary%lld"), FDateTime::Now().GetTicks());
    HttpRequest->SetHeader(TEXT("Content-Type"),
        FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    TArray<uint8> BodyData;

    auto AddFormField = [&](const FString& FieldName, const FString& FieldValue)
    {
        FString Part;
        Part += FString::Printf(TEXT("--%s\r\n"), *Boundary);
        Part += FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"\r\n\r\n"), *FieldName);
        Part += FieldValue + TEXT("\r\n");

        FTCHARToUTF8 Conv(*Part);
        BodyData.Append((uint8*)Conv.Get(), Conv.Length());
    };

    // ------------------------------------------------------------------------
    // request_id 생성 및 저장
    // ------------------------------------------------------------------------
    CurrentRequestID = FGuid::NewGuid().ToString();
    UE_LOG(LogMVE, Log, TEXT("  [Request ID] %s"), *CurrentRequestID);

    // Form 필드 전송
    AddFormField(TEXT("prompt"), Prompt);
    AddFormField(TEXT("user_email"), UserEmail);
    AddFormField(TEXT("request_id"), CurrentRequestID);

    // ------------------------------------------------------------------------
    // 이미지 파일 전송 (선택)
    // ------------------------------------------------------------------------
    if (!ImagePath.IsEmpty())
    {
        TArray<uint8> ImageBytes;
        FString Filename;
        FString Mime;
        bool bImageReady = false;

        // ★★★ Base64인지 파일 경로인지 구분 ★★★
        if (ImagePath.StartsWith(TEXT("data:image")) || ImagePath.Len() > 500)
        {
            // Base64 문자열인 경우
            UE_LOG(LogMVE, Log, TEXT("  [이미지] Base64 디코딩 중..."));
            
            FString Base64Data = ImagePath;
            
            // "data:image/png;base64," 접두사 제거
            if (Base64Data.Contains(TEXT(",")))
            {
                FString Header;
                Base64Data.Split(TEXT(","), &Header, &Base64Data);
                
                // MIME 타입 추출
                if (Header.Contains(TEXT("image/png")))
                {
                    Mime = TEXT("image/png");
                    Filename = TEXT("image.png");
                }
                else if (Header.Contains(TEXT("image/jpeg")))
                {
                    Mime = TEXT("image/jpeg");
                    Filename = TEXT("image.jpg");
                }
                else
                {
                    Mime = TEXT("image/png");
                    Filename = TEXT("image.png");
                }
            }
            else
            {
                Mime = TEXT("image/png");
                Filename = TEXT("image.png");
            }
            
            // Base64 디코딩
            if (FBase64::Decode(Base64Data, ImageBytes))
            {
                UE_LOG(LogMVE, Log, TEXT("  [이미지] Base64 디코딩 성공 (%d bytes)"), ImageBytes.Num());
                bImageReady = true;
            }
            else
            {
                UE_LOG(LogMVE, Error, TEXT("  [이미지] Base64 디코딩 실패"));
            }
        }
        else if (FPaths::FileExists(ImagePath))
        {
            // 파일 경로인 경우
            if (FFileHelper::LoadFileToArray(ImageBytes, *ImagePath))
            {
                UE_LOG(LogMVE, Log, TEXT("  [이미지] 파일 로드 성공 (%d bytes)"), ImageBytes.Num());
                
                Filename = FPaths::GetCleanFilename(ImagePath);
                FString Extension = FPaths::GetExtension(ImagePath).ToLower();
                
                if (Extension == TEXT("png"))
                    Mime = TEXT("image/png");
                else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
                    Mime = TEXT("image/jpeg");
                else if (Extension == TEXT("webp"))
                    Mime = TEXT("image/webp");
                else
                    Mime = TEXT("application/octet-stream");
                
                bImageReady = true;
            }
            else
            {
                UE_LOG(LogMVE, Warning, TEXT("  [이미지] 파일 로드 실패: %s"), *ImagePath);
            }
        }
        else
        {
            UE_LOG(LogMVE, Warning, TEXT("  [이미지] 유효하지 않은 이미지 경로/데이터"));
        }

        // 이미지 데이터가 준비되었으면 전송
        if (bImageReady && ImageBytes.Num() > 0)
        {
            FString Header;
            Header += FString::Printf(TEXT("--%s\r\n"), *Boundary);
            Header += FString::Printf(TEXT("Content-Disposition: form-data; name=\"image\"; filename=\"%s\"\r\n"), *Filename);
            Header += FString::Printf(TEXT("Content-Type: %s\r\n\r\n"), *Mime);

            FTCHARToUTF8 HeaderConv(*Header);
            BodyData.Append((uint8*)HeaderConv.Get(), HeaderConv.Length());
            BodyData.Append(ImageBytes);

            FString LineBreak = TEXT("\r\n");
            FTCHARToUTF8 LBConv(*LineBreak);
            BodyData.Append((uint8*)LBConv.Get(), LBConv.Length());
            
            UE_LOG(LogMVE, Log, TEXT("  [이미지] ✓ 전송 준비 완료"));
        }
    }

    // ------------------------------------------------------------------------
    // 종료 boundary
    // ------------------------------------------------------------------------
    {
        FString End = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
        FTCHARToUTF8 Conv(*End);
        BodyData.Append((uint8*)Conv.Get(), Conv.Length());
    }

    // ------------------------------------------------------------------------
    // 전송
    // ------------------------------------------------------------------------
    UE_LOG(LogMVE, Log, TEXT("  [전송 크기] %d bytes"), BodyData.Num());

    HttpRequest->SetContent(BodyData);

    HttpRequest->OnProcessRequestComplete().BindUObject(
        this, &USenderReceiver::HandleGenerationResponse
    );

    if (HttpRequest->ProcessRequest())
    {
        UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ 요청 전송 성공"));
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 요청 전송 실패"));
    }
    
    UE_LOG(LogMVE, Log, TEXT(""));

    LogNetworkDiagnostics(FullURL);
}


// ============================================================================
//                          수신부: GLB 바이너리 직접 수신
// ============================================================================

void USenderReceiver::HandleGenerationResponse(
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bWasSuccessful)
{
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogMVE, Log, TEXT("║              수신: AI 서버 응답 (GLB 직접 수신)              ║"));
    UE_LOG(LogMVE, Log, TEXT("╚════════════════════════════════════════════════════════════╝"));
    UE_LOG(LogMVE, Log, TEXT(""));

    // ------------------------------------------------------------------------
    // 응답 유효성 검사
    // ------------------------------------------------------------------------
    
    if (!bWasSuccessful || !Response.IsValid())
    {
        AnalyzeConnectionError(Response, bWasSuccessful);

        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ HTTP 요청 실패 - 연결 오류"));
        
        FAssetMetadata EmptyMetadata;
        OnGenerationResponse.Broadcast(false, EmptyMetadata, TEXT("HTTP 연결 실패"));
        return;
    }

    int32 ResponseCode = Response->GetResponseCode();
    
    UE_LOG(LogMVE, Log, TEXT("  [응답 코드] %d"), ResponseCode);

    // ------------------------------------------------------------------------
    // 응답 코드 확인
    // ------------------------------------------------------------------------
    
    if (ResponseCode != 200)
    {
        FString ResponseContent = Response->GetContentAsString();
        UE_LOG(LogMVE, Error, TEXT("  [서버 응답] %s"), *ResponseContent);
    
        FAssetMetadata EmptyMetadata;
        
        if (ResponseCode >= 400 && ResponseCode < 500)
        {
            UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 클라이언트 에러 (%d)"), ResponseCode);
            OnGenerationResponse.Broadcast(false, EmptyMetadata, 
                FString::Printf(TEXT("클라이언트 에러: %d"), ResponseCode));
        }
        else if (ResponseCode >= 500)
        {
            UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 서버 에러 (%d)"), ResponseCode);
            OnGenerationResponse.Broadcast(false, EmptyMetadata, 
                FString::Printf(TEXT("서버 에러: %d"), ResponseCode));
        }
        return;
    }

    // ------------------------------------------------------------------------
    // GLB 바이너리 직접 수신
    // ------------------------------------------------------------------------
    
    TArray<uint8> FileData = Response->GetContent();
    int32 FileSize = FileData.Num();
    
    UE_LOG(LogMVE, Log, TEXT("  [수신 크기] %d bytes (%.2f MB)"), 
        FileSize, FileSize / 1024.0f / 1024.0f);

    // GLB 매직 바이트 확인 ("glTF")
    if (FileSize < 4)
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 파일이 너무 작음"));
        
        FAssetMetadata EmptyMetadata;
        OnGenerationResponse.Broadcast(false, EmptyMetadata, TEXT("Invalid File Size"));
        return;
    }

    bool bIsValidGLB = (FileData[0] == 'g' && FileData[1] == 'l' && 
                        FileData[2] == 'T' && FileData[3] == 'F');
    
    if (!bIsValidGLB)
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 잘못된 GLB 파일 (매직 바이트: %c%c%c%c)"), 
            FileData[0], FileData[1], FileData[2], FileData[3]);
        
        FAssetMetadata EmptyMetadata;
        OnGenerationResponse.Broadcast(false, EmptyMetadata, TEXT("Invalid GLB File"));
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("  [파일 타입] ✓ GLB (glTF Binary)"));

    // ------------------------------------------------------------------------
    // 로컬에 저장
    // ------------------------------------------------------------------------
    
    FString SaveDir = GetSaveDirectory();
    FString Filename = FString::Printf(TEXT("%s.glb"), *CurrentRequestID);
    FString LocalPath = SaveDir / Filename;

    if (!FFileHelper::SaveArrayToFile(FileData, *LocalPath))
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 파일 저장 실패: %s"), *LocalPath);
        
        FAssetMetadata EmptyMetadata;
        OnGenerationResponse.Broadcast(false, EmptyMetadata, TEXT("File Save Failed"));
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ 파일 저장 성공"));
    UE_LOG(LogMVE, Log, TEXT("  [경로] %s"), *LocalPath);

    // ------------------------------------------------------------------------
    // 메타데이터 생성
    // ------------------------------------------------------------------------
    
    FAssetMetadata Metadata;
    FGuid ParsedGuid;
    FGuid::Parse(CurrentRequestID, ParsedGuid);
    Metadata.AssetID = ParsedGuid;
    if (!Metadata.AssetID.IsValid())
    {
        Metadata.AssetID = FGuid::NewGuid();
    }
    Metadata.AssetType = EAssetType::MESH;
    Metadata.LocalPath = LocalPath;
    Metadata.DisplayName = FString::Printf(TEXT("GeneratedMesh_%s"), 
        *FDateTime::Now().ToString(TEXT("%H%M%S")));
    Metadata.Date = FDateTime::Now();
    
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("  [메타데이터]"));
    UE_LOG(LogMVE, Log, TEXT("    - AssetID: %s"), *Metadata.AssetID.ToString());
    UE_LOG(LogMVE, Log, TEXT("    - DisplayName: %s"), *Metadata.DisplayName);
    UE_LOG(LogMVE, Log, TEXT("    - LocalPath: %s"), *Metadata.LocalPath);

    // ------------------------------------------------------------------------
    // 델리게이트 발동
    // ------------------------------------------------------------------------
    
    OnGenerationResponse.Broadcast(true, Metadata, TEXT(""));

    // ------------------------------------------------------------------------
    // UE 에셋 변환
    // ------------------------------------------------------------------------
    
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("  [상태] → UE 에셋 변환 시작..."));
    
    UObject* LoadedAsset = ConvertFileToAsset(Metadata);

    if (LoadedAsset)
    {
        UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ UE 에셋 변환 완료"));
        OnAssetLoaded.Broadcast(LoadedAsset, Metadata);
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ UE 에셋 변환 실패"));
    }
    
    UE_LOG(LogMVE, Log, TEXT(""));
}

// ============================================================================
//                          다운로드
// ============================================================================

void USenderReceiver::DownloadFromFileServer(const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogMVE, Log, TEXT("║              다운로드: 파일 서버 → 로컬                      ║"));
    UE_LOG(LogMVE, Log, TEXT("╚════════════════════════════════════════════════════════════╝"));
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("  [URL] %s"), *Metadata.RemotePath);
    UE_LOG(LogMVE, Log, TEXT("  [타입] %d"), (int32)Metadata.AssetType);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = 
        FHttpModule::Get().CreateRequest();

    HttpRequest->SetURL(Metadata.RemotePath);
    HttpRequest->SetVerb(TEXT("GET"));
    HttpRequest->SetTimeout(RequestTimeout);

    // 완료 콜백
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, Metadata](
            TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Req,
            TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Res,
            bool bSuccess)
        {
            HandleDownloadComplete(Req, Res, bSuccess, Metadata);
        }
    );

    if (HttpRequest->ProcessRequest())
    {
        UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ 다운로드 시작"));
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 다운로드 시작 실패"));
    }
    
    UE_LOG(LogMVE, Log, TEXT(""));
}

void USenderReceiver::HandleDownloadComplete(
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bWasSuccessful,
    FAssetMetadata Metadata)
{
    UE_LOG(LogMVE, Log, TEXT(""));
    UE_LOG(LogMVE, Log, TEXT("╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogMVE, Log, TEXT("║              다운로드 완료 처리                              ║"));
    UE_LOG(LogMVE, Log, TEXT("╚════════════════════════════════════════════════════════════╝"));
    UE_LOG(LogMVE, Log, TEXT(""));

    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 다운로드 실패"));
        return;
    }

    int32 ResponseCode = Response->GetResponseCode();
    if (ResponseCode != 200)
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ HTTP %d 에러"), ResponseCode);
        return;
    }

    // 파일 저장
    TArray<uint8> FileData = Response->GetContent();
    
    FString Extension = GetFileExtension(Metadata.AssetType);
    FString Filename = FString::Printf(TEXT("%s.%s"), 
        *Metadata.AssetID.ToString(), 
        *Extension);
    
    FString SaveDir = GetSaveDirectory();
    FString LocalPath = SaveDir / Filename;

    if (FFileHelper::SaveArrayToFile(FileData, *LocalPath))
    {
        UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ 파일 저장 성공"));
        UE_LOG(LogMVE, Log, TEXT("  [경로] %s"), *LocalPath);
        UE_LOG(LogMVE, Log, TEXT("  [크기] %.2f MB"), FileData.Num() / 1024.0f / 1024.0f);

        // 메타데이터 업데이트
        FAssetMetadata UpdatedMetadata = Metadata;
        UpdatedMetadata.LocalPath = LocalPath;

        // UE 오브젝트 변환
        UObject* LoadedAsset = ConvertFileToAsset(UpdatedMetadata);

        if (LoadedAsset)
        {
            UE_LOG(LogMVE, Log, TEXT("  [상태] ✓ UE 에셋 변환 완료"));
            OnAssetLoaded.Broadcast(LoadedAsset, UpdatedMetadata);
        }
        else
        {
            UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ UE 에셋 변환 실패"));
        }
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("  [상태] ✗ 파일 저장 실패: %s"), *LocalPath);
    }
    
    UE_LOG(LogMVE, Log, TEXT(""));
}

// ============================================================================
//                          로컬 파일 로드
// ============================================================================

void USenderReceiver::LoadFromLocalFile(const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, TEXT("[LocalLoad] 시작: %s"), *Metadata.RemotePath);

    if (!FPaths::FileExists(Metadata.RemotePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[LocalLoad] 파일 없음: %s"), *Metadata.RemotePath);
        return;
    }

    FAssetMetadata UpdatedMetadata = Metadata;
    UpdatedMetadata.LocalPath = Metadata.RemotePath;

    UObject* LoadedAsset = ConvertFileToAsset(UpdatedMetadata);

    if (LoadedAsset)
    {
        UE_LOG(LogMVE, Log, TEXT("[LocalLoad] ✓ 로드 성공"));
        OnAssetLoaded.Broadcast(LoadedAsset, UpdatedMetadata);
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[LocalLoad] ✗ 로드 실패"));
    }
}

// ============================================================================
//                          에셋 변환
// ============================================================================

UObject* USenderReceiver::ConvertFileToAsset(const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, TEXT("[AssetConvert] 타입: %d, 경로: %s"), 
        (int32)Metadata.AssetType, *Metadata.LocalPath);

    switch (Metadata.AssetType)
    {
    case EAssetType::IMAGE:
        return LoadImageAsTexture(Metadata.LocalPath);

    case EAssetType::MESH:
        return LoadMeshFromGLB(Metadata.LocalPath);

    default:
        UE_LOG(LogMVE, Warning, TEXT("[AssetConvert] 지원하지 않는 타입: %d"), 
            (int32)Metadata.AssetType);
        return nullptr;
    }
}

UTexture2D* USenderReceiver::LoadImageAsTexture(const FString& FilePath)
{
    UE_LOG(LogMVE, Log, TEXT("[ImageLoader] 로드 시작: %s"), *FilePath);

    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[ImageLoader] 파일 로드 실패"));
        return nullptr;
    }

    IImageWrapperModule& ImageWrapperModule = 
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

    EImageFormat Format = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());

    if (Format == EImageFormat::Invalid)
    {
        UE_LOG(LogMVE, Error, TEXT("[ImageLoader] 알 수 없는 이미지 포맷"));
        return nullptr;
    }

    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
    {
        UE_LOG(LogMVE, Error, TEXT("[ImageLoader] 이미지 디코딩 실패"));
        return nullptr;
    }

    TArray<uint8> RawData;
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
    {
        UE_LOG(LogMVE, Error, TEXT("[ImageLoader] Raw 데이터 추출 실패"));
        return nullptr;
    }

    UTexture2D* Texture = UTexture2D::CreateTransient(
        ImageWrapper->GetWidth(),
        ImageWrapper->GetHeight(),
        PF_B8G8R8A8
    );

    if (!Texture)
    {
        UE_LOG(LogMVE, Error, TEXT("[ImageLoader] Texture2D 생성 실패"));
        return nullptr;
    }

    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
    Texture->UpdateResource();

    UE_LOG(LogMVE, Log, TEXT("[ImageLoader] ✓ 완료 (%dx%d)"), 
        Texture->GetSizeX(), Texture->GetSizeY());

    return Texture;
}
// GLB 파일 로더 (SkeletalMesh 시도 후 실패 시 StaticMesh로 fallback)
UObject* USenderReceiver::LoadMeshFromGLB(const FString& FilePath)
{
    UE_LOG(LogMVE, Log, TEXT("[MeshLoader] 로드 시작: %s"), *FilePath);

    // 파일 로드
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[MeshLoader] 파일 로드 실패"));
        return nullptr;
    }

    UE_LOG(LogMVE, Log, TEXT("[MeshLoader] 파일 크기: %.2f MB"), FileData.Num() / 1024.0f / 1024.0f);

    // glTFRuntime으로 로드
    FglTFRuntimeConfig Config;
    Config.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;  // UE 좌표계로 변환
    UglTFRuntimeAsset* Asset =
        UglTFRuntimeFunctionLibrary::glTFLoadAssetFromData(FileData, Config);

    if (!Asset)
    {
        UE_LOG(LogMVE, Error, TEXT("[MeshLoader] glTF 에셋 로드 실패"));
        return nullptr;
    }

    // 1단계: SkeletalMesh 시도 (본이 있는 경우)
    UE_LOG(LogMVE, Log, TEXT("[MeshLoader] 1단계: SkeletalMesh 시도..."));
    
    FglTFRuntimeSkeletalMeshConfig SkMeshConfig;
    SkMeshConfig.bOverwriteRefSkeleton = true;
    SkMeshConfig.MorphTargetsDuplicateStrategy = EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;

    USkeletalMesh* SkMesh = Asset->LoadSkeletalMesh(0, 0, SkMeshConfig);

    if (SkMesh)
    {
        const FReferenceSkeleton& RefSkel = SkMesh->GetRefSkeleton();
        UE_LOG(LogMVE, Log, TEXT("[MeshLoader] SkeletalMesh 생성 성공"));
        UE_LOG(LogMVE, Log, TEXT("  - Bones: %d"), RefSkel.GetNum());
        UE_LOG(LogMVE, Log, TEXT("  - MorphTargets: %d"), SkMesh->GetMorphTargets().Num());
        UE_LOG(LogMVE, Log, TEXT("  - Materials: %d"), SkMesh->GetMaterials().Num());
        return SkMesh;
    }

    // 2단계: SkeletalMesh 실패 시 StaticMesh로 fallback
    UE_LOG(LogMVE, Warning, TEXT("[MeshLoader] SkeletalMesh 생성 실패 (본 없음)"));
    UE_LOG(LogMVE, Log, TEXT("[MeshLoader] 2단계: StaticMesh로 fallback..."));

    return LoadStaticMeshFromGLB(FilePath);
}

// GLB 파일을 StaticMesh로 로드
UStaticMesh* USenderReceiver::LoadStaticMeshFromGLB(const FString& FilePath)
{
    UE_LOG(LogMVE, Log, TEXT("[StaticMeshLoader] 로드 시작: %s"), *FilePath);

    // 파일 재로드
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[StaticMeshLoader] 파일 로드 실패"));
        return nullptr;
    }

    // glTFRuntime으로 로드
    FglTFRuntimeConfig Config;
    Config.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;  // UE 좌표계로 변환
    UglTFRuntimeAsset* Asset =
        UglTFRuntimeFunctionLibrary::glTFLoadAssetFromData(FileData, Config);

    if (!Asset)
    {
        UE_LOG(LogMVE, Error, TEXT("[StaticMeshLoader] glTF 에셋 로드 실패"));
        return nullptr;
    }

    // StaticMesh 설정
    FglTFRuntimeStaticMeshConfig StMeshConfig;
    StMeshConfig.bReverseWinding = false;
    StMeshConfig.bBuildSimpleCollision = true;

    // 첫 번째 메시를 StaticMesh로 로드
    UStaticMesh* StMesh = Asset->LoadStaticMesh(0, StMeshConfig);

    if (StMesh)
    {
        UE_LOG(LogMVE, Log, TEXT("[StaticMeshLoader] StaticMesh 생성 성공"));
        UE_LOG(LogMVE, Log, TEXT("  - Vertices: %d"), StMesh->GetNumVertices(0));
        UE_LOG(LogMVE, Log, TEXT("  - Triangles: %d"), StMesh->GetNumTriangles(0));
        UE_LOG(LogMVE, Log, TEXT("  - Materials: %d"), StMesh->GetStaticMaterials().Num());
        UE_LOG(LogMVE, Log, TEXT("  - LODs: %d"), StMesh->GetNumLODs());
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[StaticMeshLoader] StaticMesh 생성 실패"));
    }

    return StMesh;
}
void USenderReceiver::LogNetworkDiagnostics(const FString& URL)
{
    UE_LOG(LogMVE, Warning, TEXT("======================================="));
    UE_LOG(LogMVE, Warning, TEXT("[네트워크 진단 시작]"));
    UE_LOG(LogMVE, Warning, TEXT("요청 URL: %s"), *URL);

    FString Host, Protocol, Path, Port;
    if (URL.Split(TEXT("://"), &Protocol, &Host))
    {
        UE_LOG(LogMVE, Warning, TEXT(" - 프로토콜: %s"), *Protocol);

        FString HostPart = Host;
        if (HostPart.Split(TEXT("/"), &HostPart, &Path))
            UE_LOG(LogMVE, Warning, TEXT(" - Path: /%s"), *Path);

        if (HostPart.Split(TEXT(":"), &Host, &Port))
        {
            UE_LOG(LogMVE, Warning, TEXT(" - Host: %s"), *Host);
            UE_LOG(LogMVE, Warning, TEXT(" - Port: %s"), *Port);
        }
        else
        {
            UE_LOG(LogMVE, Warning, TEXT(" - Host: %s"), *Host);
            UE_LOG(LogMVE, Warning, TEXT(" - Port: (기본 포트)"));
        }
    }

    UE_LOG(LogMVE, Warning, TEXT("[네트워크 진단 종료]"));
    UE_LOG(LogMVE, Warning, TEXT("======================================="));
}

// ============================================================================
//                          에러 분석
// ============================================================================

void USenderReceiver::AnalyzeConnectionError(TSharedPtr<IHttpResponse> Response, bool bWasSuccessful)
{
    UE_LOG(LogMVE, Error, TEXT("──────────── 연결 체크 결과 ────────────"));
    
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogMVE, Error, TEXT("✔ 요청이 서버에 닿지 못함 (TCP 수준 오류)"));
        UE_LOG(LogMVE, Error, TEXT("가능한 원인:"));
        UE_LOG(LogMVE, Error, TEXT(" - 서버 꺼짐"));
        UE_LOG(LogMVE, Error, TEXT(" - 방화벽에서 언리얼 차단"));
        UE_LOG(LogMVE, Error, TEXT(" - 서버 host가 127.0.0.1로 바인딩됨 (0.0.0.0 필요)"));
        UE_LOG(LogMVE, Error, TEXT(" - URL 조합 오류 (/endpoint 누락)"));
        UE_LOG(LogMVE, Error, TEXT(" - 포트 불일치"));
        
        if (Response.IsValid())
        {
            FString Content = Response->GetContentAsString();
            if (!Content.IsEmpty() && Content.Len() < 500)
            {
                UE_LOG(LogMVE, Error, TEXT("[서버 응답 내용]"));
                UE_LOG(LogMVE, Error, TEXT("%s"), *Content);
            }
        }

        UE_LOG(LogMVE, Error, TEXT("────────────────────────────────────"));
        return;
    }

    // 응답은 있는데 실패한 경우
    int32 Code = Response->GetResponseCode();
    UE_LOG(LogMVE, Error, TEXT("HTTP 코드: %d"), Code);

    if (Code == 404)
    {
        UE_LOG(LogMVE, Error, TEXT("✔ 엔드포인트 없음 → GenerateEndpoint 확인 필요"));
    }
    if (Code == 405)
    {
        UE_LOG(LogMVE, Error, TEXT("✔ 서버가 POST를 허용하지 않음 → CORS allow_methods 수정 필요"));
    }
    if (Code == 500)
    {
        UE_LOG(LogMVE, Error, TEXT("✔ 서버 내부 오류 → FormField 이름 mismatch 가능"));
    }
    if (Code == 422)
    {
        UE_LOG(LogMVE, Error, TEXT("✔ Form 필드 누락 → prompt/user_email/request_id 확인"));
    }

    UE_LOG(LogMVE, Error, TEXT("[서버 응답 내용]"));
    FString Content = Response->GetContentAsString();
    if (Content.Len() > 500)
    {
        UE_LOG(LogMVE, Error, TEXT("%s... (truncated)"), *Content.Left(500));
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("%s"), *Content);
    }
    UE_LOG(LogMVE, Error, TEXT("────────────────────────────────────"));
}


// ============================================================================
//                          상태 폴링 (비동기 작업용)
// ============================================================================

void USenderReceiver::CheckGenerationStatus()
{
    UE_LOG(LogMVE, Log, TEXT("[MVE] 📡 상태 확인 중... (ID: %s)"), *CurrentRequestID);
    
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    
    FString StatusURL = ServerURL + "/status/" + CurrentRequestID;
    
    HttpRequest->SetURL(StatusURL);
    HttpRequest->SetVerb(TEXT("GET"));
    HttpRequest->SetTimeout(600.0f);
    
    HttpRequest->OnProcessRequestComplete().BindUObject(
        this,
        &USenderReceiver::OnStatusCheckComplete
    );
    
    if (!HttpRequest->ProcessRequest())
    {
        UE_LOG(LogMVE, Error, TEXT("[MVE] 상태 확인 요청 전송 실패"));
    }
}

void USenderReceiver::OnStatusCheckComplete(
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogMVE, Warning, TEXT("[MVE] 상태 확인 실패 (재시도 중)"));
        return;
    }
    
    int32 ResponseCode = Response->GetResponseCode();
    
    if (ResponseCode != 200)
    {
        UE_LOG(LogMVE, Warning, TEXT("[MVE] 상태 확인 에러: %d"), ResponseCode);
        return;
    }
    
    FString ResponseString = Response->GetContentAsString();
    UE_LOG(LogMVE, Verbose, TEXT("[MVE] 상태 응답: %s"), *ResponseString);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogMVE, Error, TEXT("[MVE] JSON 파싱 실패"));
        return;
    }
    
    FString Status = JsonObject->GetStringField(TEXT("status"));
    UE_LOG(LogMVE, Log, TEXT("[MVE] 현재 상태: %s"), *Status);
    
    if (Status == TEXT("completed"))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(StatusCheckTimer);
        }
        
        FString GLBURL = JsonObject->GetStringField(TEXT("glb_url"));
        UE_LOG(LogMVE, Log, TEXT("[MVE] 생성 완료! GLB URL: %s"), *GLBURL);
        
        FAssetMetadata Metadata;
        Metadata.AssetID = FGuid::NewGuid();
        Metadata.AssetType = EAssetType::MESH;
        Metadata.RemotePath = GLBURL;
        
        DownloadFromFileServer(Metadata);
    }
    else if (Status == TEXT("failed"))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(StatusCheckTimer);
        }
        
        FString ErrorMsg = JsonObject->HasField(TEXT("error")) 
            ? JsonObject->GetStringField(TEXT("error"))
            : TEXT("알 수 없는 오류");
        
        UE_LOG(LogMVE, Error, TEXT("[MVE] 생성 실패: %s"), *ErrorMsg);
    }
    else if (Status == TEXT("processing") || Status == TEXT("queued"))
    {
        UE_LOG(LogMVE, Log, TEXT("[MVE] 작업 진행 중... (5초 후 재확인)"));
    }
    else
    {
        UE_LOG(LogMVE, Warning, TEXT("[MVE] 알 수 없는 상태: %s"), *Status);
    }
}