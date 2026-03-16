#include "ProjectHPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/ProjectHCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Data/LevelDataAsset.h"
#include "Data/CameraPresetDataAsset.h"


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
    
	// 전수 조사 대신 첫 번째 액터만 바로 가져오기
	MainCameraActor = Cast<AProjectHCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
    
	if (MainCameraActor)
	{
		SetViewTarget(MainCameraActor);
       
		// 데이터 에셋 우선, 없으면 맵 이름으로 판정
		bool bIsWorldMap = false;
		if (CurrentLevelData)
		{
			bIsWorldMap = (CurrentLevelData->LevelType == ELevelType::WorldMap);
		}
		else
		{
			bIsWorldMap = GetWorld()->GetMapName().Contains(TEXT("World"), ESearchCase::IgnoreCase);
		}
        
		SetInputModeByType(bIsWorldMap);
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

	Subsystem->ClearAllMappings(); 
	if (IMC_Global) Subsystem->AddMappingContext(IMC_Global, 0); 
    
	if (bIsWorldMap)
	{
		if (IMC_WorldMap) Subsystem->AddMappingContext(IMC_WorldMap, 1);
        
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		if (IMC_Detailed) Subsystem->AddMappingContext(IMC_Detailed, 1);
        
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}

	// 데이터 에셋 기반 카메라 설정 업데이트
	if (MainCameraActor && CurrentLevelData && CurrentLevelData->CameraPreset)
	{
		auto* P = CurrentLevelData->CameraPreset;
		MainCameraActor->UpdateCameraSettings(P->TargetArmLength, P->FieldOfView, P->Rotation);
		
		CurrentTrackingSpeed = P->TrackingInterpSpeed; // 데이터 에셋에 설정된 속도값을 컨트롤러 변수에 저장
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

			ControlledPawn->AddMovementInput(ForwardDirection, MoveVector.Y);
			ControlledPawn->AddMovementInput(RightDirection, MoveVector.X);
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
    
	if (MainCameraActor && GetPawn())
	{
		// [최적화] 불필요한 GetActorLocation 호출을 줄이고 인터폴레이션 수행
		const FVector TargetLoc = GetPawn()->GetActorLocation();
		const FVector CurrentLoc = MainCameraActor->GetActorLocation();
        
		// 5.0f는 추후 프리셋 데이터로 빼면 더 좋습니다!
		MainCameraActor->SetActorLocation(FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, CurrentTrackingSpeed));
	}
}
