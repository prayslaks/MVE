#include "UI/Widget/Audience/Public/MVE_AUD_WC_ApiPanel.h"
#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearch.h"
#include "API/Public/MVE_API_Helper.h"
#include "Kismet/GameplayStatics.h"
#include "MVE.h"

void UMVE_AUD_WC_ApiPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ConcertSearch)
	{
		ConcertSearch->OnConcertDoubleClicked.AddDynamic(this, &UMVE_AUD_WC_ApiPanel::HandleConcertDoubleClicked);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_AUD_WC_ApiPanel::HandleConcertDoubleClicked(const FMVE_AUD_ConcertSearchResultData& ConcertData)
{
	const FString RoomId = ConcertData.RoomId;
	FOnGetConcertInfoComplete OnResult;
	OnResult.BindLambda([this](const bool bSuccess, const FGetConcertInfoResponseData& ResponseData, const FString& ErrorCode)
	{
		if (ResponseData.Success)
		{
			const FString LocalIP = ResponseData.Concert.ListenServer.LocalIP;
			const int32 Port = ResponseData.Concert.ListenServer.Port;
			PRINTLOG(TEXT("콘서트 리슨 서버 정보: %s:%d"), *LocalIP, Port);

			// 리슨 서버에 접속
			if (const UWorld* World = GetWorld())
			{
				const FString ConnectionString = FString::Printf(TEXT("%s:%d"), *LocalIP, Port);
				PRINTLOG(TEXT("리슨 서버 접속 시도: %s"), *ConnectionString);
				UGameplayStatics::OpenLevel(World, FName(*ConnectionString), true);
			}
			else
			{
				PRINTLOG(TEXT("월드를 찾을 수 없습니다."));
			}
		}
		else
		{
			PRINTLOG(TEXT("콘서트 정보 가져오기 실패: %s"), *ErrorCode);
		}
	});
	UMVE_API_Helper::GetConcertInfo(RoomId, OnResult);
}
