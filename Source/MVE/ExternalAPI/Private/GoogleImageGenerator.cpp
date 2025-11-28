// Fill out your copyright notice in the Description page of Project Settings.

#include "ExternalAPI/Public/GoogleImageGenerator.h"

#include "HttpModule.h"
#include "MVE.h"
#include "Interfaces/IHttpResponse.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Materials/MaterialInstanceDynamic.h"

AGoogleImageGenerator::AGoogleImageGenerator()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AGoogleImageGenerator::GenerateImage(const FString PromptText)
{
	// 1. HTTP 요청 객체 생성
	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	// 2. URL 설정 (Imagen 3 모델 엔드포인트)
	// 문서에 나온 모델명: imagen-3.0-generate-001
	const FString Url = FString::Printf(TEXT("https://generativelanguage.googleapis.com/v1beta/models/imagen-4.0-generate-001:predict"));
	
	Request->SetURL(Url);
	Request->SetHeader("x-goog-api-key", *ApiKey);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetVerb("POST");

	// 3. JSON 데이터 구성 (문서의 구조를 따라감)
	// 구조: { "instances": [ { "prompt": "..." } ], "parameters": { "sampleCount": 1 } }
	
	// (1) 내부 프롬프트 객체
	TSharedPtr<FJsonObject> PromptObj = MakeShareable(new FJsonObject);
	PromptObj->SetStringField("prompt", PromptText);

	// (2) instances 배열 (구글 API는 보통 입력을 배열로 받음)
	TArray<TSharedPtr<FJsonValue>> InstancesArray;
	InstancesArray.Add(MakeShareable(new FJsonValueObject(PromptObj)));

	// (3) 파라미터 객체 (옵션: 장수, 비율 등)
	TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject);
	ParamsObj->SetNumberField("sampleCount", 1); // 1장 생성
	ParamsObj->SetStringField("aspectRatio", "1:1"); // 비율

	// (4) 최상위 루트 객체
	TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject);
	RootObj->SetArrayField("instances", InstancesArray);
	RootObj->SetObjectField("parameters", ParamsObj);

	// 4. JSON을 문자열(String)로 변환
	FString RequestContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContent);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestContent);

	// 5. 콜백 연결 및 전송
	Request->OnProcessRequestComplete().BindUObject(this, &AGoogleImageGenerator::OnResponseReceived);
	Request->ProcessRequest();
	
	PRINTLOG(TEXT("이미지 생성 요청 보냄: %s"), *PromptText);
}

void AGoogleImageGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. 첫 번째 스태틱 메시 컴포넌트 획득
	TargetMesh = FindComponentByClass<UStaticMeshComponent>();

	if (TargetMesh)
	{
		// 2. 스태틱 메시 컴포넌트의 첫 번째 머터리얼 획득
		if (UMaterialInterface* BaseMaterial = TargetMesh->GetMaterial(0))
		{
			// 3. MID 생성
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			
			// 4. MID 적용
			TargetMesh->SetMaterial(0, DynamicMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent를 찾을 수 없습니다!"));
	}
}

void AGoogleImageGenerator::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, const bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("요청 실패: 인터넷 연결 확인 필요"));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("API 오류 (%d): %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		return;
	}

	// 6. 응답 파싱
	TSharedPtr<FJsonObject> JsonResponse;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonResponse))
	{
		// 구조: { "predictions": [ { "bytesBase64Encoded": "..." } ] }
		const TArray<TSharedPtr<FJsonValue>>* PredictionsArray;
		if (JsonResponse->TryGetArrayField("predictions", PredictionsArray))
		{
			// 첫 번째 결과 가져오기
			TSharedPtr<FJsonObject> PredictionObj = (*PredictionsArray)[0]->AsObject();
			FString Base64ImageString;
            
			if (PredictionObj->TryGetStringField("bytesBase64Encoded", Base64ImageString))
			{
				UE_LOG(LogTemp, Log, TEXT("이미지 데이터 수신 성공! 길이: %d"), Base64ImageString.Len());
                
				// ★ 핵심: Base64를 텍스처로 변환
				if (UTexture2D* GeneratedTexture = LoadTextureFromBase64(Base64ImageString))
				{
					// ★ 핵심: 변환된 텍스처를 메쉬에 적용
					ApplyTextureToMesh(GeneratedTexture);
					UE_LOG(LogTemp, Log, TEXT("이미지 적용 완료!"));
				}
			}
		}
	}
}

UTexture2D* AGoogleImageGenerator::LoadTextureFromBase64(const FString& Base64String)
{
	// 1. Base64 문자열을 바이너리 배열(TArray<uint8>)로 디코딩
	TArray<uint8> DecodedData;
	FBase64::Decode(Base64String, DecodedData);

	if (DecodedData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Base64 디코딩 실패!"));
		return nullptr;
	}

	// 2. 언리얼 엔진의 이미지 유틸리티를 사용하여 바이너리 데이터를 텍스처로 변환
	// FImageUtils::ImportBufferAsTexture2D는 PNG, JPG 등의 포맷을 자동으로 인식합니다.
	UTexture2D* NewTexture = FImageUtils::ImportBufferAsTexture2D(DecodedData);

	if (NewTexture)
	{
		// 중요: 텍스처가 게임 월드에서 제대로 보이도록 설정을 업데이트합니다.
		NewTexture->SRGB = true; // 색상 공간 설정
		NewTexture->UpdateResource();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("바이너리 데이터를 텍스처로 변환하는데 실패했습니다."));
	}

	return NewTexture;
}

void AGoogleImageGenerator::ApplyTextureToMesh(UTexture2D* NewTexture)
{
	if (DynamicMaterial && NewTexture)
	{
		// 아까 에디터에서 만든 파라미터 이름 "DynamicTexture"에 새 텍스처를 연결합니다.
		DynamicMaterial->SetTextureParameterValue(FName("GeneratedTexture"), NewTexture);
	}
}
