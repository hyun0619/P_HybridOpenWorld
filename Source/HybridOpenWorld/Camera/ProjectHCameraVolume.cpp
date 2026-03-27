#include "ProjectHCameraVolume.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ProjectHCameraSubsystem.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

AProjectHCameraVolume::AProjectHCameraVolume()
{
	PrimaryActorTick.bCanEverTick = false;

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
	return GetActorLocation() + GetActorRotation().RotateVector(VolumeOffset);
}

FVector AProjectHCameraVolume::GetVolumeExtent() const
{
	return CollisionBox->GetScaledBoxExtent();
}

void AProjectHCameraVolume::UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings)
{
	LocalSettings = NewSettings;
	if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
	{
		Subsystem->NotifyVolumeSettingsChanged(this);
	}
}

// ──────────────────────────────────────────────────
// Overlap
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
// 에디터 프리뷰
// ──────────────────────────────────────────────────

void AProjectHCameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(VolumeExtent);
		CollisionBox->SetRelativeLocation(VolumeOffset);
	}

	if (PreviewSpringArm && PreviewCamera)
	{
		if (LocalSettings.VolumeType == ECameraVolumeType::Static)
		{
			PreviewSpringArm->SetRelativeLocation(VolumeOffset + LocalSettings.StaticCameraOffset);
			PreviewSpringArm->SetRelativeRotation(LocalSettings.StaticCameraRotation);
			PreviewSpringArm->TargetArmLength = 0.f;
			PreviewSpringArm->SocketOffset = FVector::ZeroVector;
		}
		else
		{
			PreviewSpringArm->SetRelativeLocation(VolumeOffset);
			PreviewSpringArm->TargetArmLength = LocalSettings.TargetArmLength;
			PreviewSpringArm->SetRelativeRotation(LocalSettings.Rotation);
			PreviewSpringArm->SocketOffset = LocalSettings.CameraOffset;
		}

		// 투영 모드별 프리뷰
		if (LocalSettings.ProjectionType == ECameraProjectionType::Orthographic)
		{
			PreviewCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
			PreviewCamera->SetOrthoWidth(LocalSettings.GetEffectiveOrthoWidth());
		}
		else
		{
			PreviewCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
			PreviewCamera->SetFieldOfView(LocalSettings.GetEffectiveFOV());
		}

		ApplyPreviewPostProcessing();
	}
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
// DA 동기화
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