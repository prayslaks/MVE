#pragma once

#include "CoreMinimal.h"
#include "ScreenTypes.generated.h"

/*
 *	--------------- Screen ---------------
 */

UENUM(BlueprintType)
enum class EUIScreen : uint8
{
	None,
	Main,
	Login,
	Register,
	WelcomeRegister,
	ModeSelect,
	Credit,
	Studio_CharacterSetting,
	Studio_StageSetting,
	Studio_ChatFilterSetting,
	Studio_FinalCheckSetting,
	Studio_OnLive,
	AudienceStation,
	AudienceCustomizing,
	AudienceGenerateMesh,
	AudienceConcertRoom
};


USTRUCT(BlueprintType)
struct FScreenClassInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIScreen Screen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;
};


/*
 *	--------------- 드롭다운 ---------------
 */

// 드롭다운 위젯 타입
UENUM(BlueprintType)
enum class EDropdownType : uint8
{
	None,
	UserSetting,      // 기존 유저 설정 드롭다운
	BannedWordItem   // 금지어 아이템 드롭다운
};

// 드롭다운 위치 옵션
UENUM(BlueprintType)
enum class EDropdownAnchorPosition : uint8
{
	TopLeft         UMETA(DisplayName = "Top Left"),          // 좌상단
	TopRight        UMETA(DisplayName = "Top Right"),         // 우상단
	MiddleLeft      UMETA(DisplayName = "Middle Left"),       // 좌측 중앙
	MiddleRight     UMETA(DisplayName = "Middle Right"),      // 우측 중앙
	BottomLeft      UMETA(DisplayName = "Bottom Left"),       // 좌하단
	BottomRight     UMETA(DisplayName = "Bottom Right")       // 우하단
};

USTRUCT(BlueprintType)
struct FDropdownClassInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDropdownType DropdownType = EDropdownType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UMVE_WidgetClass_Dropdown> WidgetClass;
};

USTRUCT(BlueprintType)
struct FDropdownContext
{
	GENERATED_BODY()

	// 공통 데이터
	UPROPERTY(BlueprintReadWrite)
	FVector2D ButtonPosition;

	UPROPERTY(BlueprintReadWrite)
	FVector2D ButtonSize;

	UPROPERTY(BlueprintReadWrite)
	EDropdownAnchorPosition AnchorPosition = EDropdownAnchorPosition::TopLeft;

	// 타입별 선택적 데이터
	UPROPERTY(BlueprintReadWrite)
	FString StringData;  // UserName, ItemText 등

	UPROPERTY(BlueprintReadWrite)
	int32 IndexData = -1;  // Item Index 등

	UPROPERTY(BlueprintReadWrite)
	UObject* ObjectData = nullptr;  // 추가 데이터
};

/*
 *	--------------- 팝업 ---------------
 */


UENUM(BlueprintType)
enum class EUIPopup : uint8
{
	None,
	JoinRoomConfirm,
	LogoutConfirm,
	ModalBackground
};

USTRUCT(BlueprintType)
struct FPopupClassInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIPopup PopupType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;
};


/*
 *	--------------- Persistent 위젯 ---------------
 */

UENUM(BlueprintType)
enum class EPersistentWidget : uint8
{
	None,
	StudioSettingTabBar
};

USTRUCT(BlueprintType)
struct FPersistentWidgetClassInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	// Persistent Widget 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPersistentWidget WidgetType = EPersistentWidget::None;
    
	// Widget 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;

	// 어떤 Screen들에서 표시할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<EUIScreen> ActiveScreens;

	// ZOrder (Screen 위에 표시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ZOrder = 10;
};




