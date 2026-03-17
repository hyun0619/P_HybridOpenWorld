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
    
    UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(GetGameInstance());
    MainCameraActor = Cast<AProjectHCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
    
    bHasValidLevelData = false;
	
	// 레벨 데이터
    if (MasterLevelSettings && MasterLevelSettings->LevelTable)
    {
        FString CurrentMapName = GetWorld()->GetMapName();
        CurrentMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
        
        static const FString ContextString(TEXT("LevelContext"));
        TArray<FLevelSettingsRow*> AllRows;
        MasterLevelSettings->LevelTable->GetAllRows<FLevelSettingsRow>(ContextString, AllRows);
        
        for (FLevelSettingsRow* Row : AllRows)
        {
            if (Row && Row->LevelReference.GetAssetName() == CurrentMapName)
            {
                CurrentLevelRow = *Row;
                bHasValidLevelData = true;
                break;
            }
        }
    }
    
	// 태그 스폰
    if (bHasValidLevelData && MainCameraActor)
    {
        SetViewTarget(MainCameraActor);
        SetInputModeByType(CurrentLevelRow.LevelType == ELevelType::WorldMap);
        
        if (APawn* P = GetPawn())
        {
            FVector TargetLoc;
            if (GI && GI->PendingSpawnTag.IsValid() && CurrentLevelRow.SpawnLocations.Contains(GI->PendingSpawnTag))
            {
                TargetLoc = CurrentLevelRow.SpawnLocations[GI->PendingSpawnTag];
            }
            else
            {
                TargetLoc = CurrentLevelRow.DefaultSpawnLocation;
            }

            P->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
            MainCameraActor->SetActorLocation(TargetLoc);
          
            if (GI) GI->PendingSpawnTag = FGameplayTag::EmptyTag;
        }

        // 카메라 프리셋 적용
        if (CurrentLevelRow.CameraPreset)
        {
            auto* P = CurrentLevelRow.CameraPreset;
            MainCameraActor->UpdateCameraSettings(P->TargetArmLength, P->FieldOfView, P->Rotation);
            CurrentTrackingSpeed = P->TrackingInterpSpeed;
            bCachedFollowPawn = P->bFollowPawn;

            if (P->bEnableTiltShift)
            {
                MainCameraActor->UpdatePostProcessSettings(
                    P->ManualFocusDistance, P->ApertureFStop, P->SensorWidth,
                    P->NearBlurRadius, P->FarBlurRadius, P->FarTransitionRegion
                );
            }
        }
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
		// 월드맵 - 마우스 이동용 IMC 등록
		if (IMC_WorldMap) Subsystem->AddMappingContext(IMC_WorldMap, 1);
        
		// 마우스 커서가 자유롭게 움직이고 UI 상호작용이 가능한 모드로 설정
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		// 세부지역 - WASD 이동용 IMC 등록
		if (IMC_Detailed) Subsystem->AddMappingContext(IMC_Detailed, 1);
        
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
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