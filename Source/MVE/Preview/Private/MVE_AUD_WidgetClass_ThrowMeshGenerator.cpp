#include "../Public/MVE_AUD_WidgetClass_ThrowMeshGenerator.h"

#include <MVE_API_Helper.h>

#include "MVE.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_AUD_WidgetClass_PreviewWidget.h"
#include "MVE_GIS_SessionManager.h"
#include "MVE_HTTP_Client.h"
#include "SenderReceiver.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/StaticMesh.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendPromptButton)
	{
		SendPromptButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnSendPromptButtonClicked);
	}

	if (SaveButton)
	{
		SaveButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnSaveButtonClicked);
	}

	if (InputImageButton)
	{
		InputImageButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnInputImageButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnCloseButtonClicked);
	}

	if (USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>())
	{
		SR->OnAssetLoaded.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleAssetLoaded);
		SR->OnGenerationResponse.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleGenerationResponse);
		SR->OnDownloadProgress.AddDynamic(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleDownloadProgress);

		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 송수신 델리게이트 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] USenderReceiver를 찾을 수 없습니다"));
	}

	// 초기 상태
	SetStatus(TEXT("던질 메시의 프롬프트를 입력하세요"));
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnSendPromptButtonClicked()
{
	UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 전송 버튼 클릭"));

	// CustomizationManager에서 이미지 경로 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager =
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (!CustomizationManager)
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] CustomizationManager가 없습니다"));
		SetStatus(TEXT("시스템 오류"));
		return;
	}

	// 테스트 모드: 중계 서버에서 model id로 presigned URL 받아오기
	if (bTestMode)
	{
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 테스트 모드: Model ID %d로 presigned URL 요청"), TestModelId);
		SetStatus(FString::Printf(TEXT("Model ID %d의 다운로드 URL 요청 중..."), TestModelId));
		SetButtonsEnabled(false);

		// GetModelDownloadUrl API 호출
		FOnGetModelDownloadUrlComplete OnComplete;
		OnComplete.BindUObject(this, &UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleGetModelDownloadUrl);
		UMVE_API_Helper::GetModelDownloadUrl(TestModelId, OnComplete);
	}
	else
	{
		// 입력값 검증
		if (!PromptEditableBox)
		{
			UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] PromptEditableBox가 없습니다"));
			return;
		}

		FString PromptText = PromptEditableBox->GetText().ToString();

		if (PromptText.IsEmpty())
		{
			SetStatus(TEXT("파일을 넣고 프롬프트를 입력해주세요"));
			return;
		}

		// 참조 이미지 파일 경로 가져오기
		FString ImagePath = CustomizationManager->GetReferenceImageFilePath();
		if (ImagePath.IsEmpty())
		{
			SetStatus(TEXT("참조 이미지를 첨부해주세요"));
			return;
		}

		// UI 상태 업데이트
		SetStatus(TEXT("서버에 요청 중..."));
		SetButtonsEnabled(false);

		// 서버에 전송
		CustomizationManager->RequestModelGeneration(PromptText, ImagePath);
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnInputImageButtonClicked()
{
	UMVE_API_Helper::GetAuthToken();

	UMVE_AUD_CustomizationManager* CustomizationManager =
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (CustomizationManager)
	{
		FString LoadedFileName = CustomizationManager->OpenReferenceImageDialog();

		if (!LoadedFileName.IsEmpty() && ImportedImageNameTextBlock)
		{
			ImportedImageNameTextBlock->SetText(FText::FromString(LoadedFileName));
		}
		else if (ImportedImageNameTextBlock)
		{
			ImportedImageNameTextBlock->SetText(FText::FromString(TEXT("로드 실패")));
		}
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnCloseButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::OnSaveButtonClicked()
{
	if (UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>())
	{
		// 던지기 메시 저장 (SocketName = "THROW_MESH")
		if (!LastReceivedMetadata.RemotePath.IsEmpty())
		{
			CustomizationManager->SaveThrowMeshPreset(LastReceivedMetadata.RemotePath);
			SetStatus(TEXT("던지기 메시가 서버에 저장되었습니다"));
		}
		else
		{
			SetStatus(TEXT("저장할 메시가 없습니다"));
		}
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes)
{
	if (TotalBytes > 0)
	{
		float Progress = (float)BytesReceived / (float)TotalBytes * 100.0f;
		UE_LOG(LogMVE, Verbose, TEXT("[ThrowMeshGenerator] 다운로드: %.1f%% (%d/%d)"),
			Progress, BytesReceived, TotalBytes);
	}
	else
	{
		SetStatus(FString::Printf(TEXT("No data!")));
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata,
	const FString& ErrorMessage)
{
	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] 송수신 시작"));

	if (bSuccess)
	{
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 생성 요청 성공"));
		UE_LOG(LogMVE, Log, TEXT("  - DisplayName: %s"), *Metadata.DisplayName);
		UE_LOG(LogMVE, Log, TEXT("  - RemotePath: %s"), *Metadata.RemotePath);

		// 상태 업데이트 (다운로드는 SenderReceiver에서 자동 시작됨)
		SetStatus(TEXT("파일 다운로드 중..."));
	}
	else
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] 생성 요청 실패: %s"), *ErrorMessage);

		SetStatus(FString::Printf(TEXT(" 실패: %s"), *ErrorMessage));
		SetButtonsEnabled(true);
	}

	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] 송수신 완료"));
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::Download()
{
	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] ========================================"));
	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] 테스트 5: 파일 다운로드"));
	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] ========================================"));

	USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
	if (!SR)
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] 서브시스템 없음"));
		return;
	}

	// 메타데이터 구성
	FAssetMetadata Metadata;
	Metadata.AssetType = EAssetType::IMAGE;
	Metadata.DisplayName = TEXT("Mock 서버 이미지");
	Metadata.RemotePath = TestServerURL + TEXT("/api/download/image");
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleAssetLoaded(UObject* Asset, const FAssetMetadata& Metadata)
{
	if (!Asset)
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] Asset이 null입니다"));
		SetStatus(TEXT("에셋 로드 실패"));
		SetButtonsEnabled(true);
		return;
	}

	UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] 에셋 수신 완료"));
	UE_LOG(LogMVE, Log, TEXT("  - 디스플레이: %s"), *Metadata.DisplayName);
	UE_LOG(LogMVE, Log, TEXT("  - 경로: %s"), *Metadata.LocalPath);
	UE_LOG(LogMVE, Log, TEXT("  - 타입: %s"), *Asset->GetClass()->GetName());

	// 버튼 다시 활성화
	SetButtonsEnabled(true);

	// 타입별 처리
	if (USkeletalMesh* SkMesh = Cast<USkeletalMesh>(Asset))
	{
		// SkeletalMesh 수신 (본 있는 메시)
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] SkeletalMesh 수신"));
		UE_LOG(LogMVE, Log, TEXT("  - Bones: %d"), SkMesh->GetRefSkeleton().GetNum());

		// 메타데이터 저장
		LastReceivedMetadata = Metadata;
		LastReceivedMesh = SkMesh;

		// 상태 업데이트
		SetStatus(FString::Printf(TEXT("%s 생성 완료! (SkeletalMesh)"), *Metadata.DisplayName));

		// 던지기 메시 프리뷰 위젯에 메시 적용
		if (MeshPreviewWidget)
		{
			UMVE_AUD_CustomizationManager* CustomizationManager =
				GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

			if (CustomizationManager)
			{
				UMVE_AUD_WidgetClass_PreviewWidget* PreviewWidget =
					Cast<UMVE_AUD_WidgetClass_PreviewWidget>(MeshPreviewWidget);

				if (PreviewWidget)
				{
					FString LocalPath = Metadata.LocalPath;

					if (!LocalPath.IsEmpty())
					{
						// 던지기 전용 프리뷰 시작
						CustomizationManager->StartThrowMeshPreview(LocalPath, PreviewWidget);
						UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 던지기 메시 프리뷰 적용 완료 - 경로: %s"), *LocalPath);

						// Presigned URL 별도 저장
						CustomizationManager->SetRemoteModelUrl(Metadata.RemotePath);
					}
					else
					{
						UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] LocalPath가 비어있어 프리뷰를 시작할 수 없습니다."));
						SetStatus(TEXT("로컬 파일 경로 오류"));
					}
				}
				else
				{
					UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] MeshPreviewWidget 캐스팅 실패"));
				}
			}
			else
			{
				UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] CustomizationManager 없음"));
			}
		}
	}
	else if (UStaticMesh* StMesh = Cast<UStaticMesh>(Asset))
	{
		// StaticMesh 수신 (본 없는 메시)
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] StaticMesh 수신"));
		UE_LOG(LogMVE, Log, TEXT("  - Vertices: %d"), StMesh->GetNumVertices(0));
		UE_LOG(LogMVE, Log, TEXT("  - Triangles: %d"), StMesh->GetNumTriangles(0));

		// 메타데이터 저장
		LastReceivedMetadata = Metadata;
		LastReceivedMesh = StMesh;

		// 상태 업데이트
		SetStatus(FString::Printf(TEXT("%s 생성 완료! (StaticMesh)"), *Metadata.DisplayName));

		// 던지기 메시 프리뷰 위젯에 메시 적용
		if (MeshPreviewWidget)
		{
			UMVE_AUD_CustomizationManager* CustomizationManager =
				GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

			if (CustomizationManager)
			{
				UMVE_AUD_WidgetClass_PreviewWidget* PreviewWidget =
					Cast<UMVE_AUD_WidgetClass_PreviewWidget>(MeshPreviewWidget);

				if (PreviewWidget)
				{
					FString LocalPath = Metadata.LocalPath;

					if (!LocalPath.IsEmpty())
					{
						// 던지기 전용 프리뷰 시작
						CustomizationManager->StartThrowMeshPreview(LocalPath, PreviewWidget);
						UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 던지기 메시 프리뷰 적용 완료 - 경로: %s"), *LocalPath);
					}
					else
					{
						UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] LocalPath가 비어있어 프리뷰를 시작할 수 없습니다."));
						SetStatus(TEXT("로컬 파일 경로 오류"));
					}
				}
				else
				{
					UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] MeshPreviewWidget 캐스팅 실패"));
				}
			}
			else
			{
				UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] CustomizationManager 없음"));
			}
		}
	}
	else if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
	{
		// 이미지 수신
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 이미지 수신: %dx%d"),
			Texture->GetSizeX(), Texture->GetSizeY());
		SetStatus(TEXT("이미지 생성 완료!"));
	}
	else
	{
		UE_LOG(LogMVE, Warning, TEXT("[ThrowMeshGenerator] 알 수 없는 에셋 타입: %s"),
			Asset ? *Asset->GetClass()->GetName() : TEXT("null"));
		SetStatus(TEXT("알 수 없는 에셋 타입"));
	}
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::SetStatus(const FString& Message)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
	UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] 상태: %s"), *Message);
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::SetButtonsEnabled(bool bEnabled)
{
	if (SendPromptButton)SendPromptButton->SetIsEnabled(bEnabled);
	if (InputImageButton)InputImageButton->SetIsEnabled(bEnabled);
}

void UMVE_AUD_WidgetClass_ThrowMeshGenerator::HandleGetModelDownloadUrl(bool bSuccess, const FGetModelDownloadUrlResponseData& Data, const FString& ErrorCode)
{
	UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] HandleGetModelDownloadUrl called"));

	if (!bSuccess)
	{
		UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] Failed to get download URL: %s"), *ErrorCode);
		SetStatus(FString::Printf(TEXT("다운로드 URL 요청 실패: %s"), *ErrorCode));
		SetButtonsEnabled(true);
		return;
	}

	UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] Download URL received: %s"), *Data.Url);
	SetStatus(TEXT("파일 다운로드 중..."));

	// 다운로드 경로 설정
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("DownloadedModels");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SaveDir))
	{
		PlatformFile.CreateDirectoryTree(*SaveDir);
		UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] Created directory: %s"), *SaveDir);
	}

	FString SavePath = SaveDir / FString::Printf(TEXT("TestModel_%d.glb"), TestModelId);

	UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] Downloading to: %s"), *SavePath);

	// presigned URL로 다운로드
	FOnHttpDownloadResult OnDownloadComplete;
	OnDownloadComplete.BindLambda([this, SavePath, PresignedURL = Data.Url](bool bDownloadSuccess, const TArray<uint8>& FileData, const FString& ErrorMessage)
	{
		if (bDownloadSuccess && FileData.Num() > 0)
		{
			// 파일 저장
			if (FFileHelper::SaveArrayToFile(FileData, *SavePath))
			{
				UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] ✅ Model downloaded successfully!"));
				UE_LOG(LogMVE, Log, TEXT("   File path: %s"), *SavePath);
				UE_LOG(LogMVE, Log, TEXT("   File size: %.2f MB"), FileData.Num() / (1024.0f * 1024.0f));

				SetStatus(TEXT("모델 다운로드 완료! 프리뷰 시작 중..."));

				// CustomizationManager에 RemoteURL 저장
				UMVE_AUD_CustomizationManager* CustomizationManager =
					GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

				if (CustomizationManager)
				{
					CustomizationManager->SetRemoteModelUrl(PresignedURL);
					UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] Remote URL saved: %s"), *PresignedURL);

					// ⭐ LastReceivedMetadata 설정 (Save 버튼용)
					LastReceivedMetadata.RemotePath = PresignedURL;
					LastReceivedMetadata.LocalPath = SavePath;
					UE_LOG(LogMVE, Log, TEXT("[ThrowMeshGenerator] LastReceivedMetadata.RemotePath set for Save button"));

					// 던지기 메시 프리뷰 시작
					if (MeshPreviewWidget)
					{
						UMVE_AUD_WidgetClass_PreviewWidget* PreviewWidget =
							Cast<UMVE_AUD_WidgetClass_PreviewWidget>(MeshPreviewWidget);

						if (PreviewWidget)
						{
							CustomizationManager->StartThrowMeshPreview(SavePath, PreviewWidget);
							SetStatus(TEXT("테스트 모드: 던지기 메시 프리뷰 완료! Save 버튼 클릭 가능"));
							SetButtonsEnabled(true);
						}
					}
				}
			}
			else
			{
				UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] Failed to save file: %s"), *SavePath);
				SetStatus(TEXT("파일 저장 실패"));
				SetButtonsEnabled(true);
			}
		}
		else
		{
			UE_LOG(LogMVE, Error, TEXT("[ThrowMeshGenerator] Model download failed: %s"), *ErrorMessage);
			SetStatus(FString::Printf(TEXT("다운로드 실패: %s"), *ErrorMessage));
			SetButtonsEnabled(true);
		}
	});

	// S3 presigned URL은 Authorization 헤더 불필요
	FMVE_HTTP_Client::DownloadFile(Data.Url, TEXT(""), OnDownloadComplete);
}
