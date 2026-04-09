#include "HybridOpenWorldCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/PHPlayerController.h"
#include "Camera/PHCameraActor.h"

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

/* 캐릭터가 플레이어 컨트롤러를 부여받는 시점 */
void AHybridOpenWorldCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    // 시작 시점에 딱 한 번만 Cast 연산 수행 -> 성능 확보
    CachedPC = Cast<APHPlayerController>(NewController);
}

void AHybridOpenWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
       // WASD 이동 - Triggered: 계속 누르고 있거나 입력 값이 변할 때마다 호출
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Triggered,
       	this, &AHybridOpenWorldCharacter::HandleMove_KeyBoard);
       
       // 키에서 완전히 손을 떼거나 입력이 취소되었을 때 고정 해제
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Completed, this,
       	&AHybridOpenWorldCharacter::ResetMoveLatch);
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Canceled, this,
       	&AHybridOpenWorldCharacter::ResetMoveLatch);
       
    	// 마우스 클릭 이동 - Started: 클릭한 순간 1회
       EnhancedInputComponent->BindAction(IA_Move_MouseClick, ETriggerEvent::Started, this,
       	&AHybridOpenWorldCharacter::HandleMove_MouseClick);
    }
}

/* 키보드 이동 로직 - 조작감 보정 적용 */
void AHybridOpenWorldCharacter::HandleMove_KeyBoard(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();
	
    // 조작감 보정 로직 - 입력 조합의 변화 감지, Equals를 사용하여 오차를 무시하고 확실한 의도적 입력 변화 체크
    if (!MoveVector.Equals(LastMoveVector, 0.01f))
    {
    	// 입력이 변하면 현재의 카메라 각도를 다시 읽어야 하므로 해제
        bIsInputLatched = false;
    }
    LastMoveVector = MoveVector; // 다음 프레임 비교를 위해 현재 입력 저장

    if (CachedPC.IsValid())
    {
       if (APHCameraActor* MainCamera = CachedPC->GetMainCameraActor()) 
       {
          if (!bIsInputLatched) // 방향 고정이 안 된 상태라면 현재 카메라 기준으로 월드 벡터 계산
          {
             // 현재 카메라가 바라보는 월드 기준 방향 구함
             const FRotator YawRotation(0, MainCamera->GetCameraViewRotation().Yaw, 0);
             LatchedForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
             LatchedRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
             
             // 방향 고정 - 다음 입력 변화가 있을 때까지 이 카메라 기준 유지
             bIsInputLatched = true;
          }
          // 최종 이동 입력
          AddMovementInput(LatchedForward, MoveVector.X);
          AddMovementInput(LatchedRight, MoveVector.Y);
       }
    }
}

/* 키에서 모든 손을 떼었을 때 호출 */
void AHybridOpenWorldCharacter::ResetMoveLatch(const FInputActionValue& Value)
{
    bIsInputLatched = false;
    LastMoveVector = FVector2D::ZeroVector;
}

/* 마우스 클릭 이동 로직 */
void AHybridOpenWorldCharacter::HandleMove_MouseClick()
{
    if (CachedPC.IsValid())
    {
       FHitResult Hit;
       if (CachedPC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
       {
          // 내비게이션 시스템 -> 클릭 지점으로 자동 이동, NavMesh 필요
          UAIBlueprintHelperLibrary::SimpleMoveToLocation(CachedPC.Get(), Hit.ImpactPoint);
       	
          if (FXCursor) // 클릭한 위치 시각적 피드백 스폰
          {
             UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.ImpactPoint);
          }
       }
    }
}