
#include "MVE_GS_StageLevel.h"
#include "MVE.h"
#include "Net/UnrealNetwork.h"

AMVE_GS_StageLevel::AMVE_GS_StageLevel()
{
	ViewerCount = 0;
	bReplicates = true;
}

void AMVE_GS_StageLevel::SetViewerCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		PRINTLOG(TEXT("SetViewerCount can only be called on server!"));
		return;
	}

	if (ViewerCount != NewCount)
	{
		ViewerCount = NewCount;
		PRINTLOG(TEXT("ViewerCount updated to: %d"), ViewerCount);

		// Broadcast on server immediately
		OnViewerCountChanged.Broadcast(ViewerCount);
	}
}

void AMVE_GS_StageLevel::OnRep_ViewerCount()
{
	PRINTLOG(TEXT("OnRep_ViewerCount: %d"), ViewerCount);

	// Broadcast to widgets on clients
	OnViewerCountChanged.Broadcast(ViewerCount);
}

void AMVE_GS_StageLevel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMVE_GS_StageLevel, ViewerCount);
}
