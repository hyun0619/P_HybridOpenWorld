#include "ProjectHEdgeScrollComponent.h"
#include "ProjectHCameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UProjectHEdgeScrollComponent::UProjectHEdgeScrollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

AProjectHCameraActor* UProjectHEdgeScrollComponent::GetCamera() const
{
    if (CachedCamera.IsValid()) return CachedCamera.Get();
    return nullptr;
}

void UProjectHEdgeScrollComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 카메라 유효성 검사 및 캐싱
    if (!CachedCamera.IsValid())
    {
       CachedCamera = Cast<AProjectHCameraActor>(
          UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
       if (!CachedCamera.IsValid()) return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    // 우클릭 입력 상태 확인
    const bool bKeyHeld = PC->IsInputKeyDown(ActivationKey);

    // 1. 우클릭을 떼고 있다면, 카메라를 원래 자리(ZeroVector)로 부드럽게 돌려보냅니다.
    if (!bKeyHeld)
    {
       CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, ReturnSpeed);
       CachedCamera->SetEdgeScrollOffset(CurrentOffset);
       return;
    }

    // 마우스 좌표와 뷰포트 크기를 가져옵니다.
    float MouseX, MouseY;
    if (!PC->GetMousePosition(MouseX, MouseY)) return;

    int32 SizeX, SizeY;
    PC->GetViewportSize(SizeX, SizeY);
    if (SizeX <= 0 || SizeY <= 0) return;

    // 2. 화면의 정중앙 좌표 계산
    FVector2D ScreenCenter(SizeX * 0.5f, SizeY * 0.5f);
    FVector2D MousePos(MouseX, MouseY);

    // 3. 화면 중앙에서 현재 마우스까지의 2D 방향 벡터 계산
    FVector2D DirFromCenter = MousePos - ScreenCenter;

    // 4. 화면 크기 대비 마우스가 얼마나 멀리 있는지 비율(0.0 ~ 1.0) 계산
    // 기준을 화면 세로 길이의 절반으로 잡습니다. (화면 끝에 닿으면 1.0)
    float MaxScreenDist = (SizeY * 0.5f) * ScreenDistanceMultiplier; 
    
    // 비율을 0에서 1 사이로 제한(Clamp)합니다. 마우스가 화면 밖으로 나가도 더 멀리 가지 않게 방지.
    float DistanceRatio = FMath::Clamp(DirFromCenter.Size() / MaxScreenDist, 0.0f, 1.0f);

    FVector TargetOffset = FVector::ZeroVector;

    // 마우스가 중앙에서 아주 조금이라도 움직였다면 패닝 시작
    if (DistanceRatio > 0.01f)
    {
        // 2D 마우스 방향을 정규화(길이를 1로 만듦)하여 순수한 '방향'만 남깁니다.
        DirFromCenter.Normalize();

        // 5. 2D 방향을 3D 월드 방향으로 변환하기 위해 카메라의 현재 회전값(Yaw)을 가져옵니다.
        const FRotator CamYaw(0.f, CachedCamera->GetCameraViewRotation().Yaw, 0.f);
        const FVector WorldForward = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::X); // 카메라 기준 앞
        const FVector WorldRight = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::Y);  // 카메라 기준 오른쪽

        // [중요 수학/기하학 개념]
        // 언리얼의 2D 화면 좌표는 위에서 아래로 갈수록 Y값이 '증가'합니다. (맨 위가 0)
        // 하지만 3D 월드에서 Forward(앞)로 가려면 2D 화면상에서는 마우스를 '위'로 올려야 하므로, 
        // 화면 Y축 방향(DirFromCenter.Y)을 반대로(-) 뒤집어 주어야 월드의 앞(WorldForward)과 방향이 맞습니다.
        FVector DesiredDirection = (-DirFromCenter.Y * WorldForward + DirFromCenter.X * WorldRight).GetSafeNormal();

        // 6. 최종 목표 오프셋 위치 = 방향 * 최대 허용 거리 * 현재 마우스 거리 비율
        TargetOffset = DesiredDirection * (MaxPanDistance * DistanceRatio);
    }

    // 7. 현재 위치에서 목표 위치로 프레임마다 부드럽게 이동(보간, VInterpTo)
    CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, PanInterpSpeed);

    // 계산된 최종 오프셋을 카메라 액터에 적용
    CachedCamera->SetEdgeScrollOffset(CurrentOffset);
}