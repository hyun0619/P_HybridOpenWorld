#include "ProjectHPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/ProjectHCameraActor.h"
#include "Camera/ProjectHEdgeScrollComponent.h"
#include "Camera/ProjectHOcclusionFadeComponent.h"
#include "Kismet/GameplayStatics.h"
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
	EdgeScrollComponent = CreateDefaultSubobject<UProjectHEdgeScrollComponent>(TEXT("EdgeScrollComponent"));
	OcclusionFadeComponent = CreateDefaultSubobject<UProjectHOcclusionFadeComponent>(TEXT("OcclusionFadeComponent"));
}

void AProjectHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitEssentialReferences();
	FetchLevelData();
	if (bHasValidLevelData && MainCameraActor)
		ApplyInitialLevelSetup();
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
	if (!IsValid(MasterLevelSettings)) { UE_LOG(LogTemp, Error, TEXT("MasterLevelSettings 유효하지 않음")); return; }
	if (!IsValid(MasterLevelSettings->LevelTable)) { UE_LOG(LogTemp, Error, TEXT("LevelTable 유효하지 않음")); return; }

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	static const FString Ctx(TEXT("LevelLookupContext"));
	if (FLevelSettingsRow* Row = MasterLevelSettings->LevelTable->FindRow<FLevelSettingsRow>(FName(*MapName), Ctx))
	{
		CurrentLevelRow = *Row;
		bHasValidLevelData = true;
	}
}

void AProjectHPlayerController::ApplyInitialLevelSetup()
{
	SetViewTarget(MainCameraActor);

	switch (CurrentLevelRow.LevelType)
	{
	case ELevelType::WorldMap:   DefaultState = EInputState::WorldMap; break;
	case ELevelType::Detailed:   DefaultState = EInputState::Detailed; break;
	case ELevelType::Event:      DefaultState = EInputState::Cinematic; break;
	default:                     DefaultState = EInputState::Detailed; break;
	}
	ChangeInputState(DefaultState);

	if (MainCameraActor && CurrentLevelRow.CameraPreset)
		if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
			Sub->SetDefaultPreset(CurrentLevelRow.CameraPreset);

	HandleInitialSpawn();
}

void AProjectHPlayerController::HandleInitialSpawn()
{
	auto* GI = Cast<UProjectHGameInstance>(GetGameInstance());
	APawn* P = GetPawn();
	if (!P || !GI) return;

	FVector Loc = CurrentLevelRow.DefaultSpawnLocation;
	if (GI->PendingSpawnTag.IsValid() && CurrentLevelRow.SpawnLocations.Contains(GI->PendingSpawnTag))
		Loc = CurrentLevelRow.SpawnLocations[GI->PendingSpawnTag];

	P->SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
	MainCameraActor->SetActorLocation(Loc);
	GI->PendingSpawnTag = FGameplayTag::EmptyTag;
}

void AProjectHPlayerController::ChangeInputState(EInputState NewState)
{
	CurrentState = NewState;
	if (InputManager) InputManager->ApplyInputState(this, CurrentState);
}

void AProjectHPlayerController::RevertToDefaultState()
{
	ChangeInputState(DefaultState);
}

void AProjectHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 디테일 패널에서 IA_LookAround가 잘 할당되어 있는지 확인
		if (IA_LookAround)
		{
			// Started = 우클릭 누르기 시작할 때
			EnhancedInputComp->BindAction(IA_LookAround, ETriggerEvent::Started, this, &AProjectHPlayerController::OnLookAroundStarted);
            
			// Completed = 우클릭을 떼었을 때
			EnhancedInputComp->BindAction(IA_LookAround, ETriggerEvent::Completed, this, &AProjectHPlayerController::OnLookAroundCompleted);
		}
	}
}

void AProjectHPlayerController::OnLookAroundStarted(const FInputActionValue& Value)
{
	if (EdgeScrollComponent)
	{
		EdgeScrollComponent->SetLookAroundActive(true);
	}
}

void AProjectHPlayerController::OnLookAroundCompleted(const FInputActionValue& Value)
{
	if (EdgeScrollComponent)
	{
		EdgeScrollComponent->SetLookAroundActive(false);
	}
}