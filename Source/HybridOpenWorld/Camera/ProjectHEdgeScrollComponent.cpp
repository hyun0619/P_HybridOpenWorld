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

	if (!CachedCamera.IsValid())
	{
		CachedCamera = Cast<AProjectHCameraActor>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
		if (!CachedCamera.IsValid()) return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;

	const bool bKeyHeld = PC->IsInputKeyDown(ActivationKey);

	if (!bKeyHeld)
	{
		CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, ReturnSpeed);
		CachedCamera->SetEdgeScrollOffset(CurrentOffset);
		return;
	}

	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY)) return;

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0) return;

	const float NormX = MouseX / static_cast<float>(SizeX);
	const float NormY = MouseY / static_cast<float>(SizeY);

	FVector2D ScrollDir = FVector2D::ZeroVector;

	if (NormX < EdgeThreshold)
		ScrollDir.X = -(1.0f - NormX / EdgeThreshold);
	else if (NormX > 1.0f - EdgeThreshold)
		ScrollDir.X = (NormX - (1.0f - EdgeThreshold)) / EdgeThreshold;

	if (NormY < EdgeThreshold)
		ScrollDir.Y = (1.0f - NormY / EdgeThreshold);
	else if (NormY > 1.0f - EdgeThreshold)
		ScrollDir.Y = -((NormY - (1.0f - EdgeThreshold)) / EdgeThreshold);

	if (ScrollDir.IsNearlyZero())
	{
		CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, ReturnSpeed);
		CachedCamera->SetEdgeScrollOffset(CurrentOffset);
		return;
	}

	const FRotator CamYaw(0.f, CachedCamera->GetCameraViewRotation().Yaw, 0.f);
	const FVector WorldForward = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::X);
	const FVector WorldRight = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::Y);

	const FVector DesiredDirection = (WorldForward * ScrollDir.Y + WorldRight * ScrollDir.X).GetSafeNormal();
	const float Intensity = FMath::Min(ScrollDir.Size(), 1.0f);

	CurrentOffset += DesiredDirection * Intensity * ScrollSpeed * DeltaTime;
	if (CurrentOffset.Size() > MaxOffset)
		CurrentOffset = CurrentOffset.GetSafeNormal() * MaxOffset;

	CachedCamera->SetEdgeScrollOffset(CurrentOffset);
}