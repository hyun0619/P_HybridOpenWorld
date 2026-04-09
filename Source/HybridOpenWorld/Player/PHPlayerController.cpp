#include "PHPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/PHCameraActor.h"
#include "Camera/PHLookAroundComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Data/LevelSettingsData.h"
#include "Data/GameMasterAsset.h"
#include "Data/CameraPresetDataAsset.h"
#include "Game/PHGameInstance.h"
#include "Input/PHInputComponent.h"
#include "Camera/PHCameraSubsystem.h"


APHPlayerController::APHPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	InputManager = CreateDefaultSubobject<UPHInputComponent>(TEXT("InputManager"));
	EdgeScrollComponent = CreateDefaultSubobject<UPHLookAroundComponent>(TEXT("EdgeScrollComponent"));
}

void APHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	InitEssentialReferences();
	FetchLevelData();
	if (bHasValidLevelData && MainCameraActor)
		ApplyInitialLevelSetup();
}

/** 월드 내에 배치된 메인 카메라 찾아 캐싱 */
void APHPlayerController::InitEssentialReferences()
{
	MainCameraActor = Cast<APHCameraActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APHCameraActor::StaticClass()));
}

APHCameraActor* APHPlayerController::GetMainCameraActor() const
{
	return MainCameraActor;
}

/** 현재 맵 설정 정보 읽어옴 */
void APHPlayerController::FetchLevelData()
{
	if (!IsValid(MasterLevelSettings)) { UE_LOG(LogTemp, Error, TEXT("MasterLevelSettings 유효하지 않음")); return; }
	if (!IsValid(MasterLevelSettings->LevelTable)) { UE_LOG(LogTemp, Error, TEXT("LevelTable 유효하지 않음")); return; }

	// 현재 열린 맵 이름 가져옴
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	// DT에서 맵 이름 키 검색
	static const FString Ctx(TEXT("LevelLookupContext"));
	if (FLevelSettingsRow* Row = MasterLevelSettings->LevelTable->FindRow<FLevelSettingsRow>(FName(*MapName), Ctx))
	{
		CurrentLevelRow = *Row;
		bHasValidLevelData = true;
	}
}

/** 레벨 데이터에 따른 초기 게임 환경 구성 */
void APHPlayerController::ApplyInitialLevelSetup()
{
	SetViewTarget(MainCameraActor); // 뷰 타겟을 메인 카메라 액터로 설정

	switch (CurrentLevelRow.LevelType) // 레벨 타입에 따라 입력 상태 결정
	{
	case ELevelType::WorldMap:   DefaultState = EInputState::WorldMap; break;
	case ELevelType::Detailed:   DefaultState = EInputState::Detailed; break;
	case ELevelType::Event:      DefaultState = EInputState::Cinematic; break;
	default:                     DefaultState = EInputState::Detailed; break;
	}
	ChangeInputState(DefaultState);
	
	// 카메라 서브시스템에 해당 레벨의 기본 카메라 프리셋 전달
	if (MainCameraActor && CurrentLevelRow.CameraPreset)
		if (auto* Sub = GetWorld()->GetSubsystem<UPHCameraSubsystem>())
			Sub->SetDefaultPreset(CurrentLevelRow.CameraPreset);

	HandleInitialSpawn(); // 캐릭터 배치 로직 실행
}

/** 맵 이동 후 캐릭터 위치 결정, 카메라 동기화 */
void APHPlayerController::HandleInitialSpawn()
{
	auto* GI = Cast<UPHGameInstance>(GetGameInstance());
	APawn* P = GetPawn();
	if (!P || !GI) return;

	FVector Loc = CurrentLevelRow.DefaultSpawnLocation; // 기본 스폰 위치 설정
	
	// 만약 GameInstance에 특정 Tag 정보 있다면 해당 위치로 변경
	if (GI->PendingSpawnTag.IsValid() && CurrentLevelRow.SpawnLocations.Contains(GI->PendingSpawnTag))
		Loc = CurrentLevelRow.SpawnLocations[GI->PendingSpawnTag];

	P->SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics); // 캐릭터 순간이동
	MainCameraActor->SetActorLocation(Loc); // 카메라 액터도 즉시 해당 위치로 이동시켜 랙 없이 화면 고정
	GI->PendingSpawnTag = FGameplayTag::EmptyTag; // 처리 완료 후 태그 리셋
}

/** 상태 변경 시 실제 IMC 교체 */
void APHPlayerController::ChangeInputState(EInputState NewState)
{
	CurrentState = NewState;
	if (InputManager) InputManager->ApplyInputState(this, CurrentState);
}

void APHPlayerController::RevertToDefaultState()
{
	ChangeInputState(DefaultState);
}

/** 향상된 입력 바인딩 */
void APHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_LookAround)
		{
			// 우클릭 시작 시 둘러보기 활성화
			EnhancedInputComp->BindAction(IA_LookAround, ETriggerEvent::Started, this, &APHPlayerController::OnLookAroundStarted);
            
			// 우클릭 종료 시 비활성화
			EnhancedInputComp->BindAction(IA_LookAround, ETriggerEvent::Completed, this, &APHPlayerController::OnLookAroundCompleted);
		}
	}
}

void APHPlayerController::OnLookAroundStarted(const FInputActionValue& Value)
{
	if (EdgeScrollComponent)
	{
		EdgeScrollComponent->SetLookAroundActive(true);
	}
}

void APHPlayerController::OnLookAroundCompleted(const FInputActionValue& Value)
{
	if (EdgeScrollComponent)
	{
		EdgeScrollComponent->SetLookAroundActive(false);
	}
}