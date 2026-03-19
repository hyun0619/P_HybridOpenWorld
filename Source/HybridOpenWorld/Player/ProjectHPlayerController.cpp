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


AProjectHPlayerController::AProjectHPlayerController()
{
	// 마우스 기본 설정
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	// 카메라 추적 시 화면 떨림 방지를 위해 틱 조정
	PrimaryActorTick.TickGroup = TG_PostPhysics;
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

void AProjectHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Enhanced Input 컴포넌트로 캐스팅하여 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// WASD 이동 (Triggered: 계속 누르고 있을 때)
		EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Triggered, this, &AProjectHPlayerController::HandleMove_KeyBoard);
		// 마우스 이동 (Started: 클릭한 순간)
		EnhancedInputComponent->BindAction(IA_Move_MouseClick, ETriggerEvent::Started, this, &AProjectHPlayerController::HandleMove_MouseClick);
		/*추후 추가될 기능들 자리*/
	}
}

void AProjectHPlayerController::SetInputModeByType(bool bIsWorldMap)
{
	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem) return;

	// 기존의 모든 키 매핑 초기화 - Global 우선 순위 0
	Subsystem->ClearAllMappings(); 
	if (IMC_Global) Subsystem->AddMappingContext(IMC_Global, 0); 
	
	if (bIsWorldMap)
	{
		SetupWorldMapInput();
	}
	else
	{
		SetupDetailedInput();
	}
}

void AProjectHPlayerController::HandleMove_KeyBoard(const FInputActionValue& Value)
{
	// Detailed 모드에서만 작동
	FVector2D MoveVector = Value.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn())
	{
		// 카메라가 보고 있는 방향을 기준으로 이동
		if (MainCameraActor)
		{
			// 카메라 회전값 중 Yaw만 추출하여 방향 계산
			const FRotator YawRotation(0, MainCameraActor->GetActorRotation().Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// X를 전진에 Y를 좌우에 매핑
			ControlledPawn->AddMovementInput(ForwardDirection, MoveVector.X);
			ControlledPawn->AddMovementInput(RightDirection, MoveVector.Y);
		}
	}
}

void AProjectHPlayerController::HandleMove_MouseClick()
{
	// WorldMap 모드에서만 작동
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		// 내비게이션 시스템을 이용해 클릭 지점으로 자동 이동
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
		// 클릭 지점에 Niagara 효과 생성
		if (FXCursor)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.ImpactPoint);
		}
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
	SetInputModeByType(CurrentLevelRow.LevelType == ELevelType::WorldMap); // 입력 모드 설정
	
	HandleInitialSpawn(); // 캐릭터 스폰 배치
	ApplyCameraPreset(); // 카메라 프리셋 적용
}

void AProjectHPlayerController::SetupWorldMapInput()
{
	if (IMC_WorldMap) {
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())->AddMappingContext(IMC_WorldMap, 1);
	}
        
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AProjectHPlayerController::SetupDetailedInput()
{
	if (IMC_Detailed) {
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())->AddMappingContext(IMC_Detailed, 1);
	}
        
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
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