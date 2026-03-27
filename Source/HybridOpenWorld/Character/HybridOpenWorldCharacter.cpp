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

void AHybridOpenWorldCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    CachedPC = Cast<AProjectHPlayerController>(NewController);
}

void AHybridOpenWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
       // Latch 관련 Completed/Canceled 삭제하고 원래대로 Triggered 하나만 남김
       EnhancedInputComponent->BindAction(IA_Move_KeyBoard, ETriggerEvent::Triggered, this, &AHybridOpenWorldCharacter::HandleMove_KeyBoard);
       EnhancedInputComponent->BindAction(IA_Move_MouseClick, ETriggerEvent::Started, this, &AHybridOpenWorldCharacter::HandleMove_MouseClick);
    }
}

void AHybridOpenWorldCharacter::HandleMove_KeyBoard(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();

    if (CachedPC.IsValid())
    {
       if (AProjectHCameraActor* MainCamera = CachedPC->GetMainCameraActor()) 
       {
          // 복잡한 Latch 로직 모두 제거, 매 프레임 즉각적으로 현재 카메라 방향 갱신
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
    if (CachedPC.IsValid())
    {
       FHitResult Hit;
       if (CachedPC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
       {
          UAIBlueprintHelperLibrary::SimpleMoveToLocation(CachedPC.Get(), Hit.ImpactPoint);
            
          if (FXCursor) 
          {
             UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.ImpactPoint);
          }
       }
    }
}