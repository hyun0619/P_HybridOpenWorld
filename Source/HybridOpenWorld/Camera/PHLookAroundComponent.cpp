#include "PHLookAroundComponent.h"
#include "PHCameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Player/PHPlayerController.h"

UPHLookAroundComponent::UPHLookAroundComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // 동일 프레임의 카메라 업데이트에 오프셋 전달
}

void UPHLookAroundComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CachedPC = Cast<APHPlayerController>(GetOwner());
}

void UPHLookAroundComponent::SetLookAroundActive(bool bActive)
{
    bIsLookAroundActive = bActive;
}

// 컨트롤러가 이미 들고 있는 메인 카메라 반환
APHCameraActor* UPHLookAroundComponent::GetMainCamera() const
{
    return CachedPC.IsValid() ? CachedPC->GetMainCameraActor() : nullptr;
}

void UPHLookAroundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    APHCameraActor* CameraActor = GetMainCamera(); // 컨트롤러나 카메라가 유효하지 않으면 연산을 중단
    if (!CachedPC.IsValid() || !CameraActor) return;

    if (!bIsLookAroundActive) // 기능 사용 X일 때 - 서서히 원래 위치로 복귀
    {
        CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, ReturnSpeed);
        CameraActor->SetEdgeScrollOffset(CurrentOffset);
        return;
    }
    
    // 마우스 및 뷰포트 정보 획득
    float MouseX, MouseY;
    if (!CachedPC->GetMousePosition(MouseX, MouseY)) return;

    int32 SizeX, SizeY;
    CachedPC->GetViewportSize(SizeX, SizeY);
    if (SizeX <= 0 || SizeY <= 0) return;

    // 화면 중심~마우스까지의 벡터 계산
    FVector2D ScreenCenter(SizeX * 0.5f, SizeY * 0.5f);
    FVector2D MousePos(MouseX, MouseY);
    FVector2D DirFromCenter = MousePos - ScreenCenter;

    // 거리 비율 계산 - 중심으로 멀어질수록 값 커짐
    float MaxScreenDist = (SizeY * 0.5f) * ScreenDistanceMultiplier; 
    float DistanceRatio = FMath::Clamp(DirFromCenter.Size() / MaxScreenDist, 0.0f, 1.0f);

    FVector TargetOffset = FVector::ZeroVector;

    // 마우스가 중심에서 일정 거리 이상 벗어났을 때만 계산 시작
    if (DistanceRatio > 0.01f)
    {
        DirFromCenter.Normalize(); // 방향 추출

        /* 화면의 상하는 월드의 Forward, 화면의 좌우는 월드의 Right - 기준 카메라의 회전 값 */
        const FRotator CamYaw(0.f, CameraActor->GetCameraViewRotation().Yaw, 0.f);
        const FVector WorldForward = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::X); 
        const FVector WorldRight = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::Y);  

        // 화면 좌표를 월드 좌표에 맞춰 반전 - 방향 결정
        FVector DesiredDirection = (-DirFromCenter.Y * WorldForward + DirFromCenter.X * WorldRight).GetSafeNormal();

        TargetOffset = DesiredDirection * (MaxPanDistance * DistanceRatio);
    }
    // 오프셋 보간 후 카메라 액터에 전달 
    CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, PanInterpSpeed);
    CameraActor->SetEdgeScrollOffset(CurrentOffset); // 카메라 룩어라운드 기능으로 시야 확장 적용
}