#include "ProjectHCameraVolume.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ProjectHCameraSubsystem.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

AProjectHCameraVolume::AProjectHCameraVolume()
{
	PrimaryActorTick.bCanEverTick = false; // ★ 퍼포먼스: 이벤트 기반, Tick 불필요

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(500.f, 500.f, 200.f));
	CollisionBox->SetCollisionProfileName(TEXT("Trigger"));
	CollisionBox->SetCanEverAffectNavigation(false);

#if WITH_EDITORONLY_DATA
	CollisionBox->SetLineThickness(2.0f);
	CollisionBox->ShapeColor = FColor::Cyan;
#endif

	PreviewSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("PreviewSpringArm"));
	PreviewSpringArm->SetupAttachment(RootComponent);
	PreviewSpringArm->SetUsingAbsoluteScale(true);
	PreviewSpringArm->bDoCollisionTest = false;
	PreviewSpringArm->bHiddenInGame = true;

	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(PreviewSpringArm);
	PreviewCamera->bHiddenInGame = true;

	// 오버랩 바인딩은 생성자에서 수행 (BeginPlay 이전에도 에디터 PIE에서 작동)
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapEnd);
}

void AProjectHCameraVolume::BeginPlay()
{
	Super::BeginPlay();
}

// ──────────────────────────────────────────────────
// Public API
// ──────────────────────────────────────────────────

FBox AProjectHCameraVolume::GetVolumeBounds() const
{
	const FVector Center = GetVolumeCenter();
	const FVector Extent = CollisionBox->GetScaledBoxExtent();
	return FBox(Center - Extent, Center + Extent);
}

FVector AProjectHCameraVolume::GetVolumeCenter() const
{
	// 액터 위치 + 볼륨 오프셋 (로컬→월드 변환)
	return GetActorLocation() + GetActorRotation().RotateVector(VolumeOffset);
}

FVector AProjectHCameraVolume::GetVolumeExtent() const
{
	return CollisionBox->GetScaledBoxExtent();
}

void AProjectHCameraVolume::UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings)
{
	LocalSettings = NewSettings;

	// 현재 이 볼륨이 활성 상태라면 서브시스템에 갱신 알림
	if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
	{
		Subsystem->NotifyVolumeSettingsChanged(this);
	}
}

// ──────────────────────────────────────────────────
// Overlap Handlers
// ──────────────────────────────────────────────────

void AProjectHCameraVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (PlayerPawn && PlayerPawn->IsPlayerControlled())
	{
		if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		{
			Subsystem->PushCameraPreset(LocalSettings, Priority, this);
		}
	}
}

void AProjectHCameraVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (PlayerPawn && PlayerPawn->IsPlayerControlled())
	{
		if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		{
			Subsystem->PopCameraPreset(this);
		}
	}
}

// ──────────────────────────────────────────────────
// Editor: 에디터 뷰포트 프리뷰
// ──────────────────────────────────────────────────

void AProjectHCameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// CollisionBox 크기와 오프셋 동기화
	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(VolumeExtent);
		CollisionBox->SetRelativeLocation(VolumeOffset);
	}

	// 카메라 프리뷰 갱신
	if (PreviewSpringArm && PreviewCamera)
	{
		if (LocalSettings.VolumeType == ECameraVolumeType::Static)
		{
			// Static 모드: 볼륨 기준 고정 위치에 프리뷰 배치
			PreviewSpringArm->SetRelativeLocation(VolumeOffset + LocalSettings.StaticCameraOffset);
			PreviewSpringArm->SetRelativeRotation(LocalSettings.StaticCameraRotation);
			PreviewSpringArm->TargetArmLength = 0.f; // 위치가 곧 카메라 위치
		}
		else
		{
			// Dynamic 모드: 기존 프리뷰 방식
			PreviewSpringArm->SetRelativeLocation(VolumeOffset);
			PreviewSpringArm->TargetArmLength = LocalSettings.TargetArmLength;
			PreviewSpringArm->SetRelativeRotation(LocalSettings.Rotation);
			PreviewSpringArm->SocketOffset = LocalSettings.CameraOffset;
		}

		// FOV/OrthoWidth
		if (LocalSettings.ProjectionType == ECameraProjectionType::Orthographic)
		{
			PreviewCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
			PreviewCamera->SetOrthoWidth(LocalSettings.OrthoWidth);
		}
		else
		{
			PreviewCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
			PreviewCamera->SetFieldOfView(LocalSettings.FieldOfView);
		}

		ApplyPreviewPostProcessing();
	}

#if WITH_EDITOR
	DrawBoundsPreview();
#endif
}

void AProjectHCameraVolume::ApplyPreviewPostProcessing()
{
	if (!PreviewCamera) return;

	FPostProcessSettings& PP = PreviewCamera->PostProcessSettings;
	const bool bEnable = LocalSettings.bEnableTiltShift;

	PP.bOverride_DepthOfFieldFstop = bEnable;
	PP.bOverride_DepthOfFieldSensorWidth = bEnable;
	PP.bOverride_DepthOfFieldFocalDistance = bEnable;
	PP.bOverride_DepthOfFieldNearBlurSize = bEnable;
	PP.bOverride_DepthOfFieldFarBlurSize = bEnable;
	PP.bOverride_DepthOfFieldFarTransitionRegion = bEnable;

	if (bEnable)
	{
		PP.DepthOfFieldFstop = LocalSettings.ApertureFStop;
		PP.DepthOfFieldSensorWidth = LocalSettings.SensorWidth;
		PP.DepthOfFieldFocalDistance = LocalSettings.ManualFocusDistance;
		PP.DepthOfFieldNearBlurSize = LocalSettings.NearBlurRadius;
		PP.DepthOfFieldFarBlurSize = LocalSettings.FarBlurRadius;
		PP.DepthOfFieldFarTransitionRegion = LocalSettings.FarTransitionRegion;
	}
}

// ──────────────────────────────────────────────────
// DA 동기화 (기존 워크플로우 보존)
// ──────────────────────────────────────────────────

void AProjectHCameraVolume::LoadFromDataAsset()
{
	if (LinkedDataAsset)
	{
		LocalSettings = LinkedDataAsset->Settings;
		OnConstruction(GetActorTransform());
		UE_LOG(LogTemp, Log, TEXT("[%s] DA 원본에서 데이터를 불러왔습니다."), *GetName());
	}
}

void AProjectHCameraVolume::SaveToDataAsset()
{
	if (LinkedDataAsset)
	{
		LinkedDataAsset->Modify();
		LinkedDataAsset->Settings = LocalSettings;
		UE_LOG(LogTemp, Warning, TEXT("[%s] 에 카메라 셋팅이 영구적으로 저장되었습니다!"), *LinkedDataAsset->GetName());
	}
}

// ──────────────────────────────────────────────────
// Editor: 바운드 시각화
// ──────────────────────────────────────────────────

#if WITH_EDITOR
void AProjectHCameraVolume::DrawBoundsPreview() const
{
	if (!GetWorld() || !LocalSettings.bEnableBoundsBlocking) return;

	const FVector Center = GetVolumeCenter();
	const FVector Extent = CollisionBox->GetScaledBoxExtent();
	const float Padding = LocalSettings.BoundsPadding;
	const FVector PaddedExtent = Extent - FVector(Padding);

	// 유효 카메라 영역을 노란색으로 표시
	if (PaddedExtent.X > 0.f && PaddedExtent.Y > 0.f)
	{
		DrawDebugBox(GetWorld(), Center, PaddedExtent, GetActorQuat(), FColor::Yellow, false, 0.f, 0, 2.f);
	}
}
#endif