#include "ProjectHPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/ProjectHCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Data/LevelSettingsData.h"
#include "Data/GameMasterAsset.h" 
#include "Data/CameraPresetDataAsset.h"
#include "Game/ProjectHGameInstance.h"
#include "Input/ProjectHInputComponent.h"
#include "Camera/ProjectHCameraSubsystem.h"

AProjectHPlayerController::AProjectHPlayerController()
{
	// 마우스 기본 설정
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	// 카메라 추적 시 화면 떨림 방지를 위해 틱 조정
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	// 입력 전달 컴포넌트 생성 및 부착
	InputManager = CreateDefaultSubobject<UProjectHInputComponent>(TEXT("InputManager"));
}

void AProjectHPlayerController::BeginPlay()
{
    Super::BeginPlay();
	
	InitEssentialReferences(); // 참조 초기화
	FetchLevelData(); // DT에서 레벨 정보 가져옴
	
	if (bHasValidLevelData && MainCameraActor)
	{
		ApplyInitialLevelSetup(); // 로드 성공 시 시스템 셋팅
	}
}

void AProjectHPlayerController::InitEssentialReferences()
{
	MainCameraActor = Cast<AProjectHCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
}

AProjectHCameraActor* AProjectHPlayerController::GetMainCameraActor() const
{
	return MainCameraActor;
}

void AProjectHPlayerController::FetchLevelData()
{
	if (!IsValid(MasterLevelSettings)) // 로그 체크
	{
		UE_LOG(LogTemp, Error, TEXT("MasterLevelSettings 가 유효하지 않습니다!"));
		return;
	}

	if (!IsValid(MasterLevelSettings->LevelTable))
	{
		UE_LOG(LogTemp, Error, TEXT("LevelTable 이 유효하지 않습니다!"));
		return;
	}

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	FName RowName = FName(*MapName);
    
	// 이름으로 바로 레벨 찾기
	static const FString ContextString(TEXT("LevelLookupContext"));
	FLevelSettingsRow* FoundRow = MasterLevelSettings->LevelTable->FindRow<FLevelSettingsRow>(RowName, ContextString);

	if (FoundRow)
	{
		CurrentLevelRow = *FoundRow;
		bHasValidLevelData = true;
		UE_LOG(LogTemp, Log, TEXT("성공: %s 행 데이터를 찾았습니다."), *RowName.ToString());
	}
}

void AProjectHPlayerController::ApplyInitialLevelSetup()
{
	SetViewTarget(MainCameraActor); // 카메라 뷰 타겟 설정
	
	switch (CurrentLevelRow.LevelType)
	{
	case ELevelType::WorldMap:
		DefaultState = EInputState::WorldMap;
		break;
	case ELevelType::Detailed:
		DefaultState = EInputState::Detailed;
		break;
	case ELevelType::Event:
		DefaultState = EInputState::Cinematic;
		break;
	default:
		DefaultState = EInputState::Detailed; // 안전 장치
		break;
	}
	ChangeInputState(DefaultState); // 기본 상태로 조작을 셋팅
	
	// [코드 수정] 서브시스템에 기본 카메라 등록
	if (MainCameraActor && CurrentLevelRow.CameraPreset)
	{
		UProjectHCameraSubsystem* CameraSubsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
		if (CameraSubsystem)
		{
			// ★ 수정: Settings가 아니라 CameraPreset(DA 원본)을 던져줍니다!
			CameraSubsystem->SetDefaultPreset(CurrentLevelRow.CameraPreset);
		}
	}
	
	HandleInitialSpawn(); // 캐릭터 스폰 배치
}

void AProjectHPlayerController::HandleInitialSpawn()
{
	UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(GetGameInstance());
	APawn* P = GetPawn();

	if (P && GI)
	{
		FVector TargetLoc;
		// GI에 저장된 태그가 유효하고 현재 레벨 데이터에 해당 태그 좌표가 있다면 사용
		if (GI->PendingSpawnTag.IsValid() && CurrentLevelRow.SpawnLocations.Contains(GI->PendingSpawnTag))
		{
			TargetLoc = CurrentLevelRow.SpawnLocations[GI->PendingSpawnTag];
		}
		else
		{
			TargetLoc = CurrentLevelRow.DefaultSpawnLocation;
		}

		// 캐릭터와 카메라를 해당 위치로 텔레포트
		P->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
		MainCameraActor->SetActorLocation(TargetLoc);

		// 사용한 태그 초기화
		GI->PendingSpawnTag = FGameplayTag::EmptyTag;
	}
}

void AProjectHPlayerController::ChangeInputState(EInputState NewState)
{
	CurrentState = NewState;
	
	if (InputManager)
	{
		InputManager->ApplyInputState(this, CurrentState);
	}
}

void AProjectHPlayerController::RevertToDefaultState()
{
	ChangeInputState(DefaultState);
}