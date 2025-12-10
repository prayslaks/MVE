// Fill out your copyright notice in the Description page of Project Settings.

#include "API/Public/MVE_GIS_API.h"

#define LOCTEXT_NAMESPACE "MVE_GIS_API_CODE"

void UMVE_GIS_API::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MapResponseCodeToText();
}

FText UMVE_GIS_API::GetTranslatedErrorMessage(const FString& ErrorCode) const
{
	if (const FText* FoundText = ResponseCodeToKoreanTextMap.Find(ErrorCode))
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

void UMVE_GIS_API::MapResponseCodeToText()
{
	ResponseCodeToKoreanTextMap.Empty();

    // --- 공통 응답 코드 ---
    ResponseCodeToKoreanTextMap.Emplace(TEXT("SUCCESS"), LOCTEXT("SUCCESS", "요청이 성공적으로 처리되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INTERNAL_SERVER_ERROR"), LOCTEXT("INTERNAL_SERVER_ERROR", "서버에 문제가 발생했습니다. 잠시 후 다시 시도해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DATABASE_ERROR"), LOCTEXT("DATABASE_ERROR", "데이터베이스 처리 중 오류가 발생했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_INPUT_TYPE"), LOCTEXT("INVALID_INPUT_TYPE", "입력값이 올바르지 않습니다. 다시 확인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_FIELDS"), LOCTEXT("MISSING_FIELDS", "필수 입력 항목이 누락되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("SERVER_CONFIG_ERROR"), LOCTEXT("SERVER_CONFIG_ERROR", "서버 설정에 문제가 발생했습니다. 관리자에게 문의하세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("NO_AUTH_HEADER"), LOCTEXT("NO_AUTH_HEADER", "인증 정보가 없습니다. 다시 로그인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_AUTH_FORMAT"), LOCTEXT("INVALID_AUTH_FORMAT", "인증 형식이 올바르지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("NO_TOKEN"), LOCTEXT("NO_TOKEN", "로그인 토큰이 없습니다. 다시 로그인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOKEN_EXPIRED"), LOCTEXT("TOKEN_EXPIRED", "로그인 세션이 만료되었습니다. 다시 로그인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_TOKEN"), LOCTEXT("INVALID_TOKEN", "유효하지 않은 로그인 토큰입니다. 다시 로그인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOKEN_VERIFICATION_FAILED"), LOCTEXT("TOKEN_VERIFICATION_FAILED", "토큰 검증에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("REDIS_UNAVAILABLE"), LOCTEXT("REDIS_UNAVAILABLE", "캐시 서버에 접속할 수 없습니다. 일시적인 문제일 수 있습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("HEALTH_CHECK_OK"), LOCTEXT("HEALTH_CHECK_OK", "서버 상태가 정상입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("HEALTH_CHECK_FAILED"), LOCTEXT("HEALTH_CHECK_FAILED", "서버 상태가 좋지 않습니다. 잠시 후 다시 시도해 주세요."));

    // --- 로그인 서버 응답 코드 ---
    ResponseCodeToKoreanTextMap.Emplace(TEXT("LOGIN_SUCCESS"), LOCTEXT("LOGIN_SUCCESS", "로그인에 성공했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("LOGOUT_SUCCESS"), LOCTEXT("LOGOUT_SUCCESS", "로그아웃되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_EMAIL"), LOCTEXT("MISSING_EMAIL", "이메일 주소를 입력해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_EMAIL_FORMAT"), LOCTEXT("INVALID_EMAIL_FORMAT", "올바른 이메일 형식이 아닙니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("EMAIL_ALREADY_EXISTS"), LOCTEXT("EMAIL_ALREADY_EXISTS", "이미 가입된 이메일 주소입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOO_MANY_REQUESTS"), LOCTEXT("TOO_MANY_REQUESTS", "요청 횟수가 너무 많습니다. 잠시 후 다시 시도해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("VERIFICATION_CODE_SENT"), LOCTEXT("VERIFICATION_CODE_SENT", "인증 코드가 이메일로 전송되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("EMAIL_SEND_ERROR"), LOCTEXT("EMAIL_SEND_ERROR", "인증 코드 이메일을 보내는 데 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_CODE_FORMAT"), LOCTEXT("INVALID_CODE_FORMAT", "인증 코드 형식이 올바르지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("CODE_NOT_FOUND"), LOCTEXT("CODE_NOT_FOUND", "인증 코드를 찾을 수 없습니다. 다시 요청해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_CODE"), LOCTEXT("INVALID_CODE", "인증 코드가 일치하지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("CODE_EXPIRED"), LOCTEXT("CODE_EXPIRED", "인증 코드가 만료되었습니다. 다시 요청해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOO_MANY_ATTEMPTS"), LOCTEXT("TOO_MANY_ATTEMPTS", "인증 시도 횟수를 초과했습니다. 새로운 코드를 요청해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("EMAIL_VERIFIED"), LOCTEXT("EMAIL_VERIFIED", "이메일 인증이 완료되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("WEAK_PASSWORD"), LOCTEXT("WEAK_PASSWORD", "보안을 위해 비밀번호는 6자 이상으로 설정해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_USERNAME_LENGTH"), LOCTEXT("INVALID_USERNAME_LENGTH", "사용자명은 3자에서 20자 사이여야 합니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("USER_ALREADY_EXISTS"), LOCTEXT("USER_ALREADY_EXISTS", "이미 존재하는 사용자 이름입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("USER_CREATED"), LOCTEXT("USER_CREATED", "회원가입이 완료되었습니다. 환영합니다!"));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DUPLICATE_ENTRY"), LOCTEXT("DUPLICATE_ENTRY", "중복된 정보가 존재합니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("ENCRYPTION_ERROR"), LOCTEXT("ENCRYPTION_ERROR", "데이터 암호화 중 오류가 발생했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("USER_NOT_FOUND"), LOCTEXT("USER_NOT_FOUND", "가입되지 않은 사용자입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_PASSWORD"), LOCTEXT("INVALID_PASSWORD", "비밀번호가 일치하지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOKEN_GENERATION_ERROR"), LOCTEXT("TOKEN_GENERATION_ERROR", "로그인 토큰 생성에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_PASSWORD"), LOCTEXT("MISSING_PASSWORD", "계정 삭제를 위해서는 비밀번호를 입력해야 합니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("ACCOUNT_DELETED"), LOCTEXT("ACCOUNT_DELETED", "계정이 성공적으로 삭제되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("PROFILE_RETRIEVED"), LOCTEXT("PROFILE_RETRIEVED", "프로필 정보를 성공적으로 가져왔습니다."));

    // --- 리소스 서버 응답 코드 ---
    ResponseCodeToKoreanTextMap.Emplace(TEXT("PERMISSION_DENIED"), LOCTEXT("PERMISSION_DENIED", "요청에 대한 권한이 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MODEL_NOT_FOUND"), LOCTEXT("MODEL_NOT_FOUND", "모델을 찾을 수 없거나 접근 권한이 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_ACCESSORIES"), LOCTEXT("INVALID_ACCESSORIES", "액세서리 정보가 올바르지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("ACCESS_DENIED"), LOCTEXT("ACCESS_DENIED", "이 리소스에 접근할 권한이 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("FILE_NOT_FOUND"), LOCTEXT("FILE_NOT_FOUND", "요청한 파일을 찾을 수 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_ID"), LOCTEXT("INVALID_ID", "유효하지 않은 ID입니다. 다시 확인해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("PRESET_NOT_FOUND"), LOCTEXT("PRESET_NOT_FOUND", "프리셋을 찾을 수 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("AUDIO_NOT_FOUND"), LOCTEXT("AUDIO_NOT_FOUND", "오디오 파일을 찾을 수 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_MODEL_FILE"), LOCTEXT("MISSING_MODEL_FILE", "모델 파일이 누락되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DUPLICATE_PRESET_NAME"), LOCTEXT("DUPLICATE_PRESET_NAME", "이미 사용 중인 프리셋 이름입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("LIST_FAILED"), LOCTEXT("LIST_FAILED", "목록을 불러오는 데 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("STREAMING_ERROR"), LOCTEXT("STREAMING_ERROR", "데이터 스트리밍 중 오류가 발생했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_CLIENT_ID"), LOCTEXT("MISSING_CLIENT_ID", "클라이언트 ID가 필요합니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DEV_ONLY_API"), LOCTEXT("DEV_ONLY_API", "개발자 전용 기능입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("JOB_NOT_FOUND"), LOCTEXT("JOB_NOT_FOUND", "요청한 작업을 찾을 수 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DUPLICATE_MODEL_NAME"), LOCTEXT("DUPLICATE_MODEL_NAME", "이미 사용 중인 모델 이름입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("SAVE_FAILED"), LOCTEXT("SAVE_FAILED", "저장에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("GET_FAILED"), LOCTEXT("GET_FAILED", "정보를 가져오는 데 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("UPDATE_FAILED"), LOCTEXT("UPDATE_FAILED", "업데이트에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DELETE_FAILED"), LOCTEXT("DELETE_FAILED", "삭제에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("NO_FILE"), LOCTEXT("NO_FILE", "업로드할 파일이 선택되지 않았습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_TITLE"), LOCTEXT("MISSING_TITLE", "제목을 입력해 주세요."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("UPLOAD_ERROR"), LOCTEXT("UPLOAD_ERROR", "파일 업로드 중 오류가 발생했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_SONGS"), LOCTEXT("INVALID_SONGS", "선택된 곡 정보가 올바르지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("CREATE_FAILED"), LOCTEXT("CREATE_FAILED", "생성에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("JOIN_FAILED"), LOCTEXT("JOIN_FAILED", "참가에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("LEAVE_FAILED"), LOCTEXT("LEAVE_FAILED", "나가기에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("CONCERT_NOT_FOUND"), LOCTEXT("CONCERT_NOT_FOUND", "콘서트를 찾을 수 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("ADD_SONG_FAILED"), LOCTEXT("ADD_SONG_FAILED", "곡 추가에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("REMOVE_SONG_FAILED"), LOCTEXT("REMOVE_SONG_FAILED", "곡 삭제에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("CHANGE_SONG_FAILED"), LOCTEXT("CHANGE_SONG_FAILED", "곡 순서 변경에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("GET_CURRENT_SONG_FAILED"), LOCTEXT("GET_CURRENT_SONG_FAILED", "현재 곡 정보를 가져오는 데 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("ADD_ACCESSORY_FAILED"), LOCTEXT("ADD_ACCESSORY_FAILED", "액세서리 추가에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("REMOVE_ACCESSORY_FAILED"), LOCTEXT("REMOVE_ACCESSORY_FAILED", "액세서리 삭제에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("UPDATE_ACCESSORIES_FAILED"), LOCTEXT("UPDATE_ACCESSORIES_FAILED", "액세서리 업데이트에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("UPDATE_LISTEN_SERVER_FAILED"), LOCTEXT("UPDATE_LISTEN_SERVER_FAILED", "서버 정보 업데이트에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("TOGGLE_OPEN_FAILED"), LOCTEXT("TOGGLE_OPEN_FAILED", "콘서트 상태 변경에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("DESTROY_FAILED"), LOCTEXT("DESTROY_FAILED", "콘서트 제거에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("EXPIRE_ALL_FAILED"), LOCTEXT("EXPIRE_ALL_FAILED", "만료된 콘서트 처리에 실패했습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_JOB_CREDENTIALS"), LOCTEXT("MISSING_JOB_CREDENTIALS", "작업 자격 증명이 누락되었습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_JOB_SECRET"), LOCTEXT("INVALID_JOB_SECRET", "작업 비밀 키가 유효하지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("JOB_ALREADY_COMPLETED"), LOCTEXT("JOB_ALREADY_COMPLETED", "이미 완료된 작업입니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("INVALID_PROMPT"), LOCTEXT("INVALID_PROMPT", "프롬프트 내용이 올바르지 않습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("FORBIDDEN"), LOCTEXT("FORBIDDEN", "이 작업을 수행할 권한이 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("NO_UPDATE_FIELDS"), LOCTEXT("NO_UPDATE_FIELDS", "업데이트할 항목이 없습니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("MISSING_THUMBNAIL_FILE"), LOCTEXT("MISSING_THUMBNAIL_FILE", "썸네일 파일이 필요합니다."));
    ResponseCodeToKoreanTextMap.Emplace(TEXT("THUMBNAIL_NOT_FOUND"), LOCTEXT("THUMBNAIL_NOT_FOUND", "썸네일을 찾을 수 없습니다."));
}

#undef LOCTEXT_NAMESPACE