#pragma once

#include "CoreMinimal.h"
#include "ScreenTypes.generated.h"
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

UENUM(BlueprintType)
enum class EUIPopup : uint8
{
	None,
	JoinRoomConfirm,
	LogoutConfirm,
	ModalBackground
};

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

UENUM(BlueprintType)
enum class EPersistentWidget : uint8
{
	None,
	StudioSettingTabBar
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

USTRUCT(BlueprintType)
struct FPopupClassInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIPopup PopupType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;
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