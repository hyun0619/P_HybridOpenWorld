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
// Tick
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

	// ─── 타겟 위치 ──────────────────────────

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

	// ★ 엣지스크롤 오프셋 적용
	TargetLoc += EdgeScrollOffset;
	EdgeScrollOffset = FVector::ZeroVector; // 매 프레임 리셋

	// ─── 타겟 회전 ──────────────────────────

	FRotator TargetRot;
	if (Preset.VolumeType == ECameraVolumeType::Static)
	{
		TargetRot = Preset.StaticCameraRotation;
		if (ActiveVolume)
			TargetRot = ActiveVolume->GetActorTransform().TransformRotation(Preset.StaticCameraRotation.Quaternion()).Rotator();
	}
	else
	{
		TargetRot = Preset.Rotation;
		if (ActiveInstigator)
			TargetRot = ActiveInstigator->GetActorTransform().TransformRotation(Preset.Rotation.Quaternion()).Rotator();
	}
	if (Preset.bUsePawnControlRotation && PlayerPawn && PlayerPawn->GetController())
		TargetRot = PlayerPawn->GetControlRotation();

	// ─── 래그/투영 ──────────────────────────

	ApplyLagSettings(Preset, bHardCut);

	const float TargetFOV = Preset.GetEffectiveFOV();

	if (bHardCut)
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
		ApplyProjectionSettings(Preset, DeltaTime, true);
	}
	else
	{
		float CamSpeed = EffectiveBlendTime > 0.0f ? 5.0f / EffectiveBlendTime : 9999.0f;
		float LocSpeed = Preset.bFollowPawn ? Preset.TrackingInterpSpeed : CamSpeed;

		SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, LocSpeed));

		if (Preset.VolumeType == ECameraVolumeType::Static)
		{
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 0.f, DeltaTime, CamSpeed);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, FVector::ZeroVector, DeltaTime, CamSpeed);
		}
		else
		{
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Preset.TargetArmLength, DeltaTime, CamSpeed);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Preset.CameraOffset, DeltaTime, CamSpeed);
		}
		SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, DeltaTime, CamSpeed));
		MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, CamSpeed));
		ApplyProjectionSettings(Preset, DeltaTime, false);
	}

	UpdatePostProcessSettings(Preset.bEnableTiltShift, Preset.ManualFocusDistance, Preset.ApertureFStop,
		Preset.SensorWidth, Preset.NearBlurRadius, Preset.FarBlurRadius, Preset.FarTransitionRegion);
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