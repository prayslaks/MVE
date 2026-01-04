#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WC_LoadingScreen.generated.h"

class UMediaPlayer;
class UMediaTexture;
class UFileMediaSource;
class UImage;

/**
 * 로딩 화면 위젯
 * MediaPlayer를 사용하여 로딩 영상을 재생
 */
UCLASS()
class MVE_API UMVE_WC_LoadingScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 로딩 영상 재생 시작
	 * @param VideoPath Content/Movies/ 기준 영상 파일 경로 (예: "LoadingVideo.mp4")
	 * @param bLoop 루프 재생 여부
	 */
	void PlayLoadingVideo(const FString& VideoPath, bool bLoop = true);

	/**
	 * 로딩 영상 재생 중지
	 */
	void StopLoadingVideo();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 영상을 표시할 Image 위젯 (BindWidget) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LoadingVideoImage;

private:
	UPROPERTY()
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY()
	TObjectPtr<UMediaTexture> MediaTexture;

	UPROPERTY()
	TObjectPtr<UFileMediaSource> MediaSource;

	void InitializeMediaPlayer();

	UFUNCTION()
	void OnMediaOpened(FString OpenedUrl);

	UFUNCTION()
	void OnMediaOpenFailed(FString FailedUrl);
};
