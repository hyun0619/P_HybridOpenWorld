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
	
	if (AProjectHPlayerController* PC = Cast<AProjectHPlayerController>(GetController()))
	{
		if (AProjectHCameraActor* MainCamera = PC->GetMainCameraActor()) // 카메라가 보고 있는 방향을 기준으로 이동
		{
			// ══════════════════════════════════════════════════════
			// ★ 핵심 버그 수정
			//
			// [이전 코드 - 버그]
			//   MainCamera->GetActorRotation().Yaw
			//   → CameraActor의 루트 컴포넌트 회전을 읽음
			//   → 볼륨 전환 시 SpringArm이 별도로 보간하면서
			//     "화면에 보이는 방향"과 "이동 기준 방향"이 어긋남
			//
			// [수정 코드]
			//   MainCamera->GetCameraViewRotation().Yaw
			//   → SpringArm의 월드 회전을 읽음
			//   → 화면에 실제로 보이는 카메라 방향 = 이동 기준 방향
			//   → 어떤 볼륨에 있든 WASD가 화면 기준 상하좌우로 작동
			// ══════════════════════════════════════════════════════
			
			// 카메라 회전값 중 Yaw만 추출하여 방향 계산
			const FRotator YawRotation(0, MainCamera->GetCameraViewRotation().Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// X를 전진에 Y를 좌우에 매핑
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