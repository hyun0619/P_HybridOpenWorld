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

void AProjectHPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	if (bCachedFollowPawn && MainCameraActor && GetPawn())
	{
		const FVector TargetLocation = GetPawn()->GetActorLocation();
		const FVector CurrentLocation = MainCameraActor->GetActorLocation();
        
		// VInterpTo를 사용하여 카메라가 캐릭터를 부드럽게 추적
		FVector SmoothLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, CurrentTrackingSpeed);
		MainCameraActor->SetActorLocation(SmoothLocation);
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
	if (!MasterLevelSettings || !MasterLevelSettings->LevelTable) return;

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
	else
	{
		bHasValidLevelData = false;
		UE_LOG(LogTemp, Error, TEXT("실패: %s 이름과 일치하는 행이 테이블에 없습니다!"), *RowName.ToString());
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
	
	HandleInitialSpawn(); // 캐릭터 스폰 배치
	ApplyCameraPreset(); // 카메라 프리셋 적용
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

void AProjectHPlayerController::ApplyCameraPreset()
{
	if (MainCameraActor && CurrentLevelRow.CameraPreset)
	{
		auto* P = CurrentLevelRow.CameraPreset;

		// 기본 설정 적용
		MainCameraActor->UpdateCameraSettings(P->TargetArmLength, P->FieldOfView, P->Rotation);
		CurrentTrackingSpeed = P->TrackingInterpSpeed;
		bCachedFollowPawn = P->bFollowPawn;

		// 틸트 쉬프트 효과 적용
		if (P->bEnableTiltShift)
		{
			MainCameraActor->UpdatePostProcessSettings(
				P->ManualFocusDistance, P->ApertureFStop, P->SensorWidth,
				P->NearBlurRadius, P->FarBlurRadius, P->FarTransitionRegion
			);
		}
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