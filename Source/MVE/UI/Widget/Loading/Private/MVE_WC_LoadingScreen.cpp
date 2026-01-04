#include "../Public/MVE_WC_LoadingScreen.h"
#include "MVE.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"
#include "MediaSource.h"

void UMVE_WC_LoadingScreen::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeMediaPlayer();
}

void UMVE_WC_LoadingScreen::NativeDestruct()
{
	Super::NativeDestruct();

	StopLoadingVideo();
}

void UMVE_WC_LoadingScreen::InitializeMediaPlayer()
{
	// MediaPlayer 생성
	MediaPlayer = NewObject<UMediaPlayer>(this);
	if (MediaPlayer)
	{
		MediaPlayer->SetLooping(false); // 루프는 PlayLoadingVideo()에서 설정
		MediaPlayer->PlayOnOpen = false; // 수동으로 Play() 호출할 것이므로 false

		// 델리게이트 바인딩
		MediaPlayer->OnMediaOpened.AddDynamic(this, &UMVE_WC_LoadingScreen::OnMediaOpened);
		MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UMVE_WC_LoadingScreen::OnMediaOpenFailed);

		PRINTLOG(TEXT("MediaPlayer 생성됨"));
	}

	// MediaTexture 생성
	MediaTexture = NewObject<UMediaTexture>(this);
	if (MediaTexture)
	{
		MediaTexture->SetMediaPlayer(MediaPlayer);
		MediaTexture->AutoClear = false;
		MediaTexture->UpdateResource();

		PRINTLOG(TEXT("MediaTexture 생성됨"));
	}
}

void UMVE_WC_LoadingScreen::PlayLoadingVideo(const FString& VideoPath, bool bLoop)
{
	if (!MediaPlayer || !MediaTexture || !LoadingVideoImage)
	{
		PRINTLOG(TEXT("❌ MediaPlayer, MediaTexture 또는 LoadingVideoImage가 null입니다"));
		return;
	}

	// 루프 설정
	MediaPlayer->SetLooping(bLoop);

	// 영상 파일 경로 구성 (Content/Movies/ 폴더 기준)
	FString FullPath = FPaths::ProjectContentDir() + TEXT("Movies/") + VideoPath;

	// FileMediaSource 생성
	MediaSource = NewObject<UFileMediaSource>(this);
	if (MediaSource)
	{
		MediaSource->SetFilePath(FullPath);

		// 영상 재생
		if (MediaPlayer->OpenSource(MediaSource))
		{
			PRINTLOG(TEXT("🎬 로딩 영상 재생 시작: %s"), *FullPath);

			// Image 위젯에 MediaTexture 설정 (Brush 수동 구성)
			FSlateBrush Brush;
			Brush.SetResourceObject(MediaTexture);
			Brush.ImageSize = FVector2D(1920, 1080); // 영상 해상도에 맞게 조정
			Brush.DrawAs = ESlateBrushDrawType::Image;
			Brush.Tiling = ESlateBrushTileType::NoTile;

			LoadingVideoImage->SetBrush(Brush);

			PRINTLOG(TEXT("📺 MediaTexture를 Image 위젯에 설정 완료"));
		}
		else
		{
			PRINTLOG(TEXT("❌ 영상 재생 실패: %s"), *FullPath);
		}
	}
}

void UMVE_WC_LoadingScreen::StopLoadingVideo()
{
	if (MediaPlayer && MediaPlayer->IsPlaying())
	{
		MediaPlayer->Close();
		PRINTLOG(TEXT("⏹️ 로딩 영상 재생 중지"));
	}
}

void UMVE_WC_LoadingScreen::OnMediaOpened(FString OpenedUrl)
{
	PRINTLOG(TEXT("✅ 영상 로드 성공: %s"), *OpenedUrl);

	// 명시적으로 재생 시작 (PlayOnOpen이 작동하지 않을 수 있음)
	if (MediaPlayer)
	{
		if (!MediaPlayer->IsPlaying())
		{
			bool bPlayResult = MediaPlayer->Play();
			PRINTLOG(TEXT("🎬 MediaPlayer->Play() 호출: %s"), bPlayResult ? TEXT("Success") : TEXT("Failed"));
		}

		PRINTLOG(TEXT("📹 MediaPlayer 상태:"));
		PRINTLOG(TEXT("  - IsPlaying: %s"), MediaPlayer->IsPlaying() ? TEXT("Yes") : TEXT("No"));
		PRINTLOG(TEXT("  - IsPaused: %s"), MediaPlayer->IsPaused() ? TEXT("Yes") : TEXT("No"));
		PRINTLOG(TEXT("  - Duration: %.2f"), MediaPlayer->GetDuration().GetTotalSeconds());
		PRINTLOG(TEXT("  - Video Track Count: %d"), MediaPlayer->GetNumTracks(EMediaPlayerTrack::Video));
	}

	if (MediaTexture)
	{
		// MediaTexture 강제 업데이트
		MediaTexture->UpdateResource();

		PRINTLOG(TEXT("🖼️ MediaTexture 상태:"));
		PRINTLOG(TEXT("  - Size: %dx%d"), MediaTexture->GetWidth(), MediaTexture->GetHeight());
		PRINTLOG(TEXT("  - MediaPlayer: %s"), MediaTexture->GetMediaPlayer() ? TEXT("Connected") : TEXT("null"));
	}
}

void UMVE_WC_LoadingScreen::OnMediaOpenFailed(FString FailedUrl)
{
	PRINTLOG(TEXT("❌ 영상 로드 실패: %s"), *FailedUrl);
	PRINTLOG(TEXT("💡 영상 파일이 Content/Movies/ 폴더에 있는지 확인하세요"));
}
