#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h"
#include "Data/CameraPresetDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

AProjectHCameraActor::AProjectHCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm);

	StreamingSourceComponent = CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>(TEXT("StreamingSourceComponent"));

	SpringArm->bEnableCameraLag = bUseCameraLag;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
	SpringArm->CameraLagMaxDistance = 1000.0f;
}

void AProjectHCameraActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentOrthoWidth = MainCamera->OrthoWidth;
}

// ──────────────────────────────────────────────────
// 레거시 호환 API
// ──────────────────────────────────────────────────

void AProjectHCameraActor::UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation)
{
	if (SpringArm && MainCamera)
	{
		SpringArm->TargetArmLength = TargetArmLength;
		SpringArm->SetRelativeRotation(Rotation);
		MainCamera->SetFieldOfView(FOV);
	}
}

void AProjectHCameraActor::UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop,
	float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition)
{
	if (!MainCamera) return;

	FPostProcessSettings& PP = MainCamera->PostProcessSettings;

	PP.bOverride_DepthOfFieldFstop = bEnable;
	PP.bOverride_DepthOfFieldSensorWidth = bEnable;
	PP.bOverride_DepthOfFieldFocalDistance = bEnable;
	PP.bOverride_DepthOfFieldNearBlurSize = bEnable;
	PP.bOverride_DepthOfFieldFarBlurSize = bEnable;
	PP.bOverride_DepthOfFieldFarTransitionRegion = bEnable;

	if (bEnable)
	{
		PP.DepthOfFieldFstop = InFStop;
		PP.DepthOfFieldSensorWidth = InSensorWidth;
		PP.DepthOfFieldFocalDistance = InFocalDist;
		PP.DepthOfFieldNearBlurSize = InNearBlur;
		PP.DepthOfFieldFarBlurSize = InFarBlur;
		PP.DepthOfFieldFarTransitionRegion = InFarTransition;
	}
}

FVector AProjectHCameraActor::GetCameraTargetLocation() const
{
	FVector CameraLoc = MainCamera->GetComponentLocation();
	FVector ForwardDir = MainCamera->GetForwardVector();
	FVector IntersectionPoint;

	bool bIntersect = FMath::SegmentPlaneIntersection(
		CameraLoc, CameraLoc + (ForwardDir * 10000.f),
		FPlane(FVector::UpVector, 0.f), IntersectionPoint);

	return bIntersect ? IntersectionPoint : (CameraLoc + ForwardDir * 2000.f);
}

// ──────────────────────────────────────────────────
// 메인 Tick
// ──────────────────────────────────────────────────

void AProjectHCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
	if (!Subsystem) return;

	FCameraPresetSettings TargetPreset;
	if (!Subsystem->GetActivePreset(TargetPreset)) return;

	// ─── 볼륨 변경 감지 ──────────────────────

	AActor* ActiveInstigator = Subsystem->GetActiveInstigator();
	AProjectHCameraVolume* ActiveVolume = Cast<AProjectHCameraVolume>(ActiveInstigator);

	bool bVolumeChanged = (LastVolume != ActiveInstigator);
	LastVolume = ActiveInstigator;
	bool bHardCut = bIsFirstTick || (bVolumeChanged && TargetPreset.BlendTime <= 0.0f);
	bIsTransitioning = bVolumeChanged && !bHardCut;
	bIsFirstTick = false;

	// ★ 캐싱: PlayerController/Pawn 참조
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// ─── [1. 타겟 위치 계산] ────────────────

	FVector TargetLoc;

	if (TargetPreset.VolumeType == ECameraVolumeType::Static && ActiveVolume)
	{
		// ★ Static 모드: 볼륨 기준 고정 위치
		TargetLoc = ActiveVolume->GetVolumeCenter() + ActiveVolume->GetActorRotation().RotateVector(TargetPreset.StaticCameraOffset);
	}
	else
	{
		// Dynamic 모드: 기존 로직
		TargetLoc = ActiveInstigator ? ActiveInstigator->GetActorLocation() : GetActorLocation();
		if (TargetPreset.bFollowPawn && PlayerPawn)
		{
			TargetLoc = PlayerPawn->GetActorLocation() + TargetPreset.FocusPointOffset;
		}
	}

	// ─── [2. 바운드 클램핑] ─────────────────

	if (TargetPreset.bEnableBoundsBlocking && ActiveVolume)
	{
		TargetLoc = ClampToBounds(TargetLoc, ActiveVolume, TargetPreset);
	}

	// ─── [3. Blueprint 수정 기회] ────────────

	TargetLoc = BP_ModifyTargetLocation(TargetLoc, TargetPreset);

	// ─── [4. 타겟 회전 계산] ────────────────

	FRotator TargetRot = TargetPreset.Rotation;

	if (TargetPreset.VolumeType == ECameraVolumeType::Static)
	{
		TargetRot = TargetPreset.StaticCameraRotation;
		if (ActiveVolume)
		{
			// 볼륨 회전을 기준으로 카메라 회전 적용
			TargetRot = ActiveVolume->GetActorTransform().TransformRotation(TargetPreset.StaticCameraRotation.Quaternion()).Rotator();
		}
	}
	else if (ActiveInstigator)
	{
		TargetRot = ActiveInstigator->GetActorTransform().TransformRotation(TargetPreset.Rotation.Quaternion()).Rotator();
	}

	// ★ 추가 카메라 롤 적용
	if (FMath::Abs(TargetPreset.CameraRoll) > KINDA_SMALL_NUMBER)
	{
		TargetRot.Roll += TargetPreset.CameraRoll;
	}

	// Pawn Control Rotation
	if (TargetPreset.bUsePawnControlRotation && PlayerPawn && PlayerPawn->GetController())
	{
		TargetRot = PlayerPawn->GetControlRotation();
	}

	// ─── [5. 래그/충돌/투영 설정 적용] ──────

	ApplyLagSettings(TargetPreset, bHardCut);
	ApplyCollisionSettings(TargetPreset);

	// ─── [6. 실제 값 적용 및 보간] ──────────

	if (bHardCut)
	{
		SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);

		if (TargetPreset.VolumeType == ECameraVolumeType::Static)
		{
			SpringArm->TargetArmLength = 0.f;
			SpringArm->SocketOffset = FVector::ZeroVector;
		}
		else
		{
			SpringArm->TargetArmLength = TargetPreset.TargetArmLength;
			SpringArm->SocketOffset = TargetPreset.CameraOffset;
		}

		SpringArm->SetWorldRotation(TargetRot);
		MainCamera->SetFieldOfView(TargetPreset.FieldOfView);

		// 래그 비활성화하고 즉시 업데이트
		SpringArm->bEnableCameraLag = false;
		SpringArm->bEnableCameraRotationLag = false;
		SpringArm->UpdateChildTransforms();

		if (PC && PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetGameCameraCutThisFrame();
		}

		ApplyProjectionSettings(TargetPreset, DeltaTime, true);
	}
	else
	{
		// 보간 속도 계산
		float CamInterpSpeed = TargetPreset.BlendTime > 0.0f ? 5.0f / TargetPreset.BlendTime : 9999.0f;
		float LocInterpSpeed = TargetPreset.bFollowPawn ? TargetPreset.TrackingInterpSpeed : CamInterpSpeed;

		SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, LocInterpSpeed));

		if (TargetPreset.VolumeType == ECameraVolumeType::Static)
		{
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 0.f, DeltaTime, CamInterpSpeed);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, FVector::ZeroVector, DeltaTime, CamInterpSpeed);
		}
		else
		{
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetPreset.TargetArmLength, DeltaTime, CamInterpSpeed);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, TargetPreset.CameraOffset, DeltaTime, CamInterpSpeed);
		}

		SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, DeltaTime, CamInterpSpeed));
		MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetPreset.FieldOfView, DeltaTime, CamInterpSpeed));

		ApplyProjectionSettings(TargetPreset, DeltaTime, false);
	}

	// ─── [7. 포스트 프로세스] ────────────────

	UpdatePostProcessSettings(TargetPreset.bEnableTiltShift,
		TargetPreset.ManualFocusDistance, TargetPreset.ApertureFStop,
		TargetPreset.SensorWidth, TargetPreset.NearBlurRadius,
		TargetPreset.FarBlurRadius, TargetPreset.FarTransitionRegion);

	// ─── [8. 디버그] ────────────────────────

#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugInfo(TargetLoc, ActiveVolume, TargetPreset);
	}
#endif
}

// ──────────────────────────────────────────────────
// ★ 바운드 클램핑 (핵심 기능)
// ──────────────────────────────────────────────────

FVector AProjectHCameraActor::ClampToBounds(const FVector& TargetLocation,
	const AProjectHCameraVolume* Volume, const FCameraPresetSettings& Preset) const
{
	if (!Volume) return TargetLocation;

	// ★ 볼륨의 로컬 공간에서 클램프 → 회전된 볼륨도 올바르게 처리
	const FTransform VolumeTransform = Volume->GetActorTransform();
	FVector LocalLoc = VolumeTransform.InverseTransformPosition(TargetLocation);

	// 볼륨 오프셋 적용 (로컬 공간에서)
	const FVector VolumeOffset = Volume->GetActorTransform().InverseTransformPosition(Volume->GetVolumeCenter())
		- FVector::ZeroVector; // GetVolumeCenter의 로컬 기준점

	const FVector Extent = Volume->GetVolumeExtent();
	const float Padding = Preset.BoundsPadding;
	const FVector PaddedExtent = FVector(
		FMath::Max(Extent.X - Padding, 0.f),
		FMath::Max(Extent.Y - Padding, 0.f),
		FMath::Max(Extent.Z - Padding, 0.f)
	);

	const int32 Flags = Preset.BoundsBlockFlags;

	// X축
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinX))
		LocalLoc.X = FMath::Max(LocalLoc.X, -PaddedExtent.X);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxX))
		LocalLoc.X = FMath::Min(LocalLoc.X, PaddedExtent.X);

	// Y축
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinY))
		LocalLoc.Y = FMath::Max(LocalLoc.Y, -PaddedExtent.Y);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxY))
		LocalLoc.Y = FMath::Min(LocalLoc.Y, PaddedExtent.Y);

	// Z축
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinZ))
		LocalLoc.Z = FMath::Max(LocalLoc.Z, -PaddedExtent.Z);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxZ))
		LocalLoc.Z = FMath::Min(LocalLoc.Z, PaddedExtent.Z);

	// 월드 공간으로 다시 변환
	return VolumeTransform.TransformPosition(LocalLoc);
}

// ──────────────────────────────────────────────────
// 래그 설정 적용
// ──────────────────────────────────────────────────

void AProjectHCameraActor::ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut)
{
	if (bHardCut)
	{
		// 하드컷 시 래그 잠시 비활성화 (UpdateChildTransforms 후 재활성화는 다음 프레임)
		return;
	}

	SpringArm->bEnableCameraLag = Preset.bEnableLocationLag && bUseCameraLag;
	SpringArm->CameraLagSpeed = Preset.LocationLagSpeed;

	SpringArm->bEnableCameraRotationLag = Preset.bEnableRotationLag;
	SpringArm->CameraRotationLagSpeed = Preset.RotationLagSpeed;
}

// ──────────────────────────────────────────────────
// 투영 모드 적용
// ──────────────────────────────────────────────────

void AProjectHCameraActor::ApplyProjectionSettings(const FCameraPresetSettings& Preset,
	float DeltaTime, bool bHardCut)
{
	if (Preset.ProjectionType == ECameraProjectionType::Orthographic)
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);

		if (bHardCut)
		{
			CurrentOrthoWidth = Preset.OrthoWidth;
		}
		else
		{
			CurrentOrthoWidth = FMath::FInterpTo(CurrentOrthoWidth, Preset.OrthoWidth, DeltaTime, 5.0f);
		}
		MainCamera->SetOrthoWidth(CurrentOrthoWidth);
	}
	else
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
	}
}

// ──────────────────────────────────────────────────
// 충돌 설정 적용
// ──────────────────────────────────────────────────

void AProjectHCameraActor::ApplyCollisionSettings(const FCameraPresetSettings& Preset)
{
	SpringArm->bDoCollisionTest = Preset.bEnableCameraCollision;

	if (Preset.bEnableCameraCollision)
	{
		SpringArm->ProbeSize = Preset.CollisionProbeRadius;
		SpringArm->ProbeChannel = ECC_Camera;
	}
}

// ──────────────────────────────────────────────────
// 디버그 시각화
// ──────────────────────────────────────────────────

#if WITH_EDITOR
void AProjectHCameraActor::DrawDebugInfo(const FVector& TargetLoc,
	const AProjectHCameraVolume* Volume, const FCameraPresetSettings& Preset) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 카메라 위치 → 타겟 라인
	DrawDebugLine(World, MainCamera->GetComponentLocation(), TargetLoc,
		FColor::Yellow, false, -1.f, 0, 1.5f);
	DrawDebugSphere(World, TargetLoc, 15.f, 8, FColor::Yellow, false, -1.f);

	// 바운드 박스 (활성 시)
	if (Volume && Preset.bEnableBoundsBlocking)
	{
		const FVector Center = Volume->GetVolumeCenter();
		const FVector Extent = Volume->GetVolumeExtent();
		const float Padding = Preset.BoundsPadding;
		const FVector PaddedExtent = Extent - FVector(Padding);

		// 원본 볼륨: 시안
		DrawDebugBox(World, Center, Extent, Volume->GetActorQuat(), FColor::Cyan, false, -1.f, 0, 1.f);
		// 유효 카메라 영역: 노란색
		DrawDebugBox(World, Center, PaddedExtent, Volume->GetActorQuat(), FColor::Yellow, false, -1.f, 0, 2.f);
	}

	// 카메라 위치
	DrawDebugSphere(World, MainCamera->GetComponentLocation(), 20.f, 12, FColor::Green, false, -1.f);

	// 전환 상태
	if (bIsTransitioning)
	{
		DrawDebugString(World, GetActorLocation() + FVector(0, 0, 80),
			TEXT("TRANSITIONING"), nullptr, FColor::White, 0.f, true);
	}

	// 현재 모드
	FString ModeStr = (Preset.VolumeType == ECameraVolumeType::Static) ? TEXT("STATIC") : TEXT("DYNAMIC");
	FString ProjStr = (Preset.ProjectionType == ECameraProjectionType::Orthographic) ? TEXT("ORTHO") : TEXT("PERSP");
	DrawDebugString(World, GetActorLocation() + FVector(0, 0, 50),
		FString::Printf(TEXT("%s | %s"), *ModeStr, *ProjStr),
		nullptr, FColor::Cyan, 0.f, true);
}
#endif