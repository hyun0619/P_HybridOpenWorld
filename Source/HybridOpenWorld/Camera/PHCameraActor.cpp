#include "PHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "PHCameraSubsystem.h"
#include "PHCameraVolume.h"
#include "Data/CameraPresetDataAsset.h"
#include "GameFramework/PlayerController.h"

APHCameraActor::APHCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 물리 계산이 끝난 후 카메라를 업데이트해야 떨림 현상 X
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false; // 카메라 벽에 막혀 튀는 현상 방지
	
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm);
	
	// 월드 파티션 환경에서 카메라 위치를 기준으로 스트리밍 활성화
	StreamingSourceComponent = CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>(TEXT("StreamingSourceComponent"));

	SpringArm->bEnableCameraLag = bUseCameraLag;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraLagMaxDistance = 1000.0f;
}

void APHCameraActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentOrthoWidth = MainCamera->OrthoWidth;
	
	// 서브시스템 델리게이트 구독
	if (UPHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UPHCameraSubsystem>())
	{
		Subsystem->OnActiveVolumeChanged.AddDynamic(this, &APHCameraActor::OnVolumeChanged);
	}
}

void APHCameraActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 안전한 메모리 관리를 위한 구독 해제
	if (UPHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UPHCameraSubsystem>())
	{
		Subsystem->OnActiveVolumeChanged.RemoveDynamic(this, &APHCameraActor::OnVolumeChanged);
	}
	
	Super::EndPlay(EndPlayReason);
}

/* 현재 카메라가 최종적으로 보고 있는 회전값 반환 */
FRotator APHCameraActor::GetCameraViewRotation() const
{
	return SpringArm ? SpringArm->GetComponentRotation() : GetActorRotation();
}

UCameraComponent* APHCameraActor::GetMainCamera() const
{
	return MainCamera;
}

void APHCameraActor::SetEdgeScrollOffset(const FVector& Offset)
{
	EdgeScrollOffset = Offset;
}

/* 수동 카메라 설정 업데이트 */
void APHCameraActor::UpdateCameraSettings(float InArmLength, float FOV, FRotator Rot)
{
	if (SpringArm && MainCamera)
	{
		SpringArm->TargetArmLength = InArmLength;
		SpringArm->SetRelativeRotation(Rot);
		MainCamera->SetFieldOfView(FOV);
	}
}

/* 포스트 프로세스 오버라이드 설정 로직 */
void APHCameraActor::UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop,
	float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition)
{
	if (!MainCamera) return;
	auto& PP = MainCamera->PostProcessSettings;
	// 켜고 꺼야 할 핀 일괄 제어
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

/* 카메라 지면의 어디를 조준하는지 교차점 계산 */
FVector APHCameraActor::GetCameraTargetLocation() const
{
	FVector Loc = MainCamera->GetComponentLocation();
	FVector Fwd = MainCamera->GetForwardVector();
	FVector Hit;
	// 레이캐스트 대신 평면 방정식을 이용한 수학적 교차점 계산
	return FMath::SegmentPlaneIntersection(Loc, Loc + Fwd * 10000.f,
		FPlane(FVector::UpVector, 0.f), Hit) ? Hit : Loc + Fwd * 2000.f;
}


void APHCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto* Subsystem = GetWorld()->GetSubsystem<UPHCameraSubsystem>();
	if (!Subsystem) return;

	FCameraPresetSettings Preset;
	if (!Subsystem->GetActivePreset(Preset)) return;

	AActor* ActiveInstigator = Subsystem->GetActiveInstigator();
	APHCameraVolume* ActiveVolume = Cast<APHCameraVolume>(ActiveInstigator);

	float EffectiveBlendTime = Preset.BlendTime;
	// 델리게이트를 통해 볼륨 변경이 감지된 프레임
	if (bVolumeChangedThisFrame)
	{
		float ExitOverride;
		if (Subsystem->ConsumeExitBlendOverride(ExitOverride))
		{
			EffectiveBlendTime = ExitOverride;
		}
		bVolumeChangedThisFrame = false;
	}

	// 순간이동 여부 판단 - 블렌드 타임 0일 때
	bool bHardCut = bIsFirstTick || (EffectiveBlendTime <= 0.0f);
	bIsFirstTick = false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// 목표값 계산
	const FVector TargetLoc = ComputeTargetLocation(Preset, ActiveInstigator, ActiveVolume, PlayerPawn);
	const FRotator TargetRot = ComputeTargetRotation(Preset, ActiveInstigator, ActiveVolume);
	const float TargetFOV = Preset.GetEffectiveFOV();

	ApplyLagSettings(Preset, bHardCut); // 카메라 래그 설정 업데이트

	if (bHardCut) // 결과 반영 - 순간이동, 부드러움 보간
	{
		ApplyHardCut(Preset, TargetLoc, TargetRot, TargetFOV, PC);
	}
	else
	{
		// 블렌드 타임이 길수록 이동 속도 느려짐
		const float CamSpeed = EffectiveBlendTime > 0.0f ? BASE_INTERP_SPEED_MULTIPLIER / EffectiveBlendTime : 9999.0f;
		ApplySmooth(Preset, TargetLoc, TargetRot, TargetFOV, CamSpeed, DeltaTime);
	}

	// 포스트 프로세싱 최종 적용
	UpdatePostProcessSettings(Preset.bEnableTiltShift, Preset.ManualFocusDistance, Preset.ApertureFStop,
		Preset.SensorWidth, Preset.NearBlurRadius, Preset.FarBlurRadius, Preset.FarTransitionRegion);
	
	// 매 프레임 모든 처리가 끝난 후 오프셋 초기화
	EdgeScrollOffset = FVector::ZeroVector;
}

void APHCameraActor::OnVolumeChanged(APHCameraVolume* NewVolume, APHCameraVolume* PreviousVolume)
{
	// 볼륨이 변경되었다는 플래그 On
	bVolumeChangedThisFrame = true;
}

/* 카메라 액터 자체가 이동해야 할 위치 계산 */
FVector APHCameraActor::ComputeTargetLocation(const FCameraPresetSettings& Preset,
	AActor* ActiveInstigator, APHCameraVolume* ActiveVolume, APawn* PlayerPawn) const
{
	FVector TargetLoc;
	if (Preset.VolumeType == ECameraVolumeType::Static && ActiveVolume) // 고정 카메라 모드
	{
		TargetLoc = ActiveVolume->GetVolumeCenter()
			+ ActiveVolume->GetActorRotation().RotateVector(Preset.StaticCameraOffset);
	}
	else // 추적 카메라 모드
	{
		TargetLoc = ActiveInstigator ? ActiveInstigator->GetActorLocation() : GetActorLocation();
		if (Preset.bFollowPawn && PlayerPawn)
			TargetLoc = PlayerPawn->GetActorLocation() + Preset.FocusPointOffset;
	}

	TargetLoc += EdgeScrollOffset; // 카메라 룩어라운드 등 추가 오프셋 합산
	return TargetLoc;
}

/* 카메라가 회전해야 할 각도 계산 */
FRotator APHCameraActor::ComputeTargetRotation(const FCameraPresetSettings& Preset,
	AActor* ActiveInstigator, APHCameraVolume* ActiveVolume) const
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

/* 즉시 카메라 위치 및 상태 설정 */
void APHCameraActor::ApplyHardCut(const FCameraPresetSettings& Preset,
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
	
	// 엔진에 카메라 컷 발생을 알려 모션 블러 등이 튀지 않게 함
	if (PC && PC->PlayerCameraManager)
		PC->PlayerCameraManager->SetGameCameraCutThisFrame();
	ApplyProjectionSettings(Preset, 0.f, true);
}

/* 목표값으로 부드럽게 이동 */
void APHCameraActor::ApplySmooth(const FCameraPresetSettings& Preset,
	const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, float CamSpeed, float DT)
{
	// 폰 추적 시 전용 속도로 부드러운 느낌 조절
	float LocSpeed = Preset.bFollowPawn ? Preset.TrackingInterpSpeed : CamSpeed;
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DT, LocSpeed));

	if (Preset.VolumeType == ECameraVolumeType::Static) // 스프링암, FOV 보간
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
}

/* 래그 설정 동기화 */
void APHCameraActor::ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut)
{
	if (bHardCut) return;
	SpringArm->bEnableCameraLag = Preset.bEnableLocationLag && bUseCameraLag;
	SpringArm->CameraLagSpeed = Preset.LocationLagSpeed;
}

/* 투영 방식 설정 및 직교 너비 보간 */
void APHCameraActor::ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DT, bool bHard)
{
	if (Preset.ProjectionType == ECameraProjectionType::Orthographic)
	{
		MainCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		float Target = Preset.GetEffectiveOrthoWidth();
		
		// 직교 너비도 부드럽게 보간
		CurrentOrthoWidth = bHard ? Target : FMath::FInterpTo(CurrentOrthoWidth, Target, DT, 5.0f);
		MainCamera->SetOrthoWidth(CurrentOrthoWidth);
	}
	else
		MainCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
}