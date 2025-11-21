// Fill out your copyright notice in the Description page of Project Settings.

#include "SenderReceiver.h"
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

void UGenAISenderReceiver::RequestGeneration(const FString& Prompt, const FString& UserEmail,
                                             const FString& OptionalImagePath)
{
	PRINTLOG(TEXT("생성 요청 시작"))
	PRINTLOG(TEXT("prompt:%s"), *Prompt);
	PRINTLOG(TEXT("userEmail:%s"), *UserEmail);
	UE_LOG(LogMVE, Warning, TEXT("  - Image: %s"), OptionalImagePath.IsEmpty() ? TEXT("없음") : *OptionalImagePath);

	// http 요청 객체 생성
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	FString FullURL = ServerURL + GenerateEndpoint;

	
	
	HttpRequest->SetURL(FullURL);
	HttpRequest->SetVerb(TEXT("POST"));

	// 멀티파츠 구성
	FString Boundary = FString::Printf(TEXT("----UnrealBoundary%d"),FDateTime::Now().GetTicks());

	HttpRequest->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("application/octet-stream, boundary=%s"), *Boundary));

	//Json 메타 데이터
	// Json 객체 생성
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("Prompt"), Prompt);
	JsonObject->SetStringField(TEXT("UserEmail"), UserEmail);
	JsonObject->SetStringField(TEXT("OptionalImagePath"), FGuid::NewGuid().ToString());

	// Json을 문자열로 직렬화
	FString JsonContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonContent);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// HTTP Body 데이터
	TArray<uint8> BodyData;
	FString BodyString;

	//Json 데이터
	BodyString += FString::Printf(TEXT("%s\r\n"),*Boundary);
	BodyString += TEXT("Content-Disposition: form-data; name=\"metadata\"\r\n");
	BodyString += TEXT("Content-Type: application/json\r\n\r\n");
	
	// 이미지 파일
	if (!OptionalImagePath.IsEmpty() && FPaths::FileExists(OptionalImagePath))
	{
		TArray<uint8> ImageData;
		if (FFileHelper::LoadFileToArray(ImageData,*OptionalImagePath))
		{
			UE_LOG(LogMVE, Log , TEXT("이미지 로드 성공 : %d bytes"), ImageData.Num());
			FString Extension = FPaths::GetExtension(OptionalImagePath).ToLower();
			FString MimeType = TEXT("image/png");

			if (Extension == TEXT("png"))
				MimeType = TEXT("image/png");
			else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
				MimeType = TEXT("image/jpeg");
			else if (Extension == TEXT("webp"))
				MimeType = TEXT("image/webp");
			//Image Part 헤더
			BodyString += FString::Printf(TEXT("%s\r\n"),*Boundary);
			BodyString += TEXT("Content-Disposition: form-data; name=\"metadata\"\r\n");
			BodyString += TEXT("Content-Type: application/json\r\n\r\n");
			
			// 현재까지의 텍스트를 바이너리로 변환 하여 바디에 추가한다
			BodyData.Append((uint8*)TCHAR_TO_UTF8(*BodyString), BodyString.Len());
			BodyString.Empty();
			
			// 이미지 바이너리 데이터를 Body에 추가
			BodyData.Append(ImageData);

			// 줄바꿈
			BodyString += TEXT("\r\n");
		}
		else
		{
			UE_LOG(LogMVE, Warning,TEXT("이미지 파일 로드 실패 %s"),*OptionalImagePath);
		}
	}
	else if (!OptionalImagePath.IsEmpty())
	{
		UE_LOG(LogMVE, Warning,TEXT("이미지 파일이 존재하지 않음 %s"),*OptionalImagePath);
	}
	BodyString += FString::Printf(TEXT("--%s--\r\n"), *Boundary);

	// 최종 Body 완성
	BodyData.Append((uint8*)TCHAR_TO_UTF8(*BodyString), BodyString.Len());

	UE_LOG(LogMVE, Log, TEXT("[GenAI Sender] 최종 Body 크기: %d bytes"), BodyData.Num());
	
	// HTTP 요청 전송
	HttpRequest->SetContent(BodyData);

	// 응답 핸들러 바인딩
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this,
		&UGenAISenderReceiver::OnGenerationRequestComplete
	);

	// 요청 전송
	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogMVE, Log, TEXT("요청 전송 성공: %s"), *FullURL);
	}
	else
	{
		UE_LOG(LogMVE, Error, TEXT("요청 전송 실패"));
	}
}

void UGenAISenderReceiver::OnGenerationRequestComplete(
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bWasSuccessful)
{

    // 응답 유효성 검사
	if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogMVE, Error, TEXT("[GenAI Sender] HTTP 요청 실패"));
        return;
    }

    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseString = Response->GetContentAsString();

    UE_LOG(LogMVE, Log, TEXT("[GenAI Sender] 응답 코드: %d"), ResponseCode);
    UE_LOG(LogMVE, Verbose, TEXT("[GenAI Sender] 응답 내용: %s"), *ResponseString);
	
    // 응답 코드별 처리
    if (ResponseCode == 200)
    {
        // 성공: 요청이 큐에 등록됨
        UE_LOG(LogMVE, Log, TEXT("[GenAI Sender] ✓ 생성 요청이 큐에 등록되었습니다"));
        
        // 서버 응답 JSON 파싱 (선택사항)
    }
    else if (ResponseCode >= 400 && ResponseCode < 500)
    {
        // 클라이언트 에러 (잘못된 요청)
        UE_LOG(LogMVE, Error, 
            TEXT("[GenAI Sender] ✗ 클라이언트 에러 (%d): %s"), 
            ResponseCode, *ResponseString);
    }
    else if (ResponseCode >= 500)
    {
        // 서버 에러
        UE_LOG(LogMVE, Error, 
            TEXT("[GenAI Sender] ✗ 서버 에러 (%d): %s"), 
            ResponseCode, *ResponseString);
    }
}


//수신부: 에셋 다운로드 및 로드
void UGenAISenderReceiver::DownloadAsset(const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, TEXT("[GenAI Receiver] 다운로드 시작"));
    UE_LOG(LogMVE, Log, TEXT("  - AssetID: %s"), *Metadata.AssetID.ToString());
    UE_LOG(LogMVE, Log, TEXT("  - Type: %d"), (int32)Metadata.AssetType);
    UE_LOG(LogMVE, Log, TEXT("  - URL: %s"), *Metadata.RemotePath);


    //HTTP GET 요청 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = 
        FHttpModule::Get().CreateRequest();

    HttpRequest->SetURL(Metadata.RemotePath);
    HttpRequest->SetVerb(TEXT("GET"));


    // 응답 핸들러 바인딩
    HttpRequest->OnProcessRequestComplete().BindUObject(
        this,
        &UGenAISenderReceiver::OnAssetDownloaded,
        Metadata  // 메타데이터를 복사하여 핸들러에 전달
    );
	
    //요청 전송
    if (HttpRequest->ProcessRequest())
    {
        UE_LOG(LogMVE, Log, TEXT("[GenAI Receiver] ✓ 다운로드 요청 전송 성공"));
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[GenAI Receiver] ✗ 다운로드 요청 전송 실패"));
    }
}

void UGenAISenderReceiver::OnAssetDownloaded(
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bWasSuccessful,
    FAssetMetadata Metadata)
{

    // 응답 유효성 검사
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogMVE, Error, TEXT("[GenAI Receiver] 다운로드 실패"));
        return;
    }

    int32 ResponseCode = Response->GetResponseCode();
    if (ResponseCode != 200)
    {
        UE_LOG(LogMVE, Error, 
            TEXT("[GenAI Receiver] 다운로드 실패 (코드: %d)"), ResponseCode);
        return;
    }

    // 바이너리 데이터 추출
    const TArray<uint8>& FileData = Response->GetContent();
    
    UE_LOG(LogMVE, Log, 
        TEXT("[GenAI Receiver] 다운로드 완료: %d bytes"), FileData.Num());


    // 로컬 파일로 저장
    // 저장 디렉토리 경로
    // 예: "C:/MyProject/Saved/GenAIAssets/"
    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("GenAIAssets");
    
    // 디렉토리 생성 (없으면)
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDir))
    {
        PlatformFile.CreateDirectory(*SaveDir);
        UE_LOG(LogMVE, Log, TEXT("[GenAI Receiver] 디렉토리 생성: %s"), *SaveDir);
    }

    // 파일 확장자 결정
    FString Extension;
    switch (Metadata.AssetType)
    {
    case EGenAIAssetType::MESH:
        Extension = TEXT("glb");
        break;
    case EGenAIAssetType::IMAGE:
        Extension = TEXT("png");
        break;
    case EGenAIAssetType::AUDIO:
        Extension = TEXT("wav");
        break;
    case EGenAIAssetType::VIDEO:
        Extension = TEXT("mp4");
        break;
    default:
        Extension = TEXT("dat");
        break;
    }

    // 로컬 파일 경로 생성
    // 예: "Saved/GenAIAssets/abc-123-def-456.glb"
    FString LocalFilePath = SaveDir / FString::Printf(TEXT("%s.%s"), 
        *Metadata.AssetID.ToString(), *Extension);

    // 파일 저장
    if (!FFileHelper::SaveArrayToFile(FileData, *LocalFilePath))
    {
        UE_LOG(LogMVE, Error, 
            TEXT("[GenAI Receiver] 파일 저장 실패: %s"), *LocalFilePath);
        return;
    }

    UE_LOG(LogMVE, Log, 
        TEXT("[GenAI Receiver] ✓ 파일 저장 완료: %s"), *LocalFilePath);


    // 메타데이터 업데이트
    FAssetMetadata UpdatedMetadata = Metadata;
    UpdatedMetadata.LocalPath = LocalFilePath;
	
    // 언리얼 에셋으로 로드
    UObject* LoadedAsset = LoadAssetFromLocalFile(UpdatedMetadata);
    
    if (LoadedAsset)
    {
        UE_LOG(LogMVE, Log, 
            TEXT("[GenAI Receiver] ✓ 에셋 로드 완료: %s"), 
            *LoadedAsset->GetClass()->GetName());

        // 델리게이트 발동 (블루프린트/UI에 전달)
        OnAssetGenerated.Broadcast(LoadedAsset, UpdatedMetadata);
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[GenAI Receiver] 에셋 로드 실패"));
    }
}


// 타입별 에셋 로더
UObject* UGenAISenderReceiver::LoadAssetFromLocalFile(const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, 
        TEXT("[GenAI Loader] 에셋 타입: %d, 경로: %s"),
        (int32)Metadata.AssetType, *Metadata.LocalPath);

    // 타입별 로더 호출
    switch (Metadata.AssetType)
    {
    case EGenAIAssetType::IMAGE:
        return LoadImageFromFile(Metadata.LocalPath);

    case EGenAIAssetType::MESH:
        return LoadMeshFromFile(Metadata.LocalPath);

    case EGenAIAssetType::AUDIO:
        UE_LOG(LogMVE, Warning, 
            TEXT("[GenAI Loader] 오디오 로더는 아직 구현되지 않았습니다"));
        return nullptr;

    case EGenAIAssetType::VIDEO:
        UE_LOG(LogMVE, Warning, 
            TEXT("[GenAI Loader] 비디오 로더는 아직 구현되지 않았습니다"));
        return nullptr;

    default:
        UE_LOG(LogMVE, Warning, 
            TEXT("[GenAI Loader] 알 수 없는 에셋 타입"));
        return nullptr;
    }
}


// 이미지 로더 (PNG, JPG)
UTexture2D* UGenAISenderReceiver::LoadImageFromFile(const FString& FilePath)
{
    UE_LOG(LogMVE, Log, TEXT("[Image Loader] 이미지 로드 시작: %s"), *FilePath);


    // 파일 데이터 로드
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] 파일 로드 실패"));
        return nullptr;
    }
	
    // 이미지 포맷 감지
    IImageWrapperModule& ImageWrapperModule = 
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

    EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(
        FileData.GetData(), FileData.Num());
    
    if (ImageFormat == EImageFormat::Invalid)
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] 잘못된 이미지 포맷"));
        return nullptr;
    }

    UE_LOG(LogMVE, Verbose, TEXT("[Image Loader] 포맷 감지 성공: %d"), (int32)ImageFormat);


    // 이미지 래퍼 생성
    TSharedPtr<IImageWrapper> ImageWrapper = 
        ImageWrapperModule.CreateImageWrapper(ImageFormat);
    
    if (!ImageWrapper.IsValid())
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] 이미지 래퍼 생성 실패"));
        return nullptr;
    }


    // 압축된 이미지 데이터 설정
    if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] 압축 데이터 설정 실패"));
        return nullptr;
    }


    //Raw 데이터 추출 (BGRA 포맷
    TArray<uint8> RawData;
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] Raw 데이터 추출 실패"));
        return nullptr;
    }

    int32 Width = ImageWrapper->GetWidth();
    int32 Height = ImageWrapper->GetHeight();

    UE_LOG(LogMVE, Log, TEXT("[Image Loader] 이미지 크기: %dx%d"), Width, Height);

    //UTexture2D 생성

    UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    
    if (!Texture)
    {
        UE_LOG(LogMVE, Error, TEXT("[Image Loader] 텍스처 생성 실패"));
        return nullptr;
    }


    //텍스처 데이터 복사
    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();


    //텍스처 업데이트
    Texture->UpdateResource();

    UE_LOG(LogMVE, Log, TEXT("[Image Loader] ✓ 텍스처 생성 완료"));

    return Texture;
}

// GLB 메시 로더
USkeletalMesh* UGenAISenderReceiver::LoadMeshFromFile(const FString& FilePath)
{
    UE_LOG(LogMVE, Log, TEXT("[Mesh Loader] GLB 로드 시작: %s"), *FilePath);
	
    // 파일 데이터 로드

    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Mesh Loader] 파일 로드 실패"));
        return nullptr;
    }

    UE_LOG(LogMVE, Verbose, TEXT("[Mesh Loader] 파일 크기: %d bytes"), FileData.Num());


    //glTFRuntime Asset 생성
    FglTFRuntimeConfig LoaderConfig;
    // 필요시 여기에 설정 추가
  
    
    UglTFRuntimeAsset* RuntimeAsset = 
        UglTFRuntimeFunctionLibrary::glTFLoadAssetFromData(FileData, LoaderConfig);

    if (!RuntimeAsset)
    {
        UE_LOG(LogMVE, Error, TEXT("[Mesh Loader] glTF Asset 로드 실패"));
        return nullptr;
    }

    UE_LOG(LogMVE, Log, TEXT("[Mesh Loader] ✓ glTF Asset 로드 성공"));
	
    //  SkeletalMesh 생성
    FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
    SkeletalMeshConfig.bOverwriteRefSkeleton = true;
    SkeletalMeshConfig.MorphTargetsDuplicateStrategy = 
        EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;

    // 첫 번째 메시 로드 
    USkeletalMesh* LoadedMesh = RuntimeAsset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);

    if (!LoadedMesh)
    {
        UE_LOG(LogMVE, Error, TEXT("[Mesh Loader] SkeletalMesh 생성 실패"));
        return nullptr;
    }

    // 메시 정보 로깅
    const FReferenceSkeleton& RefSkeleton = LoadedMesh->GetRefSkeleton();
    int32 BoneCount = RefSkeleton.GetNum();
    
    UE_LOG(LogMVE, Log, TEXT("[Mesh Loader] ✓ SkeletalMesh 생성 완료"));
    UE_LOG(LogMVE, Log, TEXT("  - Bone 개수: %d"), BoneCount);
    UE_LOG(LogMVE, Log, TEXT("  - MorphTarget 개수: %d"), 
        LoadedMesh->GetMorphTargets().Num());

    return LoadedMesh;
}

void UGenAISenderReceiver::LoadLocalAsset(const FAssetMetadata& Metadata)
{
	UE_LOG(LogMVE,Warning, TEXT(" 로컬파일 직접 로드 시작합니다"));
	UE_LOG(LogMVE,Warning, TEXT(" 경로 %s"), *Metadata.RemotePath);
	UE_LOG(LogMVE,Warning, TEXT(" 타입 %d"),(int32)Metadata.AssetType);
	if (!FPaths::FileExists(Metadata.RemotePath))
	{
		UE_LOG(LogMVE,Warning, TEXT(" 파일이 존재하지 %s"),*Metadata.RemotePath);
		return;
	}
	FAssetMetadata UpdateMetadata = Metadata;
	UpdateMetadata.LocalPath = Metadata.RemotePath;

	UObject* LoadedAsset = LoadAssetFromLocalFile(UpdateMetadata);
	if (LoadedAsset)
	{
		UE_LOG(LogMVE,Warning,TEXT("로컬 에셋 로드 완료함 %s"),*LoadedAsset->GetClass()->GetName());
		OnAssetGenerated.Broadcast(LoadedAsset, UpdateMetadata);
	}
	else
	{
		UE_LOG(LogMVE,Warning,TEXT("로컬 에셋 로드 실패함"));
	}
	
}
