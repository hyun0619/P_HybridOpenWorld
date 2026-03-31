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
	
#if WITH_EDITORONLY_DATA // 에디터에서만 보이는 설정 - 볼륨 선 두께, 색상
	CollisionBox->SetLineThickness(2.0f);
	CollisionBox->ShapeColor = FColor::Cyan;
#endif

	// 프리뷰용 스프링암 설정
	PreviewSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("PreviewSpringArm"));
	PreviewSpringArm->SetupAttachment(RootComponent);
	PreviewSpringArm->SetUsingAbsoluteScale(true);
	PreviewSpringArm->bDoCollisionTest = false;
	PreviewSpringArm->bHiddenInGame = true;

	// 프리뷰용 카메라 설정
	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(PreviewSpringArm);
	PreviewCamera->bHiddenInGame = true;

	// 델리게이트 연결 - 충돌 이벤트 발생
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapEnd);
}

void AProjectHCameraVolume::BeginPlay()
{
	Super::BeginPlay();
}

/* 유틸리티 함수 - 볼륨 정보 반환 */
FVector AProjectHCameraVolume::GetVolumeCenter() const { return GetActorLocation(); }
FVector AProjectHCameraVolume::GetVolumeExtent() const { return CollisionBox->GetScaledBoxExtent(); }

/* 런타임 설정 업데이트 - 설정 바뀌면 서브시스템에 알려 즉시 반영 */
void AProjectHCameraVolume::UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings)
{
	LocalSettings = NewSettings;
	if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		Sub->NotifyVolumeSettingsChanged(this);
}

/* 플레이어가 볼륨 진입 시 */
void AProjectHCameraVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 제어 중인 플레이어 폰인지 확인
	if (APawn* P = Cast<APawn>(OtherActor))
	{
		if (P->IsPlayerControlled())
		{
			// 서브시스템의 스택에 현재 카메라 설정 푸쉬
			if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
			{
				Sub->PushCameraPreset(LocalSettings, Priority, this);
			}
		}
	}
}

/* 플레이어가 볼륨 나갔을 때 */
void AProjectHCameraVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* P = Cast<APawn>(OtherActor))
	{
		if (P->IsPlayerControlled())
		{
			// 서브시스템 스택에서 현재 볼륨 설정 제거
			if (auto* Sub = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
			{
				Sub->PopCameraPreset(this);
			}
		}
	}
}

/* 에디터 프로퍼티 변경 시 프리뷰 로직 */
void AProjectHCameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (CollisionBox) CollisionBox->SetBoxExtent(VolumeExtent);

	// 카메라 설정에 따른 프리뷰 컴포넌트 배치
	if (PreviewSpringArm && PreviewCamera)
	{
		if (LocalSettings.VolumeType == ECameraVolumeType::Static) // 고정 카메라 모드일 때
		{
			PreviewSpringArm->SetRelativeLocation(LocalSettings.StaticCameraOffset);
			PreviewSpringArm->SetRelativeRotation(LocalSettings.StaticCameraRotation);
			PreviewSpringArm->TargetArmLength = 0.f;
			PreviewSpringArm->SocketOffset = FVector::ZeroVector;
		}
		else // 추적 카메라 모드일 때 - 플레이어 따라다님
		{
			PreviewSpringArm->SetRelativeLocation(FVector::ZeroVector);
			PreviewSpringArm->TargetArmLength = LocalSettings.TargetArmLength;
			PreviewSpringArm->SetRelativeRotation(LocalSettings.Rotation);
			PreviewSpringArm->SocketOffset = LocalSettings.CameraOffset;
		}

		// 투영 모드 설정 (원근, 직교)
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
		ApplyPreviewPostProcessing(); // 포스트 프로세스 적용
	}
}


FCameraPresetSettings AProjectHCameraVolume::GetCameraSettings() const
{
	return LocalSettings;
}

/* 포스트 프로세싱 효과 적용 - 틸트 쉬프트 */
void AProjectHCameraVolume::ApplyPreviewPostProcessing()
{
	if (!PreviewCamera) return;
	
	auto& PP = PreviewCamera->PostProcessSettings;
	const bool bOn = LocalSettings.bEnableTiltShift;
	
	// 각 속성의 오버라이드 체크박스를 활성화, 비활성화
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


/* 에셋 관리 - 값 불러오기 */
void AProjectHCameraVolume::LoadFromDataAsset()
{
	if (LinkedDataAsset)
	{
		LocalSettings = LinkedDataAsset->Settings;
		OnConstruction(GetActorTransform()); // 변경사항 즉시 시각화
	}
}

/* 에셋 관리 - 값 저장하기 */
void AProjectHCameraVolume::SaveToDataAsset()
{
	if (LinkedDataAsset)
	{
		LinkedDataAsset->Modify(); // 에샛 변경을 알림
		LinkedDataAsset->Settings = LocalSettings; 
	}
}