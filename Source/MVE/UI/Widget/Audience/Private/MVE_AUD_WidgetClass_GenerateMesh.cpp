
#include "../Public/MVE_AUD_WidgetClass_GenerateMesh.h"

#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"

void UMVE_AUD_WidgetClass_GenerateMesh::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendPromptButton)
	{
		SendPromptButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked);
	}
}

void UMVE_AUD_WidgetClass_GenerateMesh::OnSendPromptButtonClicked()
{
	// 프롬프트 보내기
	FText TextContent = PromptEditableBox->GetText();
	FString StringContent = TextContent.ToString();
	
}
