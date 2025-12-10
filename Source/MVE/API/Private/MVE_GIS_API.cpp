// Fill out your copyright notice in the Description page of Project Settings.

#include "API/Public/MVE_GIS_API.h"

#define LOCTEXT_NAMESPACE "MVE_GIS_API_Errors"

void UMVE_GIS_API::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeErrorCodes();
}

FText UMVE_GIS_API::GetTranslatedErrorMessage(const FString& ErrorCode) const
{
	if (const FText* FoundText = ErrorCodeMap.Find(ErrorCode))
	{
		return *FoundText;
	}
	return FText::Format(LOCTEXT("UnknownError", "알 수 없는 오류가 발생했습니다. (코드: {0})"), FText::FromString(ErrorCode));
}

UMVE_GIS_API* UMVE_GIS_API::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
    
	return GameInstance->GetSubsystem<UMVE_GIS_API>();
}

void UMVE_GIS_API::InitializeErrorCodes()
{
	ErrorCodeMap.Empty();

	// 로그인 API 에러
	ErrorCodeMap.Emplace(TEXT("MISSING_EMAIL"), LOCTEXT("MISSING_EMAIL", "이메일 입력이 필요합니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_INPUT_TYPE"), LOCTEXT("INVALID_INPUT_TYPE", "입력값이 올바른 타입이 아닙니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_EMAIL_FORMAT"), LOCTEXT("INVALID_EMAIL_FORMAT", "유효하지 않은 이메일 형식입니다."));
	ErrorCodeMap.Emplace(TEXT("EMAIL_ALREADY_EXISTS"), LOCTEXT("EMAIL_ALREADY_EXISTS", "이메일이 이미 존재합니다."));
	ErrorCodeMap.Emplace(TEXT("DATABASE_ERROR"), LOCTEXT("DATABASE_ERROR", "데이터베이스 오류가 발생했습니다."));
	ErrorCodeMap.Emplace(TEXT("INTERNAL_SERVER_ERROR"), LOCTEXT("INTERNAL_SERVER_ERROR", "서버 내부 오류가 발생했습니다."));
	ErrorCodeMap.Emplace(TEXT("TOO_MANY_REQUESTS"), LOCTEXT("TOO_MANY_REQUESTS", "요청이 너무 많습니다. 잠시 후 다시 시도해주세요."));
	ErrorCodeMap.Emplace(TEXT("EMAIL_SEND_ERROR"), LOCTEXT("EMAIL_SEND_ERROR", "인증 이메일 전송에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("MISSING_FIELDS"), LOCTEXT("MISSING_FIELDS", "필수 항목이 누락되었습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_CODE_FORMAT"), LOCTEXT("INVALID_CODE_FORMAT", "인증번호 형식이 올바르지 않습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_CODE"), LOCTEXT("INVALID_CODE", "인증번호가 일치하지 않습니다."));
	ErrorCodeMap.Emplace(TEXT("CODE_NOT_FOUND"), LOCTEXT("CODE_NOT_FOUND", "요청된 인증번호를 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("CODE_EXPIRED"), LOCTEXT("CODE_EXPIRED", "인증번호가 만료되었습니다."));
	ErrorCodeMap.Emplace(TEXT("TOO_MANY_ATTEMPTS"), LOCTEXT("TOO_MANY_ATTEMPTS", "인증 시도 횟수를 초과했습니다. 새 코드를 요청하세요."));
	ErrorCodeMap.Emplace(TEXT("WEAK_PASSWORD"), LOCTEXT("WEAK_PASSWORD", "비밀번호는 6자 이상이어야 합니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_USERNAME_LENGTH"), LOCTEXT("INVALID_USERNAME_LENGTH", "사용자명은 3자에서 20자 사이여야 합니다."));
	ErrorCodeMap.Emplace(TEXT("USER_ALREADY_EXISTS"), LOCTEXT("USER_ALREADY_EXISTS", "이미 존재하는 사용자입니다."));
	ErrorCodeMap.Emplace(TEXT("DUPLICATE_ENTRY"), LOCTEXT("DUPLICATE_ENTRY", "중복된 항목입니다."));
	ErrorCodeMap.Emplace(TEXT("ENCRYPTION_ERROR"), LOCTEXT("ENCRYPTION_ERROR", "암호화 오류가 발생했습니다."));
	ErrorCodeMap.Emplace(TEXT("USER_NOT_FOUND"), LOCTEXT("USER_NOT_FOUND", "사용자를 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_PASSWORD"), LOCTEXT("INVALID_PASSWORD", "비밀번호가 일치하지 않습니다."));
	ErrorCodeMap.Emplace(TEXT("SERVER_CONFIG_ERROR"), LOCTEXT("SERVER_CONFIG_ERROR", "서버 설정 오류입니다."));
	ErrorCodeMap.Emplace(TEXT("TOKEN_GENERATION_ERROR"), LOCTEXT("TOKEN_GENERATION_ERROR", "토큰 생성에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("NO_AUTH_HEADER"), LOCTEXT("NO_AUTH_HEADER", "인증 헤더가 없습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_AUTH_FORMAT"), LOCTEXT("INVALID_AUTH_FORMAT", "인증 헤더 형식이 올바르지 않습니다. 'Bearer '로 시작해야 합니다."));
	ErrorCodeMap.Emplace(TEXT("NO_TOKEN"), LOCTEXT("NO_TOKEN", "토큰이 제공되지 않았습니다."));
	ErrorCodeMap.Emplace(TEXT("TOKEN_EXPIRED"), LOCTEXT("TOKEN_EXPIRED", "토큰이 만료되었습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_TOKEN"), LOCTEXT("INVALID_TOKEN", "유효하지 않은 토큰입니다."));
	ErrorCodeMap.Emplace(TEXT("MISSING_PASSWORD"), LOCTEXT("MISSING_PASSWORD", "계정 삭제를 위해 비밀번호가 필요합니다."));
	ErrorCodeMap.Emplace(TEXT("TOKEN_VERIFICATION_FAILED"), LOCTEXT("TOKEN_VERIFICATION_FAILED", "토큰 검증에 실패했습니다."));

	// 리소스 API 에러
	ErrorCodeMap.Emplace(TEXT("AUDIO_NOT_FOUND"), LOCTEXT("AUDIO_NOT_FOUND", "음원을 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("FILE_NOT_FOUND"), LOCTEXT("FILE_NOT_FOUND", "서버에서 파일을 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("STREAMING_ERROR"), LOCTEXT("STREAMING_ERROR", "스트리밍에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("NO_FILE"), LOCTEXT("NO_FILE", "제공된 파일이 없습니다."));
	ErrorCodeMap.Emplace(TEXT("MISSING_TITLE"), LOCTEXT("MISSING_TITLE", "제목이 필요합니다."));
	ErrorCodeMap.Emplace(TEXT("UPLOAD_ERROR"), LOCTEXT("UPLOAD_ERROR", "업로드에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_ACCESSORIES"), LOCTEXT("INVALID_ACCESSORIES", "액세서리 정보가 올바르지 않습니다."));
	ErrorCodeMap.Emplace(TEXT("DUPLICATE_PRESET_NAME"), LOCTEXT("DUPLICATE_PRESET_NAME", "이미 사용 중인 프리셋 이름입니다."));
	ErrorCodeMap.Emplace(TEXT("SAVE_FAILED"), LOCTEXT("SAVE_FAILED", "저장에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("LIST_FAILED"), LOCTEXT("LIST_FAILED", "목록을 불러오는데 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("INVALID_ID"), LOCTEXT("INVALID_ID", "유효하지 않은 ID입니다."));
	ErrorCodeMap.Emplace(TEXT("PRESET_NOT_FOUND"), LOCTEXT("PRESET_NOT_FOUND", "프리셋을 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("ACCESS_DENIED"), LOCTEXT("ACCESS_DENIED", "접근 권한이 없습니다."));
	ErrorCodeMap.Emplace(TEXT("DUPLICATE_MODEL_NAME"), LOCTEXT("DUPLICATE_MODEL_NAME", "이미 사용 중인 모델 이름입니다."));
	ErrorCodeMap.Emplace(TEXT("NO_UPDATE_FIELDS"), LOCTEXT("NO_UPDATE_FIELDS", "업데이트할 항목이 없습니다."));
	ErrorCodeMap.Emplace(TEXT("MODEL_NOT_FOUND"), LOCTEXT("MODEL_NOT_FOUND", "모델을 찾을 수 없거나 접근 권한이 없습니다."));
	ErrorCodeMap.Emplace(TEXT("CREATE_FAILED"), LOCTEXT("CREATE_FAILED", "생성에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("MISSING_CLIENT_ID"), LOCTEXT("MISSING_CLIENT_ID", "클라이언트 ID가 필요합니다."));
	ErrorCodeMap.Emplace(TEXT("JOIN_FAILED"), LOCTEXT("JOIN_FAILED", "참가에 실패했습니다."));
	ErrorCodeMap.Emplace(TEXT("PERMISSION_DENIED"), LOCTEXT("PERMISSION_DENIED", "권한이 없습니다."));
	ErrorCodeMap.Emplace(TEXT("CONCERT_NOT_FOUND"), LOCTEXT("CONCERT_NOT_FOUND", "콘서트를 찾을 수 없습니다."));
	ErrorCodeMap.Emplace(TEXT("ADD_ACCESSORY_FAILED"), LOCTEXT("ADD_ACCESSORY_FAILED", "액세서리 추가에 실패했습니다."));
}

#undef LOCTEXT_NAMESPACE