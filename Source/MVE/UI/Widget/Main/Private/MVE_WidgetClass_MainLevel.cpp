
#include "../Public/MVE_WidgetClass_MainLevel.h"

#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMVE_WidgetClass_MainLevel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (StdBypassButton)
	{
		StdBypassButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel::OnStdBypassButtonClicked);
	}
	
	if (AudBypassButton)
	{
		AudBypassButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel::OnAudBypassButtonClicked);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel::OnStdBypassButtonClicked()
{
	PRINTLOG(TEXT("OnCreditButtonClicked Called"));
	UGameplayStatics::OpenLevel(GetWorld(), FName("STD_API_TEST"), true, TEXT("listen"));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel::OnAudBypassButtonClicked()
{
	PRINTLOG(TEXT("OnCreditButtonClicked Called"));
	UGameplayStatics::OpenLevel(GetWorld(), FName("AUD_API_TEST"), true);
}

