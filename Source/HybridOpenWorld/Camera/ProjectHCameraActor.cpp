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
// ★ 핵심 수정: 실제 카메라 뷰 방향 반환
// ──────────────────────────────────────────────────

FRotator AProjectHCameraActor::GetCameraViewRotation() const
{
	// SpringArm의 월드 회전 = 실제 화면에 보이는 카메라 방향
	// GetActorRotation()은 카메라 액터의 루트 위치일 뿐,
	// 볼륨 전환 중 SpringArm이 보간하면서 실제 뷰 방향과 달라질 수 있습니다.
	if (SpringArm)
	{
		return SpringArm->GetComponentRotation();
	}
	return GetActorRotation();
}

// ──────────────────────────────────────────────────
// 레거시 API
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
// Tick
// ──────────────────────────────────────────────────

void AProjectHCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
	if (!Subsystem) return;

	FCameraPresetSettings TargetPreset;
	if (!Subsystem->GetActivePreset(TargetPreset)) return;

	AActor* ActiveInstigator = Subsystem->GetActiveInstigator();
	AProjectHCameraVolume* ActiveVolume = Cast<AProjectHCameraVolume>(ActiveInstigator);

	// ─── 볼륨 변경 감지 ──────────────────────

	bool bVolumeChanged = (LastVolume != ActiveInstigator);
	LastVolume = ActiveInstigator;

	// ★ 퇴장 블렌드 오버라이드 소비
	// 볼륨이 바뀌었고, 서브시스템에 ExitBlendTime 오버라이드가 있으면 그걸 사용
	float EffectiveBlendTime = TargetPreset.BlendTime;
	if (bVolumeChanged)
	{
		float ExitOverride;
		if (Subsystem->ConsumeExitBlendOverride(ExitOverride))
		{
			EffectiveBlendTime = ExitOverride;
		}
	}

	bool bHardCut = bIsFirstTick || (bVolumeChanged && EffectiveBlendTime <= 0.0f);
	bIsFirstTick = false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// ─── [1. 타겟 위치 계산] ────────────────

	FVector TargetLoc;

	if (TargetPreset.VolumeType == ECameraVolumeType::Static && ActiveVolume)
	{
		TargetLoc = ActiveVolume->GetVolumeCenter()
			+ ActiveVolume->GetActorRotation().RotateVector(TargetPreset.StaticCameraOffset);
	}
	else
	{
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

	// ─── [3. 타겟 회전 계산] ────────────────

	FRotator TargetRot;

	if (TargetPreset.VolumeType == ECameraVolumeType::Static)
	{
		TargetRot = TargetPreset.StaticCameraRotation;
		if (ActiveVolume)
		{
			TargetRot = ActiveVolume->GetActorTransform()
				.TransformRotation(TargetPreset.StaticCameraRotation.Quaternion()).Rotator();
		}
	}
	else
	{
		TargetRot = TargetPreset.Rotation;
		if (ActiveInstigator)
		{
			TargetRot = ActiveInstigator->GetActorTransform()
				.TransformRotation(TargetPreset.Rotation.Quaternion()).Rotator();
		}
	}

	// Pawn Control Rotation 오버라이드
	if (TargetPreset.bUsePawnControlRotation && PlayerPawn && PlayerPawn->GetController())
	{
		TargetRot = PlayerPawn->GetControlRotation();
	}

	// ─── [4. 래그/충돌/투영 적용] ───────────

	ApplyLagSettings(TargetPreset, bHardCut);
	ApplyCollisionSettings(TargetPreset);

	// ─── [5. 실제 값 적용] ──────────────────

	// 모드에 따른 FOV/OrthoWidth 결정
	const float TargetFOV = TargetPreset.GetEffectiveFOV();
	const float TargetOrthoW = TargetPreset.GetEffectiveOrthoWidth();

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
		MainCamera->SetFieldOfView(TargetFOV);

		SpringArm->bEnableCameraLag = false;
		SpringArm->bEnableCameraRotationLag = false;
		SpringArm->UpdateChildTransforms();

		if (PC && PC->PlayerCameraManager)
			PC->PlayerCameraManager->SetGameCameraCutThisFrame();

		ApplyProjectionSettings(TargetPreset, DeltaTime, true);
	}
	else
	{
		// ★ 보간 속도: EffectiveBlendTime을 사용 (ExitBlendTime 반영됨)
		float CamInterpSpeed = EffectiveBlendTime > 0.0f ? 5.0f / EffectiveBlendTime : 9999.0f;
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
		MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, CamInterpSpeed));

		ApplyProjectionSettings(TargetPreset, DeltaTime, false);
	}

	// ─── [6. 포스트 프로세스] ────────────────

	UpdatePostProcessSettings(TargetPreset.bEnableTiltShift,
		TargetPreset.ManualFocusDistance, TargetPreset.ApertureFStop,
		TargetPreset.SensorWidth, TargetPreset.NearBlurRadius,
		TargetPreset.FarBlurRadius, TargetPreset.FarTransitionRegion);
}

// ──────────────────────────────────────────────────
// 바운드 클램핑 (볼륨 로컬 공간에서 클램프)
// ──────────────────────────────────────────────────

FVector AProjectHCameraActor::ClampToBounds(const FVector& TargetLocation,
	const AProjectHCameraVolume* Volume, const FCameraPresetSettings& Preset) const
{
	if (!Volume) return TargetLocation;

	const FTransform VolumeTransform = Volume->GetActorTransform();
	FVector LocalLoc = VolumeTransform.InverseTransformPosition(TargetLocation);

	const FVector Extent = Volume->GetVolumeExtent();
	const float Padding = Preset.BoundsPadding;
	const FVector PaddedExtent = FVector(
		FMath::Max(Extent.X - Padding, 0.f),
		FMath::Max(Extent.Y - Padding, 0.f),
		FMath::Max(Extent.Z - Padding, 0.f)
	);

	const int32 Flags = Preset.BoundsBlockFlags;

	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinX))
		LocalLoc.X = FMath::Max(LocalLoc.X, -PaddedExtent.X);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxX))
		LocalLoc.X = FMath::Min(LocalLoc.X, PaddedExtent.X);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinY))
		LocalLoc.Y = FMath::Max(LocalLoc.Y, -PaddedExtent.Y);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxY))
		LocalLoc.Y = FMath::Min(LocalLoc.Y, PaddedExtent.Y);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMinZ))
		LocalLoc.Z = FMath::Max(LocalLoc.Z, -PaddedExtent.Z);
	if (Flags & static_cast<int32>(ECameraBoundsBlockFlags::BlockMaxZ))
		LocalLoc.Z = FMath::Min(LocalLoc.Z, PaddedExtent.Z);

	return VolumeTransform.TransformPosition(LocalLoc);
}

// ──────────────────────────────────────────────────
// 래그 / 투영 / 충돌 적용
// ──────────────────────────────────────────────────

void AProjectHCameraActor::ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut)
{
	if (bHardCut) return; // 하드컷 시 래그 비활성화됨 (Tick에서 직접 처리)

	SpringArm->bEnableCameraLag = Preset.bEnableLocationLag && bUseCameraLag;
	SpringArm->CameraLagSpeed = Preset.LocationLagSpeed;
	SpringArm->bEnableCameraRotationLag = Preset.bEnableRotationLag;
	SpringArm->CameraRotationLagSpeed = Preset.RotationLagSpeed;
}

void AProjectHCameraActor::ApplyProjectionSettings(const FCameraPresetSettings& Preset,
	float DeltaTime, bool bHardCut)
{
	if (Preset.ProjectionType == ECameraProjectionType::Orthographic)
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		const float TargetOrthoW = Preset.GetEffectiveOrthoWidth();
		CurrentOrthoWidth = bHardCut ? TargetOrthoW
			: FMath::FInterpTo(CurrentOrthoWidth, TargetOrthoW, DeltaTime, 5.0f);
		MainCamera->SetOrthoWidth(CurrentOrthoWidth);
	}
	else
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
	}
}

void AProjectHCameraActor::ApplyCollisionSettings(const FCameraPresetSettings& Preset)
{
	SpringArm->bDoCollisionTest = Preset.bEnableCameraCollision;
	if (Preset.bEnableCameraCollision)
	{
		SpringArm->ProbeSize = Preset.CollisionProbeRadius;
		SpringArm->ProbeChannel = ECC_Camera;
	}
}