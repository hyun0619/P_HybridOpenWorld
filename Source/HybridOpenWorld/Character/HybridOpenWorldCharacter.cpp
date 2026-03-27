#include "HybridOpenWorldCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/ProjectHPlayerController.h"
#include "Camera/ProjectHCameraActor.h"


AHybridOpenWorldCharacter::AHybridOpenWorldCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AHybridOpenWorldCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

// 캐릭터가 컨트롤러를 부여받는 시점에 PC를 미리 캐싱
void AHybridOpenWorldCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 시작 시점에 딱 한 번만 Cast 연산을 수행하여 성능 확보
	CachedPC = Cast<AProjectHPlayerController>(NewController);
}

void AHybridOpenWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Enhanced Input 컴포넌트로 캐스팅하여 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// WASD 이동 (Triggered: 계속 누르고 있을 때)
		EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Triggered, this, &AHybridOpenWorldCharacter::HandleMove_KeyBoard);
		// 마우스 이동 (Started: 클릭한 순간)
		EnhancedInputComponent->BindAction(IA_Move_MouseClick, ETriggerEvent::Started, this, &AHybridOpenWorldCharacter::HandleMove_MouseClick);
	}
}

void AHybridOpenWorldCharacter::HandleMove_KeyBoard(const FInputActionValue& Value)
{
	FVector2D MoveVector = Value.Get<FVector2D>();
	
	// Cast 연산 제거. 미리 저장해둔 PC 유효한지만 체크
	if (CachedPC.IsValid())
	{
		// PC에서 메인 카메라 액터를 가져옴
		if (AProjectHCameraActor* MainCamera = CachedPC->GetMainCameraActor()) 
		{
			const FRotator YawRotation(0, MainCamera->GetCameraViewRotation().Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(ForwardDirection, MoveVector.X);
			AddMovementInput(RightDirection, MoveVector.Y);
		}
	}
}

void AHybridOpenWorldCharacter::HandleMove_MouseClick()
{
	if (AProjectHPlayerController* PC = Cast<AProjectHPlayerController>(GetController()))
	{
		FHitResult Hit;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
		{
			// 내비게이션 시스템을 이용해 클릭 지점으로 자동 이동
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, Hit.ImpactPoint);
            
			if (FXCursor) // 클릭 지점에 Niagara 효과 생성
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.ImpactPoint);
			}
		}
	}
}