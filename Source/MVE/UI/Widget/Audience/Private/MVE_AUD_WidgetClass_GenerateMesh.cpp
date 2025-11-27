
#include "../Public/MVE_AUD_WidgetClass_GenerateMesh.h"

#include "MVE.h"
#include "MVE_AUD_CustomizationManager.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"

void UMVE_AUD_WidgetClass_GenerateMesh::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendPromptButton)
	{
		SendPromptButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked);
	}

	if (EnterConcertRoomButton)
	{
		EnterConcertRoomButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnEnterConcertRoomButtonClicked);
	}

	if (InputImageButton)
	{
		InputImageButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnInputImageButtonClicked);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked()
{
	FString PromptText = PromptEditableBox->GetText().ToString();
    
	UMVE_AUD_CustomizationManager* CustomizationManager = 
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
    
	if (CustomizationManager)
	{
		CustomizationManager->RequestModelGeneration(PromptText);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnEnterConcertRoomButtonClicked()
{
	UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
    
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::AudienceConcertRoom);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnInputImageButtonClicked()
{
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
