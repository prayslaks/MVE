
#include "../Public/MVE_AUD_CustomizationManager.h"

#include "DesktopPlatformModule.h"
#include "HttpModule.h"
#include "IDesktopPlatform.h"
#include "MVE.h"
#include "Interfaces/IHttpResponse.h"

FString UMVE_AUD_CustomizationManager::OpenReferenceImageDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		PRINTLOG(TEXT("Desktop Platform not available"));
		return TEXT("");
	}

	TArray<FString> OutFiles;
	const FString Filter = TEXT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg");
    
	if (DesktopPlatform->OpenFileDialog(
		nullptr,
		TEXT("참고 이미지 선택"),
		FPaths::ProjectContentDir(),
		TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			if (LoadReferenceImage(OutFiles[0]))
			{
				return ReferenceImageFileName;
			}
		}
	}

	return TEXT("");
}

void UMVE_AUD_CustomizationManager::RequestModelGeneration(const FString& PromptText)
{
	if (ReferenceImageData.Num() == 0)
	{
		PRINTLOG(TEXT("No reference image attached"));
		return;
	}

	// 요청 데이터 생성
	FInputPromptData Request;
	Request.PromptMessageText = PromptText;
	Request.ReferenceImageData = ReferenceImageData;
	Request.ImageFormat = ReferenceImageFormat;
	Request.UserID = GetWorld()->GetFirstPlayerController()->GetUniqueID();

	// Listen 서버로 RPC 호출
	SendToExternalServer(Request);

	// 임시 데이터 클리어
	ReferenceImageData.Empty();
	ReferenceImageFormat.Empty();
}

bool UMVE_AUD_CustomizationManager::LoadReferenceImage(const FString& FilePath)
{
	// 기존 데이터 클리어
	ReferenceImageData.Empty();
	ReferenceImageFileName.Empty();
	ReferenceImageFormat.Empty();

	// 파일을 바이트 배열로 읽기
	if (!FFileHelper::LoadFileToArray(ReferenceImageData, *FilePath))
	{
		PRINTLOG(TEXT("Failed to load file: %s"), *FilePath);
		return false;
	}

	// 파일 크기 제한 (10MB)
	const int32 MaxFileSizeBytes = 10 * 1024 * 1024;
	if (ReferenceImageData.Num() > MaxFileSizeBytes)
	{
		PRINTLOG(TEXT("File too large: %d bytes"), ReferenceImageData.Num());
		ReferenceImageData.Empty();
		return false;
	}

	// 파일명과 확장자 저장
	ReferenceImageFileName = FPaths::GetCleanFilename(FilePath);
	ReferenceImageFormat = FPaths::GetExtension(FilePath).ToLower();

	// 지원 포맷 확인
	if (ReferenceImageFormat != TEXT("png") && 
		ReferenceImageFormat != TEXT("jpg") && 
		ReferenceImageFormat != TEXT("jpeg") &&
		ReferenceImageFormat != TEXT("gif"))
	{
		PRINTLOG(TEXT("Unsupported format: %s"), *ReferenceImageFormat);
		ReferenceImageData.Empty();
		ReferenceImageFileName.Empty();
		ReferenceImageFormat.Empty();
		return false;
	}

	return true;
}

void UMVE_AUD_CustomizationManager::SendToExternalServer(const FInputPromptData& Request)
{
	// HTTP 모듈 가져오기
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

	// 요청 설정
	HttpRequest->SetURL(TEXT("https://your-ai-server.com/api/generate-model"));
	HttpRequest->SetVerb(TEXT("POST"));

	// Multipart form data 생성
	FString Boundary = TEXT("----UnrealBoundary") + FGuid::NewGuid().ToString();
	HttpRequest->SetHeader(TEXT("Content-Type"), 
		FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

	// Body 생성
	TArray<uint8> RequestBody;
    
	// 프롬프트 텍스트 추가
	FString PromptPart = FString::Printf(
		TEXT("--%s\r\nContent-Disposition: form-data; name=\"prompt\"\r\n\r\n%s\r\n"),
		*Boundary, *Request.PromptMessageText);
	RequestBody.Append((uint8*)TCHAR_TO_UTF8(*PromptPart), PromptPart.Len());

	// 이미지 파일 추가
	FString ImageHeader = FString::Printf(
		TEXT("--%s\r\nContent-Disposition: form-data; name=\"reference_image\"; filename=\"reference.%s\"\r\nContent-Type: image/%s\r\n\r\n"),
		*Boundary, *Request.ImageFormat, *Request.ImageFormat);
	RequestBody.Append((uint8*)TCHAR_TO_UTF8(*ImageHeader), ImageHeader.Len());
	RequestBody.Append(Request.ReferenceImageData);
	RequestBody.Append((uint8*)"\r\n", 2);

	// 종료 boundary
	FString EndBoundary = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
	RequestBody.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());

	HttpRequest->SetContent(RequestBody);

	// 응답 콜백
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UMVE_AUD_CustomizationManager::OnModelGenerationResponse);

	HttpRequest->ProcessRequest();
    
	PRINTLOG(TEXT("Model generation request sent to AI server"));
}

void UMVE_AUD_CustomizationManager::OnModelGenerationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		PRINTLOG(TEXT("Failed to connect to AI server"));
		return;
	}

	if (Response->GetResponseCode() == 200)
	{
		// JSON 응답 파싱
		FString ResponseContent = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ModelID = JsonObject->GetStringField(TEXT("model_id"));
			FString GLBFileURL = JsonObject->GetStringField(TEXT("glb_url"));
            
			PRINTLOG(TEXT("Model generation queued: %s"), *ModelID);
			PRINTLOG(TEXT("GLB URL: %s"), *GLBFileURL);
            
			// 생성 완료 처리
			OnModelGenerationComplete(ModelID, GLBFileURL);
		}
	}
	else
	{
		PRINTLOG(TEXT("AI server error: %d"), Response->GetResponseCode());
	}
}

void UMVE_AUD_CustomizationManager::OnModelGenerationComplete(const FString& ModelID, const FString& GLBFileURL)
{
	PRINTLOG(TEXT("Model generation complete: %s"), *ModelID);
	
	// GLB 파일 다운로드 및 로딩
	// glTFRuntime으로 런타임 로딩
	// 기존 캐릭터에 악세서리 부착
}
