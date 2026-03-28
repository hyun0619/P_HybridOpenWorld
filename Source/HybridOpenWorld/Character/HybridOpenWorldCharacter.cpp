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
    
    // 탑다운/쿼터뷰 게임이므로 컨트롤러 회전에 캐릭터가 같이 돌지 않도록 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    // 이동하는 방향을 자연스럽게 바라보도록 무브먼트 컴포넌트 설정
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

// 캐릭터가 플레이어 컨트롤러를 부여받는 시점
void AHybridOpenWorldCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // 시작 시점에 딱 한 번만 Cast 연산을 수행하여 성능을 확보 (매 프레임 Cast 방지)
    CachedPC = Cast<AProjectHPlayerController>(NewController);
}

void AHybridOpenWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
       // 1. WASD 이동 (Triggered: 계속 누르고 있거나 입력 값이 변할 때마다 호출)
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Triggered, this, &AHybridOpenWorldCharacter::HandleMove_KeyBoard);
       
       // 2. [핵심] 키에서 완전히 손을 떼거나 입력이 취소되었을 때 고정(Latch) 해제
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Completed, this, &AHybridOpenWorldCharacter::ResetMoveLatch);
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Canceled, this, &AHybridOpenWorldCharacter::ResetMoveLatch);

       // 3. 마우스 클릭 이동 (Started: 클릭한 순간 1회)
       EnhancedInputComponent->BindAction(IA_Move_MouseClick, ETriggerEvent::Started, this, &AHybridOpenWorldCharacter::HandleMove_MouseClick);
    }
}

void AHybridOpenWorldCharacter::HandleMove_KeyBoard(const FInputActionValue& Value)
{
    // 현재 입력값 (예: W 누름 -> X:1, Y:0 / WA 누름 -> X:1, Y:-1)
    FVector2D MoveVector = Value.Get<FVector2D>();

    // ════════════════════════════════════════════════════════════════
    // ★ [스마트 래치 로직] 입력 조합의 변화를 감지 (W -> WA 등)
    // Equals를 사용하여 오차를 무시하고 확실한 의도적 입력 변화를 체크합니다.
    // ════════════════════════════════════════════════════════════════
    if (!MoveVector.Equals(LastMoveVector, 0.01f))
    {
        // 사용자가 새로운 키를 누르거나 기존 키를 뗐으므로,
        // 현재 활성화된(돌아가고 있는) 카메라 뷰에 맞춰 조작 방향을 즉시 재보정합니다.
        bIsInputLatched = false;
    }
    
    // 다음 프레임 비교를 위해 현재 입력을 저장
    LastMoveVector = MoveVector;

    if (CachedPC.IsValid())
    {
       if (AProjectHCameraActor* MainCamera = CachedPC->GetMainCameraActor()) 
       {
          // 방향이 고정되지 않았을 때(새로운 입력이 감지되었을 때)만 카메라 Yaw를 새로 읽음
          if (!bIsInputLatched)
          {
             // 현재 카메라가 바라보는 월드 기준 방향을 구함
             const FRotator YawRotation(0, MainCamera->GetCameraViewRotation().Yaw, 0);
             LatchedForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
             LatchedRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
             
             // 방향 고정 (다음 입력 변화가 있을 때까지 이 카메라 기준 유지)
             bIsInputLatched = true;
          }

          // 고정된 카메라 방향 벡터에 현재 입력(MoveVector)을 적용해 캐릭터를 이동
          // WA 대각선 입력의 경우 MoveVector.X와 Y가 모두 존재하므로 대각선으로 자연스럽게 이동됨
          AddMovementInput(LatchedForward, MoveVector.X);
          AddMovementInput(LatchedRight, MoveVector.Y);
       }
    }
}

// 키에서 모든 손을 떼었을 때 호출 (Completed, Canceled)
void AHybridOpenWorldCharacter::ResetMoveLatch(const FInputActionValue& Value)
{
    // 완전히 키를 떼었으므로 다음 이동 시 무조건 새로운 카메라 뷰를 기준으로 잡게 함
    bIsInputLatched = false;
    LastMoveVector = FVector2D::ZeroVector;
}

void AHybridOpenWorldCharacter::HandleMove_MouseClick()
{
    if (CachedPC.IsValid())
    {
       FHitResult Hit;
       // Visibility 채널을 기준으로 마우스가 클릭한 바닥의 좌표를 찾음
       if (CachedPC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
       {
          // 내비게이션 시스템을 이용해 클릭 지점으로 자동 이동 (NavMesh 필요)
          UAIBlueprintHelperLibrary::SimpleMoveToLocation(CachedPC.Get(), Hit.ImpactPoint);
            
          // 클릭한 위치에 시각적 피드백(Niagara Particle) 스폰
          if (FXCursor) 
          {
             UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.ImpactPoint);
          }
       }
    }
}