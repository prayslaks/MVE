#include "../Public/MVE_AUD_WidgetClass_GenerateMesh.h"

#include <MVE_API_Helper.h>

#include "MVE.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_AUD_WidgetClass_PreviewWidget.h"
#include "MVE_GIS_SessionManager.h"
#include "SenderReceiver.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/StaticMesh.h"

void UMVE_AUD_WidgetClass_GenerateMesh::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendPromptButton)
	{
		SendPromptButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked);
	}

	if (InputImageButton)
	{
		InputImageButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnInputImageButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnCloseButtonClicked);
	}

	if (HeadButton)
		HeadButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnHeadButtonClicked);

	if (LeftHandButton)
		LeftHandButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnLeftHandButtonClicked);

	if (RightHandButton)
		RightHandButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnRightHandButtonClicked);

	if (USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>())
	{
		SR->OnAssetLoaded.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::HandleAssetLoaded);
		SR->OnGenerationResponse.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::HandleGenerationResponse);
		SR->OnDownloadProgress.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::HandleDownloadProgress);
        
		UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 송수신 델리게이트 바인딩 완료"));
	}
	else
	{
		UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] USenderReceiver를 찾을 수 없습니다"));
	}

	// 초기 상태
	SetStatus(TEXT("프롬프트를 입력하세요"));
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked()
{
	UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 전송 버튼 클릭"));

	// CustomizationManager에서 이미지 경로 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager =
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
	
	if (!CustomizationManager)
	{
		UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] CustomizationManager가 없습니다"));
		SetStatus(TEXT("시스템 오류"));
		return;
	}
	
	if (bTestMode)
	{
		FString ContentDir = FPaths::ProjectContentDir();
		FString TestGLBPath = TEXT("Assets/Test/fbx.glb");
		FString FullPath = FPaths::Combine(ContentDir, TestGLBPath);
		UMVE_AUD_WidgetClass_PreviewWidget* PreviewWidget = Cast<UMVE_AUD_WidgetClass_PreviewWidget>(MeshPreviewWidget);
		CustomizationManager->StartMeshPreview(FullPath, PreviewWidget);
	}
	else
	{
		// 입력값 검증
		if (!PromptEditableBox)
		{
			UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] PromptEditableBox가 없습니다"));
			return;
		}

		FString PromptText = PromptEditableBox->GetText().ToString();
	
		if (PromptText.IsEmpty())
		{
			SetStatus(TEXT("파일을 넣고 프롬프틑 입력해주세요"));
			return;
		}
		
		// 참조 이미지 경로 (없으면 빈 문자열)
		FString ImagePath = CustomizationManager->GetReferenceImageDataAsBase64();
		if (ImagePath.IsEmpty())
		{
			SetStatus(TEXT("참조 이미지를 첨부해주세요"));
			return;
		}

		// UI 상태 업데이트
		SetStatus(TEXT("서버에 요청 중..."));
		SetButtonsEnabled(false);

		// 서버에 전송
		USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
		SR->SendGenerationRequest(PromptText, TEXT("woals1375@naver.com"), ImagePath);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnInputImageButtonClicked()
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

void UMVE_AUD_WidgetClass_GenerateMesh::OnCloseButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnRightHandButtonClicked()
{
	UMVE_AUD_CustomizationManager* Manager = 
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
    
	if (Manager)
	{
		Manager->AttachMeshToSocket(FName("RightHand"));
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnLeftHandButtonClicked()
{
	UMVE_AUD_CustomizationManager* Manager = 
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
    
	if (Manager)
	{
		Manager->AttachMeshToSocket(FName("LeftHand"));
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnHeadButtonClicked()
{
	UMVE_AUD_CustomizationManager* Manager = 
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
    
	if (Manager)
	{
		Manager->AttachMeshToSocket(FName("Head"));
		SetStatus(TEXT("머리에 부착 완료"));
	}
	else
	{
		SetStatus(TEXT("메시 설정 하세요"));
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes)
{
	if (TotalBytes > 0)
	{
		float Progress = (float)BytesReceived / (float)TotalBytes * 100.0f;
		UE_LOG(LogMVE, Verbose, TEXT("[Test] 다운로드: %.1f%% (%d/%d)"),
			Progress, BytesReceived, TotalBytes);
	}
	else
	{
		SetStatus(FString::Printf(TEXT("No data!")));
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata,
	const FString& ErrorMessage)
{
	UE_LOG(LogMVE, Warning, TEXT("[Test] 송수신 시작"));
    
	if (bSuccess)
	{
		UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 생성 요청 성공"));
		UE_LOG(LogMVE, Log, TEXT("  - DisplayName: %s"), *Metadata.DisplayName);
		UE_LOG(LogMVE, Log, TEXT("  - RemotePath: %s"), *Metadata.RemotePath);
        
		// 상태 업데이트 (다운로드는 SenderReceiver에서 자동 시작됨)
		SetStatus(TEXT("파일 다운로드 중..."));
	}
	else
	{
		UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] 생성 요청 실패: %s"), *ErrorMessage);
        
		SetStatus(FString::Printf(TEXT(" 실패: %s"), *ErrorMessage));
		SetButtonsEnabled(true);
	}
    
	UE_LOG(LogMVE, Warning, TEXT("[Test] 송수신 완료"));
}

void UMVE_AUD_WidgetClass_GenerateMesh::Download()
{
	UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
	UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 5: 파일 다운로드"));
	UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

	USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
	if (!SR)
	{
		UE_LOG(LogMVE, Error, TEXT("[Test] 서브시스템 없음"));
		return;
	}

	// 메타데이터 구성
	FAssetMetadata Metadata;
	Metadata.AssetType = EAssetType::IMAGE;
	Metadata.DisplayName = TEXT("Mock 서버 이미지");
	Metadata.RemotePath = TestServerURL + TEXT("/api/download/image");
}

void UMVE_AUD_WidgetClass_GenerateMesh::HandleAssetLoaded(UObject* Asset, const FAssetMetadata& Metadata)
{
	if (!Asset)
	{
		UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] Asset이 null입니다"));
		SetStatus(TEXT("에셋 로드 실패"));
		SetButtonsEnabled(true);
		return;
	}

	UE_LOG(LogMVE, Warning, TEXT("[GenerateMesh] 에셋 수신 완료"));
	UE_LOG(LogMVE, Log, TEXT("  - 디스플레이: %s"), *Metadata.DisplayName);
	UE_LOG(LogMVE, Log, TEXT("  - 경로: %s"), *Metadata.LocalPath);
	UE_LOG(LogMVE, Log, TEXT("  - 타입: %s"), *Asset->GetClass()->GetName());
	
	// 버튼 다시 활성화
	SetButtonsEnabled(true);

	// 타입별 처리
	if (USkeletalMesh* SkMesh = Cast<USkeletalMesh>(Asset))
	{
		// SkeletalMesh 수신 (본 있는 메시)
		UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] SkeletalMesh 수신"));
		UE_LOG(LogMVE, Log, TEXT("  - Bones: %d"), SkMesh->GetRefSkeleton().GetNum());
		
		// 메타데이터 저장 (소켓 부착 시 사용)
		LastReceivedMetadata = Metadata;
		LastReceivedMesh = SkMesh;
		
		// 상태 업데이트
		SetStatus(FString::Printf(TEXT("%s 생성 완료! (SkeletalMesh)"), *Metadata.DisplayName));

		// 프리뷰 위젯에 메시 적용
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
						CustomizationManager->StartMeshPreview(LocalPath, PreviewWidget);
						UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 프리뷰 적용 완료 - 경로: %s"), *LocalPath);
					}
					else
					{
						UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] LocalPath가 비어있어 프리뷰를 시작할 수 없습니다."));
						SetStatus(TEXT("로컬 파일 경로 오류"));
					}
				}
				else
				{
					UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] MeshPreviewWidget 캐스팅 실패"));
				}
			}
			else
			{
				UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] CustomizationManager 없음"));
			}
		}
	}
	else if (UStaticMesh* StMesh = Cast<UStaticMesh>(Asset))
	{
		// StaticMesh 수신 (본 없는 메시)
		UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] StaticMesh 수신"));
		UE_LOG(LogMVE, Log, TEXT("  - Vertices: %d"), StMesh->GetNumVertices(0));
		UE_LOG(LogMVE, Log, TEXT("  - Triangles: %d"), StMesh->GetNumTriangles(0));
		
		// 메타데이터 저장
		LastReceivedMetadata = Metadata;
		LastReceivedMesh = StMesh;
		
		// 상태 업데이트
		SetStatus(FString::Printf(TEXT("%s 생성 완료! (StaticMesh)"), *Metadata.DisplayName));

		// 프리뷰 위젯에 메시 적용
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
						CustomizationManager->StartMeshPreview(LocalPath, PreviewWidget);
						UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 프리뷰 적용 완료 - 경로: %s"), *LocalPath);
					}
					else
					{
						UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] LocalPath가 비어있어 프리뷰를 시작할 수 없습니다."));
						SetStatus(TEXT("로컬 파일 경로 오류"));
					}
				}
				else
				{
					UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] MeshPreviewWidget 캐스팅 실패"));
				}
			}
			else
			{
				UE_LOG(LogMVE, Error, TEXT("[GenerateMesh] CustomizationManager 없음"));
			}
		}
	}
	else if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
	{
		// 이미지 수신
		UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 이미지 수신: %dx%d"),
			Texture->GetSizeX(), Texture->GetSizeY());
		SetStatus(TEXT("이미지 생성 완료!"));
	}
	else
	{
		UE_LOG(LogMVE, Warning, TEXT("[GenerateMesh] 알 수 없는 에셋 타입: %s"), 
			Asset ? *Asset->GetClass()->GetName() : TEXT("null"));
		SetStatus(TEXT("알 수 없는 에셋 타입"));
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::SetStatus(const FString& Message)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
	UE_LOG(LogMVE, Log, TEXT("[GenerateMesh] 상태: %s"), *Message);
}

void UMVE_AUD_WidgetClass_GenerateMesh::SetButtonsEnabled(bool bEnabled)
{
	if (SendPromptButton)SendPromptButton->SetIsEnabled(bEnabled);
	if (InputImageButton)InputImageButton->SetIsEnabled(bEnabled);
}