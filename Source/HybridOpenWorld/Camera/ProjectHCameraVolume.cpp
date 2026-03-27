#include "ProjectHCameraVolume.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ProjectHCameraSubsystem.h"
#include "GameFramework/Pawn.h"

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

void AProjectHCameraVolume::BeginPlay() { Super::BeginPlay(); }

FVector AProjectHCameraVolume::GetVolumeCenter() const { return GetActorLocation(); }
FVector AProjectHCameraVolume::GetVolumeExtent() const { return CollisionBox->GetScaledBoxExtent(); }

void AProjectHCameraVolume::UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings)
{
	LocalSettings = NewSettings;
	if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		Sub->NotifyVolumeSettingsChanged(this);
}

void AProjectHCameraVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* P = Cast<APawn>(OtherActor))
		if (P->IsPlayerControlled())
			if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
				Sub->PushCameraPreset(LocalSettings, Priority, this);
}

void AProjectHCameraVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* P = Cast<APawn>(OtherActor))
		if (P->IsPlayerControlled())
			if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
				Sub->PopCameraPreset(this);
}

void AProjectHCameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (CollisionBox) CollisionBox->SetBoxExtent(VolumeExtent);

	if (PreviewSpringArm && PreviewCamera)
	{
		if (LocalSettings.VolumeType == ECameraVolumeType::Static)
		{
			PreviewSpringArm->SetRelativeLocation(LocalSettings.StaticCameraOffset);
			PreviewSpringArm->SetRelativeRotation(LocalSettings.StaticCameraRotation);
			PreviewSpringArm->TargetArmLength = 0.f;
			PreviewSpringArm->SocketOffset = FVector::ZeroVector;
		}
		else
		{
			PreviewSpringArm->SetRelativeLocation(FVector::ZeroVector);
			PreviewSpringArm->TargetArmLength = LocalSettings.TargetArmLength;
			PreviewSpringArm->SetRelativeRotation(LocalSettings.Rotation);
			PreviewSpringArm->SocketOffset = LocalSettings.CameraOffset;
		}

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
	auto& PP = PreviewCamera->PostProcessSettings;
	const bool bOn = LocalSettings.bEnableTiltShift;
	PP.bOverride_DepthOfFieldFstop = bOn;
	PP.bOverride_DepthOfFieldSensorWidth = bOn;
	PP.bOverride_DepthOfFieldFocalDistance = bOn;
	PP.bOverride_DepthOfFieldNearBlurSize = bOn;
	PP.bOverride_DepthOfFieldFarBlurSize = bOn;
	PP.bOverride_DepthOfFieldFarTransitionRegion = bOn;
	if (bOn)
	{
		PP.DepthOfFieldFstop = LocalSettings.ApertureFStop;
		PP.DepthOfFieldSensorWidth = LocalSettings.SensorWidth;
		PP.DepthOfFieldFocalDistance = LocalSettings.ManualFocusDistance;
		PP.DepthOfFieldNearBlurSize = LocalSettings.NearBlurRadius;
		PP.DepthOfFieldFarBlurSize = LocalSettings.FarBlurRadius;
		PP.DepthOfFieldFarTransitionRegion = LocalSettings.FarTransitionRegion;
	}
}

void AProjectHCameraVolume::LoadFromDataAsset()
{
	if (LinkedDataAsset) { LocalSettings = LinkedDataAsset->Settings; OnConstruction(GetActorTransform()); }
}

void AProjectHCameraVolume::SaveToDataAsset()
{
	if (LinkedDataAsset) { LinkedDataAsset->Modify(); LinkedDataAsset->Settings = LocalSettings; }
}