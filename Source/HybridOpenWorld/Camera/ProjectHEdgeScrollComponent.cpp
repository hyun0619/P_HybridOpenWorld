#include "ProjectHEdgeScrollComponent.h"
#include "ProjectHCameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ProjectHPlayerController.h"

UProjectHEdgeScrollComponent::UProjectHEdgeScrollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UProjectHEdgeScrollComponent::BeginPlay()
{
    Super::BeginPlay();

    // 컴포넌트의 주인이 PlayerController이므로 시작 시점에 딱 한 번만 Cast해서 캐싱
    CachedPC = Cast<AProjectHPlayerController>(GetOwner());
}

void UProjectHEdgeScrollComponent::SetLookAroundActive(bool bActive)
{
    bIsLookAroundActive = bActive;
}

// 컨트롤러가 이미 들고 있는 메인 카메라 반환
AProjectHCameraActor* UProjectHEdgeScrollComponent::GetMainCamera() const
{
    if (CachedPC.IsValid())
    {
        return CachedPC->GetMainCameraActor();
    }
    return nullptr;
}

void UProjectHEdgeScrollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 컨트롤러나 카메라가 유효하지 않으면 연산을 중단
    AProjectHCameraActor* CameraActor = GetMainCamera();
    if (!CachedPC.IsValid() || !CameraActor) return;

    if (!bIsLookAroundActive)
    {
        CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, ReturnSpeed);
        CameraActor->SetEdgeScrollOffset(CurrentOffset);
        return;
    }

    float MouseX, MouseY;
    if (!CachedPC->GetMousePosition(MouseX, MouseY)) return;

    int32 SizeX, SizeY;
    CachedPC->GetViewportSize(SizeX, SizeY);
    if (SizeX <= 0 || SizeY <= 0) return;

    FVector2D ScreenCenter(SizeX * 0.5f, SizeY * 0.5f);
    FVector2D MousePos(MouseX, MouseY);
    FVector2D DirFromCenter = MousePos - ScreenCenter;

    float MaxScreenDist = (SizeY * 0.5f) * ScreenDistanceMultiplier; 
    float DistanceRatio = FMath::Clamp(DirFromCenter.Size() / MaxScreenDist, 0.0f, 1.0f);

    FVector TargetOffset = FVector::ZeroVector;

    if (DistanceRatio > 0.01f)
    {
        DirFromCenter.Normalize();

        const FRotator CamYaw(0.f, CameraActor->GetCameraViewRotation().Yaw, 0.f);
        const FVector WorldForward = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::X); 
        const FVector WorldRight = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::Y);  

        FVector DesiredDirection = (-DirFromCenter.Y * WorldForward + DirFromCenter.X * WorldRight).GetSafeNormal();

        TargetOffset = DesiredDirection * (MaxPanDistance * DistanceRatio);
    }

    CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, PanInterpSpeed);
    CameraActor->SetEdgeScrollOffset(CurrentOffset);
}