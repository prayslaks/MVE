# 멀티플레이어 커스터마이징 시스템 설계 (GameState 기반)

## 개요
오디언스가 세션에 참가할 때 자신의 커스터마이징(액세서리) 정보를 파일 서버에 업로드하고, **GameState의 TMap에 메타데이터를 저장**합니다. 다른 플레이어들은 GameState Replication을 통해 새로운 메타데이터를 받아 파일을 다운로드하고 적용합니다.

**핵심 특징:**
- ✅ **DB 서버 불필요** - GameState만으로 모든 데이터 관리
- ✅ **세션별 독립성** - 각 세션이 독립적으로 데이터 관리
- ✅ **실시간 동기화** - GameState Replication으로 자동 동기화
- ✅ **간단한 구조** - 파일 서버만 필요

---

## 1. 데이터 구조

### 1.1 GameState (MVE_GS_StageLevel)

```cpp
USTRUCT(BlueprintType)
struct FAccessoryData
{
    GENERATED_BODY()

    UPROPERTY()
    FName SocketName;

    UPROPERTY()
    FVector RelativeLocation;

    UPROPERTY()
    FRotator RelativeRotation;

    UPROPERTY()
    float Ratio;

    UPROPERTY()
    FString ModelUrl;  // "/models/user123_hat.glb"

    FAccessoryData()
        : SocketName(NAME_None)
        , RelativeLocation(FVector::ZeroVector)
        , RelativeRotation(FRotator::ZeroRotator)
        , Ratio(1.0f)
        , ModelUrl(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FCustomizationMetadata
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FAccessoryData> Accessories;
};

UCLASS()
class MVE_API AMVE_GS_StageLevel : public AGameStateBase
{
    GENERATED_BODY()

public:
    // UserID → 커스터마이징 메타데이터
    UPROPERTY(ReplicatedUsing=OnRep_CustomizationMap)
    TMap<FString, FCustomizationMetadata> PlayerCustomizationMap;

    UFUNCTION()
    void OnRep_CustomizationMap();

    // 서버: 플레이어 커스터마이징 추가/업데이트
    UFUNCTION(Server, Reliable)
    void Server_UpdatePlayerCustomization(const FString& UserId, const FCustomizationMetadata& Data);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // 클라이언트: 이미 처리한 UserID 목록 (증분 업데이트용)
    TSet<FString> ProcessedUserIds;

    // 플레이어 찾기
    AMVE_AUD_AudienceCharacter* FindCharacterByUserId(const FString& UserId);

    // 커스터마이징 적용
    void ApplyCustomizationToCharacter(AMVE_AUD_AudienceCharacter* Character, const TArray<FAccessoryData>& Accessories);

    // GLB 로드 및 부착
    void LoadAndAttachAccessory(AMVE_AUD_AudienceCharacter* Character, const FAccessoryData& Accessory, const FString& LocalPath);
};
```

### 1.2 메타데이터 JSON 형식 (참고용)

파일 서버 업로드 시 클라이언트가 생성하는 형식:

```json
{
  "accessories": [
    {
      "socketName": "head_socket",
      "relativeLocation": {"x": 0, "y": 0, "z": 10},
      "relativeRotation": {"pitch": 0, "yaw": 0, "roll": 0},
      "ratio": "0.1",
      "modelUrl": "/models/user123_hat_20250108.glb"
    }
  ]
}
```

---

## 2. 전체 플로우

### 2.1 세션 참가 플로우

```
[클라이언트 - JoinRoomConfirmPopup]
1. ConfirmButton 클릭
2. CustomizationManager에서 SavedCustomization 가져오기
3. 파일 서버에 GLB 업로드
   - POST /api/files/upload
   - Response: { "modelUrl": "/models/user123_hat.glb" }
4. FCustomizationMetadata 로컬에 캐시
   - CachedMetadata.Accessories.Add({ socketName, location, rotation, ratio, modelUrl })
5. 세션 참가 (기존 SessionManager)

[클라이언트 - StageLevel 진입 후]
6. AudienceCharacter 스폰됨
7. BeginPlay() 또는 OnPossess()
8. GameState 가져오기
9. GameState->Server_UpdatePlayerCustomization(UserId, CachedMetadata)

[서버 - GameState]
10. Server_UpdatePlayerCustomization() 받음
11. PlayerCustomizationMap[UserId] = Metadata
12. Map이 Replicate됨 → 모든 클라이언트로 전파
```

### 2.2 기존 플레이어 처리 (증분 업데이트)

```
[기존 클라이언트]
1. GameState::OnRep_CustomizationMap() 호출됨
2. PlayerCustomizationMap 순회
3. ProcessedUserIds에 없는 새 UserID만 처리:
   a. 자기 자신이면 스킵 (이미 로컬에 적용됨)
   b. 다른 플레이어면:
      - Accessories 순회
      - 각 ModelUrl에 대해:
        * CacheManager->IsFileCached() 확인
        * 없으면 다운로드
      - 해당 플레이어의 Pawn 찾기
      - GLB 로드 → 소켓 부착
   c. ProcessedUserIds.Add(UserId)
```

### 2.3 새로 참가한 플레이어 처리 (전체 로드)

```
[새 클라이언트]
1. GameState::OnRep_CustomizationMap() 호출됨
2. ProcessedUserIds가 비어있음 (처음)
3. PlayerCustomizationMap 전체 순회
4. 각 UserID에 대해:
   a. 자기 자신이면 스킵
   b. 다른 플레이어면:
      - Accessories 순회
      - ModelUrl 다운로드 (캐시 없으면)
      - 해당 플레이어의 Pawn 찾기
      - GLB 로드 → 소켓 부착
   c. ProcessedUserIds.Add(UserId)
```

---

## 3. 주요 구현

### 3.1 GameState::OnRep_CustomizationMap()

```cpp
void AMVE_GS_StageLevel::OnRep_CustomizationMap()
{
    PRINTLOG(TEXT("=== OnRep_CustomizationMap ==="));
    PRINTLOG(TEXT("Total players: %d, Already processed: %d"),
        PlayerCustomizationMap.Num(), ProcessedUserIds.Num());

    // 로컬 플레이어 ID 가져오기
    APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
    FString LocalUserId = GetUserIdFromController(LocalPC);

    // Map 순회
    for (auto& Pair : PlayerCustomizationMap)
    {
        const FString& UserId = Pair.Key;
        const FCustomizationMetadata& Metadata = Pair.Value;

        // 이미 처리했으면 스킵
        if (ProcessedUserIds.Contains(UserId))
        {
            continue;
        }

        PRINTLOG(TEXT("Processing new user: %s"), *UserId);

        // 자기 자신은 스킵 (이미 로컬에 적용됨)
        if (UserId == LocalUserId)
        {
            PRINTLOG(TEXT("Skipping own customization"));
            ProcessedUserIds.Add(UserId);
            continue;
        }

        // 해당 플레이어의 Pawn 찾기
        AMVE_AUD_AudienceCharacter* TargetCharacter = FindCharacterByUserId(UserId);
        if (!TargetCharacter)
        {
            PRINTLOG(TEXT("⚠️ Character not found for user: %s (will retry later)"), *UserId);
            // ProcessedUserIds에 추가 안 함 → 다음에 다시 시도
            continue;
        }

        // 커스터마이징 적용
        ApplyCustomizationToCharacter(TargetCharacter, Metadata.Accessories);

        // 처리 완료
        ProcessedUserIds.Add(UserId);
    }
}
```

### 3.2 GameState::Server_UpdatePlayerCustomization()

```cpp
void AMVE_GS_StageLevel::Server_UpdatePlayerCustomization_Implementation(
    const FString& UserId,
    const FCustomizationMetadata& Data)
{
    if (!HasAuthority())
    {
        return;
    }

    PRINTLOG(TEXT("=== Server_UpdatePlayerCustomization ==="));
    PRINTLOG(TEXT("UserId: %s, Accessories: %d"), *UserId, Data.Accessories.Num());

    // Map에 추가/업데이트
    PlayerCustomizationMap.Add(UserId, Data);

    // Replication 트리거 (자동)
    PRINTLOG(TEXT("✅ Added to map, will replicate to all clients"));
}
```

### 3.3 GameState::ApplyCustomizationToCharacter()

```cpp
void AMVE_GS_StageLevel::ApplyCustomizationToCharacter(
    AMVE_AUD_AudienceCharacter* Character,
    const TArray<FAccessoryData>& Accessories)
{
    UCustomizationCacheManager* CacheManager =
        GetGameInstance()->GetSubsystem<UCustomizationCacheManager>();

    if (!CacheManager)
    {
        PRINTLOG(TEXT("❌ CacheManager is null"));
        return;
    }

    for (const FAccessoryData& Accessory : Accessories)
    {
        PRINTLOG(TEXT("Applying accessory: %s to socket: %s"),
            *Accessory.ModelUrl, *Accessory.SocketName.ToString());

        // 캐시 확인
        if (CacheManager->IsFileCached(Accessory.ModelUrl))
        {
            FString LocalPath = CacheManager->GetCachedFilePath(Accessory.ModelUrl);
            PRINTLOG(TEXT("✅ Using cached file: %s"), *LocalPath);
            LoadAndAttachAccessory(Character, Accessory, LocalPath);
        }
        else
        {
            PRINTLOG(TEXT("Downloading file: %s"), *Accessory.ModelUrl);

            // 다운로드 (비동기)
            CacheManager->DownloadFile(Accessory.ModelUrl,
                [this, Character, Accessory](bool bSuccess, FString LocalPath)
                {
                    if (bSuccess)
                    {
                        PRINTLOG(TEXT("✅ Download complete: %s"), *LocalPath);
                        LoadAndAttachAccessory(Character, Accessory, LocalPath);
                    }
                    else
                    {
                        PRINTLOG(TEXT("❌ Download failed: %s"), *Accessory.ModelUrl);
                    }
                });
        }
    }
}
```

### 3.4 GameState::LoadAndAttachAccessory()

```cpp
void AMVE_GS_StageLevel::LoadAndAttachAccessory(
    AMVE_AUD_AudienceCharacter* Character,
    const FAccessoryData& Accessory,
    const FString& LocalPath)
{
    if (!Character)
    {
        PRINTLOG(TEXT("❌ Character is null"));
        return;
    }

    // GLB 로드
    FglTFRuntimeConfig LoaderConfig;
    LoaderConfig.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;

    UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
        LocalPath, false, LoaderConfig);

    if (!Asset)
    {
        PRINTLOG(TEXT("❌ Failed to load GLB: %s"), *LocalPath);
        return;
    }

    // Static Mesh 생성
    FglTFRuntimeStaticMeshConfig StaticMeshConfig;
    UStaticMesh* StaticMesh = Asset->LoadStaticMesh(0, StaticMeshConfig);

    if (!StaticMesh)
    {
        PRINTLOG(TEXT("❌ Failed to create static mesh"));
        return;
    }

    // Actor 생성
    AActor* MeshActor = GetWorld()->SpawnActor<AActor>();
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(MeshActor);
    MeshComponent->SetStaticMesh(StaticMesh);
    MeshActor->SetRootComponent(MeshComponent);
    MeshComponent->RegisterComponent();

    // 소켓에 부착
    USkeletalMeshComponent* SkelMesh = Character->GetMesh();
    if (!SkelMesh || !SkelMesh->DoesSocketExist(Accessory.SocketName))
    {
        PRINTLOG(TEXT("❌ Socket not found: %s"), *Accessory.SocketName.ToString());
        MeshActor->Destroy();
        return;
    }

    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    MeshActor->AttachToComponent(SkelMesh, AttachRules, Accessory.SocketName);

    // Transform 적용
    FTransform RelativeTransform;
    RelativeTransform.SetLocation(Accessory.RelativeLocation);
    RelativeTransform.SetRotation(Accessory.RelativeRotation.Quaternion());
    RelativeTransform.SetScale3D(FVector(Accessory.Ratio));

    MeshActor->SetActorRelativeTransform(RelativeTransform);

    PRINTLOG(TEXT("✅ Accessory attached to socket: %s"), *Accessory.SocketName.ToString());
}
```

### 3.5 GameState::FindCharacterByUserId()

```cpp
AMVE_AUD_AudienceCharacter* AMVE_GS_StageLevel::FindCharacterByUserId(const FString& UserId)
{
    // 모든 PlayerController 순회
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        // PlayerState 또는 다른 방법으로 UserId 확인
        // (구현 방법: UniqueNetId, Custom PlayerState 변수 등)
        FString PlayerUserId = GetUserIdFromController(PC);

        if (PlayerUserId == UserId)
        {
            APawn* Pawn = PC->GetPawn();
            return Cast<AMVE_AUD_AudienceCharacter>(Pawn);
        }
    }

    return nullptr;
}

FString AMVE_GS_StageLevel::GetUserIdFromController(APlayerController* PC)
{
    if (!PC) return TEXT("");

    // 옵션 1: UniqueNetId 사용
    if (PC->PlayerState)
    {
        return PC->PlayerState->GetUniqueId().ToString();
    }

    // 옵션 2: Custom PlayerState 변수
    // AMVE_PS_StageLevel* PS = Cast<AMVE_PS_StageLevel>(PC->PlayerState);
    // return PS ? PS->UserId : TEXT("");

    return TEXT("");
}
```

---

## 4. 클라이언트 측 구현

### 4.1 JoinRoomConfirmPopup::OnConfirmButtonClicked()

```cpp
void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::OnConfirmButtonClicked()
{
    PRINTLOG(TEXT("=== OnConfirmButtonClicked ==="));

    // 1. CustomizationManager에서 데이터 가져오기
    UCustomizationManager* CustomMgr = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
    FCustomizationData SavedData = CustomMgr->GetSavedCustomization();

    if (SavedData.GLBFilePath.IsEmpty())
    {
        PRINTLOG(TEXT("No customization, joining directly"));
        JoinSessionDirect();
        return;
    }

    // 2. 로딩 UI 표시
    ShowLoadingUI(TEXT("Uploading customization..."));

    // 3. 파일 서버에 업로드
    UCustomizationNetworkManager* NetMgr = GetGameInstance()->GetSubsystem<UCustomizationNetworkManager>();

    FString LocalUserId = GetLocalUserId();  // UniqueNetId 또는 이메일

    NetMgr->UploadGLBFile(SavedData.GLBFilePath, LocalUserId,
        [this, SavedData, LocalUserId](bool bSuccess, FString ModelUrl)
        {
            HideLoadingUI();

            if (bSuccess)
            {
                PRINTLOG(TEXT("✅ Upload success: %s"), *ModelUrl);

                // 메타데이터 생성 및 캐시
                FAccessoryData Accessory;
                Accessory.SocketName = SavedData.SocketName;
                Accessory.RelativeLocation = SavedData.RelativeTransform.GetLocation();
                Accessory.RelativeRotation = SavedData.RelativeTransform.Rotator();
                Accessory.Ratio = 1.0f;  // 또는 SavedData에서 가져오기
                Accessory.ModelUrl = ModelUrl;

                CachedCustomizationMetadata.Accessories.Add(Accessory);
                CachedUserId = LocalUserId;

                // 세션 참가
                JoinSessionDirect();
            }
            else
            {
                PRINTLOG(TEXT("❌ Upload failed"));
                ShowErrorPopup(TEXT("Failed to upload customization"));
            }
        });
}

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::JoinSessionDirect()
{
    // 기존 세션 참가 로직
    auto SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>();
    SessionManager->JoinSession(/* ... */);
}

FString UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::GetLocalUserId()
{
    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->PlayerState)
    {
        return PC->PlayerState->GetUniqueId().ToString();
    }
    return TEXT("UnknownUser");
}
```

### 4.2 AudienceCharacter::BeginPlay()

```cpp
void AMVE_AUD_AudienceCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 로컬 플레이어만 커스터마이징 적용
    if (IsLocallyControlled())
    {
        // 1. 로컬에 저장된 커스터마이징 적용 (즉시)
        ApplyCustomization();

        // 2. 서버로 메타데이터 전송 (다른 플레이어들이 볼 수 있도록)
        SendCustomizationToServer();
    }
}

void AMVE_AUD_AudienceCharacter::SendCustomizationToServer()
{
    // JoinConfirmPopup에서 캐시한 데이터 가져오기
    // (GameInstance 또는 PlayerController에 임시 저장되어 있어야 함)

    AMVE_GS_StageLevel* GS = GetWorld()->GetGameState<AMVE_GS_StageLevel>();
    if (!GS)
    {
        PRINTLOG(TEXT("❌ GameState is null"));
        return;
    }

    // GameInstance에서 캐시된 메타데이터 가져오기
    UGameInstance* GI = GetGameInstance();
    // UMyGameInstance* MyGI = Cast<UMyGameInstance>(GI);
    // FCustomizationMetadata CachedMetadata = MyGI->GetCachedCustomizationMetadata();
    // FString UserId = MyGI->GetCachedUserId();

    // 임시: CustomizationManager에서 변환
    UCustomizationManager* CustomMgr = GI->GetSubsystem<UMVE_AUD_CustomizationManager>();
    FCustomizationData SavedData = CustomMgr->GetSavedCustomization();

    if (SavedData.GLBFilePath.IsEmpty())
    {
        PRINTLOG(TEXT("No customization to send"));
        return;
    }

    // FCustomizationMetadata 생성
    FCustomizationMetadata Metadata;
    // ... (JoinConfirmPopup에서 생성한 것과 동일)

    // 서버로 전송
    GS->Server_UpdatePlayerCustomization(GetLocalUserId(), Metadata);
}
```

---

## 5. 파일 서버 API

**Base URL:** `https://fileserver.example.com`

### 5.1 파일 업로드

```
POST /api/files/upload

Request:
  Content-Type: multipart/form-data
  Body:
    - file: <GLB binary>
    - userId: "player123"

Response (200):
{
  "success": true,
  "modelUrl": "/models/player123_hat_20250108123456.glb"
}

Response (400):
{
  "success": false,
  "error": "File too large (max 5MB)"
}
```

### 5.2 파일 다운로드

```
GET /models/{filename}

Response (200):
  Content-Type: application/octet-stream
  Body: <GLB binary>

Response (404):
{
  "error": "File not found"
}
```

---

## 6. CustomizationCacheManager

### 6.1 역할

- 로컬 파일 캐싱 (`Saved/Customizations/`)
- 파일 다운로드 (비동기 HTTP)
- 캐시 관리 (용량 제한, LRU)

### 6.2 주요 함수

```cpp
class UCustomizationCacheManager : public UGameInstanceSubsystem
{
public:
    // 캐시에 파일이 있는지 확인
    bool IsFileCached(const FString& ModelUrl);

    // 캐시된 파일의 로컬 경로 반환
    FString GetCachedFilePath(const FString& ModelUrl);

    // 파일 다운로드 (비동기)
    void DownloadFile(
        const FString& ModelUrl,
        TFunction<void(bool bSuccess, FString LocalPath)> OnComplete
    );

private:
    FString FileServerBaseURL = TEXT("https://fileserver.example.com");
    FString CacheDirectory;  // "Saved/Customizations/"

    // ModelUrl → Hash 계산 (파일명으로 사용)
    FString GetFileHash(const FString& ModelUrl);

    // 캐시 디렉토리 초기화
    void InitializeCacheDirectory();

    // HTTP 요청
    void DownloadFileHTTP(const FString& FullURL, const FString& SavePath, TFunction<void(bool)> OnComplete);
};
```

### 6.3 구현 예시

```cpp
bool UCustomizationCacheManager::IsFileCached(const FString& ModelUrl)
{
    FString LocalPath = GetCachedFilePath(ModelUrl);
    return FPaths::FileExists(LocalPath);
}

FString UCustomizationCacheManager::GetCachedFilePath(const FString& ModelUrl)
{
    FString FileHash = GetFileHash(ModelUrl);
    return FPaths::Combine(CacheDirectory, FileHash + TEXT(".glb"));
}

FString UCustomizationCacheManager::GetFileHash(const FString& ModelUrl)
{
    // SHA256 또는 단순 파일명 추출
    FString Filename = FPaths::GetCleanFilename(ModelUrl);
    return Filename;
}

void UCustomizationCacheManager::DownloadFile(
    const FString& ModelUrl,
    TFunction<void(bool, FString)> OnComplete)
{
    FString FullURL = FileServerBaseURL + ModelUrl;
    FString SavePath = GetCachedFilePath(ModelUrl);

    DownloadFileHTTP(FullURL, SavePath, [this, SavePath, OnComplete](bool bSuccess)
    {
        if (OnComplete)
        {
            OnComplete(bSuccess, bSuccess ? SavePath : TEXT(""));
        }
    });
}
```

---

## 7. CustomizationNetworkManager

### 7.1 역할

- 파일 서버 통신
- GLB 업로드 (multipart/form-data)

### 7.2 주요 함수

```cpp
class UCustomizationNetworkManager : public UGameInstanceSubsystem
{
public:
    // GLB 파일 업로드
    void UploadGLBFile(
        const FString& LocalFilePath,
        const FString& UserId,
        TFunction<void(bool bSuccess, FString ModelUrl)> OnComplete
    );

private:
    FString FileServerBaseURL = TEXT("https://fileserver.example.com");
};
```

### 7.3 구현 예시

```cpp
void UCustomizationNetworkManager::UploadGLBFile(
    const FString& LocalFilePath,
    const FString& UserId,
    TFunction<void(bool, FString)> OnComplete)
{
    // 파일 로드
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *LocalFilePath))
    {
        PRINTLOG(TEXT("❌ Failed to load file: %s"), *LocalFilePath);
        if (OnComplete) OnComplete(false, TEXT(""));
        return;
    }

    // HTTP 요청
    FHttpModule* HttpModule = &FHttpModule::Get();
    TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

    HttpRequest->SetURL(FileServerBaseURL + TEXT("/api/files/upload"));
    HttpRequest->SetVerb(TEXT("POST"));

    // Multipart boundary
    FString Boundary = FString::Printf(TEXT("----UnrealBoundary%d"), FDateTime::Now().GetTicks());
    HttpRequest->SetHeader(TEXT("Content-Type"),
        FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    // Body 구성
    TArray<uint8> BodyData;

    // userId 파트
    FString UserIdPart;
    UserIdPart += FString::Printf(TEXT("--%s\r\n"), *Boundary);
    UserIdPart += TEXT("Content-Disposition: form-data; name=\"userId\"\r\n\r\n");
    UserIdPart += UserId;
    UserIdPart += TEXT("\r\n");

    FTCHARToUTF8 UserIdConv(*UserIdPart);
    BodyData.Append((uint8*)UserIdConv.Get(), UserIdConv.Length());

    // file 파트
    FString FilePart;
    FilePart += FString::Printf(TEXT("--%s\r\n"), *Boundary);
    FilePart += TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"accessory.glb\"\r\n");
    FilePart += TEXT("Content-Type: model/gltf-binary\r\n\r\n");

    FTCHARToUTF8 FilePartConv(*FilePart);
    BodyData.Append((uint8*)FilePartConv.Get(), FilePartConv.Length());
    BodyData.Append(FileData);

    FString LineBreak = TEXT("\r\n");
    FTCHARToUTF8 LBConv(*LineBreak);
    BodyData.Append((uint8*)LBConv.Get(), LBConv.Length());

    // 종료 boundary
    FString Closing = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
    FTCHARToUTF8 ClosingConv(*Closing);
    BodyData.Append((uint8*)ClosingConv.Get(), ClosingConv.Length());

    HttpRequest->SetContent(BodyData);

    // 콜백
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (!bSucceeded || !Response.IsValid())
            {
                PRINTLOG(TEXT("❌ Upload failed"));
                if (OnComplete) OnComplete(false, TEXT(""));
                return;
            }

            int32 ResponseCode = Response->GetResponseCode();
            FString ResponseContent = Response->GetContentAsString();

            if (ResponseCode == 200)
            {
                // JSON 파싱
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

                if (FJsonSerializer::Deserialize(Reader, JsonObject))
                {
                    FString ModelUrl = JsonObject->GetStringField(TEXT("modelUrl"));
                    PRINTLOG(TEXT("✅ Upload success: %s"), *ModelUrl);
                    if (OnComplete) OnComplete(true, ModelUrl);
                }
                else
                {
                    PRINTLOG(TEXT("❌ Failed to parse response"));
                    if (OnComplete) OnComplete(false, TEXT(""));
                }
            }
            else
            {
                PRINTLOG(TEXT("❌ Server error: %d"), ResponseCode);
                if (OnComplete) OnComplete(false, TEXT(""));
            }
        });

    HttpRequest->ProcessRequest();
}
```

---

## 8. 시퀀스 다이어그램

### 8.1 세션 참가 및 동기화

```
Player A (기존)  |  GameState (Server)  |  Player B (신규)
                 |                       |
                 | Map = { A: {...} }    |
                 |                       |
                 |                       | [JoinConfirmPopup]
                 |                       | Upload GLB → FileServer
                 |                       | { modelUrl: "..." }
                 |                       |
                 |                       | BeginPlay()
                 | ← Server_UpdatePlayerCustomization(B, {...})
                 | Map[B] = {...}        |
                 | → Replicate Map       |
                 |                       |
OnRep_Map()      |                       | OnRep_Map()
ProcessedIds={}  |                       | ProcessedIds={}
New: A, B        |                       | New: A, B
Skip B (self)    |                       | Skip A (self)
Download A's GLB |                       | Download B's GLB
Apply to A       |                       | Apply to B
ProcessedIds={A,B}                      | ProcessedIds={A,B}
```

### 8.2 플레이어 퇴장

```
Player A leaves:

Server GameMode::Logout():
  - GameState->PlayerCustomizationMap.Remove(A's UserId)
  - Map Replicates

All Clients::OnRep_CustomizationMap():
  - ProcessedUserIds.Remove(A's UserId)
  - (액세서리는 Pawn Destroy 시 자동 제거)
```

---

## 9. 구현 단계

### Phase 1: Core Infrastructure
- [x] `MVE_GS_StageLevel` 생성
- [x] `FCustomizationMetadata`, `FAccessoryData` 구조체
- [x] `PlayerCustomizationMap` + Replication
- [x] `OnRep_CustomizationMap()` 구현
- [x] `Server_UpdatePlayerCustomization()` 구현

### Phase 2: Cache & Network
- [x] `CustomizationCacheManager` 생성
  - [x] `IsFileCached()`
  - [x] `GetCachedFilePath()`
  - [x] `DownloadFile()`
- [x] `CustomizationNetworkManager` 생성
  - [x] `UploadGLBFile()`

### Phase 3: Client Upload
- [x] `JoinRoomConfirmPopup` 수정
  - [x] ConfirmButton 클릭 → 파일 업로드
  - [x] 메타데이터 캐시
  - [x] 세션 참가
- [x] 로딩 UI 추가

### Phase 4: Client Apply
- [x] `AudienceCharacter::BeginPlay()` 수정
  - [x] 로컬 커스터마이징 적용
  - [x] 서버로 메타데이터 전송
- [x] `ApplyCustomizationToCharacter()` 구현
- [x] `LoadAndAttachAccessory()` 구현

### Phase 5: Testing
- [ ] 싱글 플레이어 테스트
- [ ] 멀티 플레이어 테스트 (PIE 2-3명)
- [ ] 파일 캐싱 동작 확인
- [ ] 네트워크 실패 시나리오
- [ ] 성능 테스트

---

## 10. 최적화 및 예외 처리

### 10.1 Replication 최적화

```cpp
void AMVE_GS_StageLevel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 변경될 때만 Replicate
    DOREPLIFETIME(AMVE_GS_StageLevel, PlayerCustomizationMap);
}
```

### 10.2 파일 크기 제한

- **최대 GLB 파일 크기:** 5MB
- 업로드 전 클라이언트에서 검증
- 서버에서도 검증

### 10.3 다운로드 실패 처리

```cpp
void UCustomizationCacheManager::DownloadFile(...)
{
    int32 MaxRetries = 3;
    int32 RetryCount = 0;

    // Retry 로직
    // ...
}
```

### 10.4 캐시 관리

- **최대 캐시 크기:** 100MB
- **LRU 정책** (Least Recently Used)
- 30일 미사용 파일 자동 삭제

### 10.5 동시 다운로드 제한

```cpp
class UCustomizationCacheManager
{
private:
    int32 MaxConcurrentDownloads = 3;
    TArray<FDownloadRequest> DownloadQueue;
    int32 CurrentDownloads = 0;

    void ProcessDownloadQueue();
};
```

---

## 11. 보안 고려사항

### 11.1 파일 검증

- **업로드 시:** MIME 타입 확인 (model/gltf-binary)
- **다운로드 후:** 파일 시그니처 확인

### 11.2 메타데이터 검증

```cpp
bool ValidateAccessoryData(const FAccessoryData& Data)
{
    // SocketName 화이트리스트
    static const TSet<FName> AllowedSockets = {
        TEXT("head_socket"),
        TEXT("hand_r"),
        TEXT("hand_l"),
        TEXT("back_socket")
    };

    if (!AllowedSockets.Contains(Data.SocketName))
        return false;

    // Transform 범위 제한
    if (Data.RelativeLocation.Size() > 1000.0f)
        return false;

    if (Data.Ratio < 0.01f || Data.Ratio > 2.0f)
        return false;

    return true;
}
```

### 11.3 사용자 인증

- 파일 서버 요청 시 JWT 토큰 포함
- 서버 측에서 UserId 검증

---

## 12. 장점 및 특징

### ✅ 주요 장점

1. **단순한 구조**
   - DB 서버 불필요
   - GameState만으로 완결
   - 개발/운영 비용 절감

2. **세션별 독립성**
   - 각 세션이 독립적
   - 데이터 충돌 없음
   - 세션 종료 시 자동 정리

3. **실시간 동기화**
   - GameState Replication으로 자동
   - 레이턴시 최소화
   - 신뢰성 보장

4. **효율적 업데이트**
   - TMap으로 O(1) 조회
   - 증분 업데이트 (ProcessedUserIds)
   - 불필요한 재처리 방지

### 🔧 확장 가능성

1. **여러 액세서리 지원**
   - Accessories 배열 활용
   - 모자, 무기, 목걸이 등 동시 착용

2. **실시간 변경**
   - 세션 중 액세서리 교체
   - Map 업데이트 → 자동 Replicate

3. **플레이어 퇴장 처리**
   - GameMode::Logout에서 Map.Remove()
   - 자동 동기화

---

## 부록: 설정 값

### A.1 DefaultGame.ini

```ini
[/Script/MVE.CustomizationNetworkManager]
FileServerBaseURL=https://fileserver.example.com

[/Script/MVE.CustomizationCacheManager]
MaxCacheSize=104857600  ; 100MB
CacheDirectory=Saved/Customizations
MaxConcurrentDownloads=3

[/Script/MVE.MVE_GS_StageLevel]
MaxPlayersPerSession=50
```

### A.2 파일 제한

```ini
[/Script/MVE.CustomizationNetworkManager]
MaxFileSize=5242880     ; 5MB
AllowedFileExtensions=glb
```

---

## 요약

이 설계는 **GameState의 TMap**을 활용하여 DB 없이도 멀티플레이어 커스터마이징을 완벽하게 지원합니다. 세션별 독립성과 실시간 동기화를 보장하며, 간단하면서도 확장 가능한 구조입니다.

**핵심:**
- 파일 서버: GLB 업로드/다운로드만
- GameState: TMap으로 메타데이터 관리 + Replication
- 증분 업데이트: ProcessedUserIds로 중복 처리 방지
- 로컬 캐싱: 다운로드 최소화
