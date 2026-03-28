#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h"
#include "Data/CameraPresetDataAsset.h"
#include "GameFramework/PlayerController.h"

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
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraLagMaxDistance = 1000.0f;
}

void AProjectHCameraActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentOrthoWidth = MainCamera->OrthoWidth;
}

FRotator AProjectHCameraActor::GetCameraViewRotation() const
{
	return SpringArm ? SpringArm->GetComponentRotation() : GetActorRotation();
}

void AProjectHCameraActor::UpdateCameraSettings(float InArmLength, float FOV, FRotator Rot)
{
	if (SpringArm && MainCamera)
	{
		SpringArm->TargetArmLength = InArmLength;
		SpringArm->SetRelativeRotation(Rot);
		MainCamera->SetFieldOfView(FOV);
	}
}

void AProjectHCameraActor::UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop,
	float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition)
{
	if (!MainCamera) return;
	auto& PP = MainCamera->PostProcessSettings;
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
	FVector Loc = MainCamera->GetComponentLocation();
	FVector Fwd = MainCamera->GetForwardVector();
	FVector Hit;
	return FMath::SegmentPlaneIntersection(Loc, Loc + Fwd * 10000.f, FPlane(FVector::UpVector, 0.f), Hit)
		? Hit : Loc + Fwd * 2000.f;
}

// ──────────────────────────────────────────────────
// Tick — 각 책임을 헬퍼 함수로 분리
// ──────────────────────────────────────────────────

void AProjectHCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
	if (!Subsystem) return;

	FCameraPresetSettings Preset;
	if (!Subsystem->GetActivePreset(Preset)) return;

	AActor* ActiveInstigator = Subsystem->GetActiveInstigator();
	AProjectHCameraVolume* ActiveVolume = Cast<AProjectHCameraVolume>(ActiveInstigator);

	bool bVolumeChanged = (LastVolume != ActiveInstigator);
	LastVolume = ActiveInstigator;

	float EffectiveBlendTime = Preset.BlendTime;
	if (bVolumeChanged)
	{
		float ExitOverride;
		if (Subsystem->ConsumeExitBlendOverride(ExitOverride))
			EffectiveBlendTime = ExitOverride;
	}

	bool bHardCut = bIsFirstTick || (bVolumeChanged && EffectiveBlendTime <= 0.0f);
	bIsFirstTick = false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// ★ 각 단계를 명확한 헬퍼로 분리
	const FVector TargetLoc = ComputeTargetLocation(Preset, ActiveInstigator, ActiveVolume, PlayerPawn);
	const FRotator TargetRot = ComputeTargetRotation(Preset, ActiveInstigator, ActiveVolume);
	const float TargetFOV = Preset.GetEffectiveFOV();

	ApplyLagSettings(Preset, bHardCut);

	if (bHardCut)
	{
		ApplyHardCut(Preset, TargetLoc, TargetRot, TargetFOV, PC);
	}
	else
	{
		const float CamSpeed = EffectiveBlendTime > 0.0f ? 5.0f / EffectiveBlendTime : 9999.0f;
		ApplySmooth(Preset, TargetLoc, TargetRot, TargetFOV, CamSpeed, DeltaTime);
	}

	UpdatePostProcessSettings(Preset.bEnableTiltShift, Preset.ManualFocusDistance, Preset.ApertureFStop,
		Preset.SensorWidth, Preset.NearBlurRadius, Preset.FarBlurRadius, Preset.FarTransitionRegion);
}

// ──────────────────────────────────────────────────
// ★ 추출된 헬퍼 함수들
// ──────────────────────────────────────────────────

FVector AProjectHCameraActor::ComputeTargetLocation(const FCameraPresetSettings& Preset,
	AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume, APawn* PlayerPawn) const
{
	FVector TargetLoc;
	if (Preset.VolumeType == ECameraVolumeType::Static && ActiveVolume)
	{
		TargetLoc = ActiveVolume->GetVolumeCenter()
			+ ActiveVolume->GetActorRotation().RotateVector(Preset.StaticCameraOffset);
	}
	else
	{
		TargetLoc = ActiveInstigator ? ActiveInstigator->GetActorLocation() : GetActorLocation();
		if (Preset.bFollowPawn && PlayerPawn)
			TargetLoc = PlayerPawn->GetActorLocation() + Preset.FocusPointOffset;
	}

	TargetLoc += EdgeScrollOffset;
	return TargetLoc;
}

FRotator AProjectHCameraActor::ComputeTargetRotation(const FCameraPresetSettings& Preset,
	AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume) const
{
	if (Preset.VolumeType == ECameraVolumeType::Static)
	{
		FRotator Rot = Preset.StaticCameraRotation;
		if (ActiveVolume)
			Rot = ActiveVolume->GetActorTransform().TransformRotation(Preset.StaticCameraRotation.Quaternion()).Rotator();
		return Rot;
	}
	else
	{
		FRotator Rot = Preset.Rotation;
		if (ActiveInstigator)
			Rot = ActiveInstigator->GetActorTransform().TransformRotation(Preset.Rotation.Quaternion()).Rotator();
		return Rot;
	}
}

void AProjectHCameraActor::ApplyHardCut(const FCameraPresetSettings& Preset,
	const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, APlayerController* PC)
{
	SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
	if (Preset.VolumeType == ECameraVolumeType::Static)
	{
		SpringArm->TargetArmLength = 0.f;
		SpringArm->SocketOffset = FVector::ZeroVector;
	}
	else
	{
		SpringArm->TargetArmLength = Preset.TargetArmLength;
		SpringArm->SocketOffset = Preset.CameraOffset;
	}
	SpringArm->SetWorldRotation(TargetRot);
	MainCamera->SetFieldOfView(TargetFOV);
	SpringArm->bEnableCameraLag = false;
	SpringArm->UpdateChildTransforms();
	if (PC && PC->PlayerCameraManager)
		PC->PlayerCameraManager->SetGameCameraCutThisFrame();
	ApplyProjectionSettings(Preset, 0.f, true);

	// ★ 하드컷 시 엣지스크롤 오프셋 리셋 (한 번에 처리)
	EdgeScrollOffset = FVector::ZeroVector;
}

void AProjectHCameraActor::ApplySmooth(const FCameraPresetSettings& Preset,
	const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, float CamSpeed, float DT)
{
	float LocSpeed = Preset.bFollowPawn ? Preset.TrackingInterpSpeed : CamSpeed;
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DT, LocSpeed));

	if (Preset.VolumeType == ECameraVolumeType::Static)
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 0.f, DT, CamSpeed);
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, FVector::ZeroVector, DT, CamSpeed);
	}
	else
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Preset.TargetArmLength, DT, CamSpeed);
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Preset.CameraOffset, DT, CamSpeed);
	}
	SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, DT, CamSpeed));
	MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DT, CamSpeed));
	ApplyProjectionSettings(Preset, DT, false);

	// ★ 매 프레임 리셋
	EdgeScrollOffset = FVector::ZeroVector;
}

void AProjectHCameraActor::ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut)
{
	if (bHardCut) return;
	SpringArm->bEnableCameraLag = Preset.bEnableLocationLag && bUseCameraLag;
	SpringArm->CameraLagSpeed = Preset.LocationLagSpeed;
}

void AProjectHCameraActor::ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DT, bool bHard)
{
	if (Preset.ProjectionType == ECameraProjectionType::Orthographic)
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		float Target = Preset.GetEffectiveOrthoWidth();
		CurrentOrthoWidth = bHard ? Target : FMath::FInterpTo(CurrentOrthoWidth, Target, DT, 5.0f);
		MainCamera->SetOrthoWidth(CurrentOrthoWidth);
	}
	else
		MainCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
}