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
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;

	InputManager = CreateDefaultSubobject<UProjectHInputComponent>(TEXT("InputManager"));
}

void AProjectHPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitEssentialReferences();
	FetchLevelData();

	if (bHasValidLevelData && MainCameraActor)
	{
		ApplyInitialLevelSetup();
	}
}

void AProjectHPlayerController::InitEssentialReferences()
{
	MainCameraActor = Cast<AProjectHCameraActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
}

AProjectHCameraActor* AProjectHPlayerController::GetMainCameraActor() const
{
	return MainCameraActor;
}

void AProjectHPlayerController::FetchLevelData()
{
	if (!IsValid(MasterLevelSettings))
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
	SetViewTarget(MainCameraActor);

	// 레벨 타입에 따른 기본 입력 상태 결정
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
		DefaultState = EInputState::Detailed;
		break;
	}
	ChangeInputState(DefaultState);

	// ★ 서브시스템에 기본 카메라 DA 등록 (기존 호환)
	if (MainCameraActor && CurrentLevelRow.CameraPreset)
	{
		if (UProjectHCameraSubsystem* CameraSubsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		{
			CameraSubsystem->SetDefaultPreset(CurrentLevelRow.CameraPreset);
		}
	}

	HandleInitialSpawn();
}

void AProjectHPlayerController::HandleInitialSpawn()
{
	UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(GetGameInstance());
	APawn* P = GetPawn();

	if (P && GI)
	{
		FVector TargetLoc;
		if (GI->PendingSpawnTag.IsValid() && CurrentLevelRow.SpawnLocations.Contains(GI->PendingSpawnTag))
		{
			TargetLoc = CurrentLevelRow.SpawnLocations[GI->PendingSpawnTag];
		}
		else
		{
			TargetLoc = CurrentLevelRow.DefaultSpawnLocation;
		}

		P->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
		MainCameraActor->SetActorLocation(TargetLoc);

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