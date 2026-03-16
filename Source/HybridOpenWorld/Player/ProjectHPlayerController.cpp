#include "ProjectHPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "ProjectHCameraActor.h" 
#include "Kismet/GameplayStatics.h"


AProjectHPlayerController::AProjectHPlayerController()
{
	// 마우스 기본 설정
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AProjectHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에 배치된 전용 카메라 액터 자동 연결
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectHCameraActor::StaticClass(), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		MainCameraActor = Cast<AProjectHCameraActor>(FoundCameras[0]);
		SetViewTarget(MainCameraActor); // 찾은 카메라로 화면 연결
	}
	
	// 초기 카메라 배정 로직 (나중에 Spawn이나 FindActor 등으로 구현 필요)
	
	SetInputModeByType(true); // 초기 입력 모드 설정
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
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings(); // 기존 모든 매핑을 한번에 정리
		
		if (IMC_Global) Subsystem->AddMappingContext(IMC_Global, 0); // 공통 기능은 최우선으로
		
		if (IMC_WorldMap)
		{
			// 월드맵 모드 - 마우스 이동 활성화
			if (IMC_WorldMap) Subsystem->AddMappingContext(IMC_WorldMap, 1);
			bShowMouseCursor = true;
			// 카메라 설정 변경
			if (MainCameraActor) MainCameraActor->SetCameraMode(true);
		}
		else
		{
			// 세부지역 모드 - WASD 이동 활성화
			if (IMC_Detailed) Subsystem->AddMappingContext(IMC_Detailed, 1);
			bShowMouseCursor = false;
			if (MainCameraActor) MainCameraActor->SetCameraMode(false);
		}
	}
}

void AProjectHPlayerController::HandleMove_KeyBoard(const FInputActionValue& Value)
{
	// Detailed 모드에서만 작동함
	FVector2D MoveVector = Value.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn())
	{
		// 카메라의 전방 방향을 기준으로 이동
		ControlledPawn->AddMovementInput(FVector::ForwardVector, MoveVector.Y);
		ControlledPawn->AddMovementInput(FVector::RightVector, MoveVector.X);
	}
}

void AProjectHPlayerController::HandleMove_MouseClick()
{
	// WorldMap 모드에서만 작동함 (IMC에 의해 필터링됨)
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		// 내비게이션 시스템을 이용해 클릭 지점으로 자동 이동
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
	}
}

void AProjectHPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}
