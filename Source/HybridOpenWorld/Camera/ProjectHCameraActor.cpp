#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "ProjectHCameraSubsystem.h" // 추가
#include "Data/CameraPresetDataAsset.h"    // 추가
#include "GameFramework/PlayerController.h" // 추가



AProjectHCameraActor::AProjectHCameraActor()
{
	PrimaryActorTick.bCanEverTick = true; // 컨트롤러가 위치를 업데이트하므로 카메라에선 Tick 필요 x
	
	// ★ 핵심 1: 캐릭터가 먼저 움직인 '후'에 카메라가 따라가도록 순서를 늦춥니다. (미끄러짐 방지)
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false; // 월드맵 뷰에서 장애물에 카메라 튀는 현상 방지
	
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm);
	
	// 스트리밍 소스 컴포넌트 생성
	StreamingSourceComponent = CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>(TEXT("StreamingSourceComponent"));
	
	// 카메라 Lag 기능 활성화
	SpringArm->bEnableCameraLag = bUseCameraLag;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
	SpringArm->CameraLagMaxDistance = 1000.0f; // 카메라가 너무 멀리 뒤처지는 것을 방지
}

void AProjectHCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectHCameraActor::UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation)
{
	if (SpringArm && MainCamera)
	{
		SpringArm->TargetArmLength = TargetArmLength;
		SpringArm->SetRelativeRotation(Rotation);
		MainCamera->SetFieldOfView(FOV);
	}
}

void AProjectHCameraActor::UpdatePostProcessSettings(float InFocalDist, float InFStop, float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition)
{
	if (!MainCamera) return;

	FPostProcessSettings& PP_Settings = MainCamera->PostProcessSettings;

	// 조리개 값
	PP_Settings.bOverride_DepthOfFieldFstop = true;
	PP_Settings.DepthOfFieldFstop = InFStop; 

	// 센서 폭
	PP_Settings.bOverride_DepthOfFieldSensorWidth = true;
	PP_Settings.DepthOfFieldSensorWidth = InSensorWidth; 

	// 초점 거리
	PP_Settings.bOverride_DepthOfFieldFocalDistance = true;
	PP_Settings.DepthOfFieldFocalDistance = InFocalDist;

	// 전이 영역 미세 조정
	PP_Settings.bOverride_DepthOfFieldNearBlurSize = true;
	PP_Settings.DepthOfFieldNearBlurSize = InNearBlur;
	PP_Settings.bOverride_DepthOfFieldFarBlurSize = true;
	PP_Settings.DepthOfFieldFarBlurSize = InFarBlur;
	
	// 초점 구역에서 먼 블러 구역으로 전환되는 구간의 길이
	PP_Settings.bOverride_DepthOfFieldFarTransitionRegion = true;
	PP_Settings.DepthOfFieldFarTransitionRegion = InFarTransition;
}

FVector AProjectHCameraActor::GetCameraTargetLocation() const
{
	// 카메라가 바라보는 방향으로 월드맵 로드
	FVector CameraLoc = MainCamera->GetComponentLocation();
	FVector ForwardDir = MainCamera->GetForwardVector();
	
	// 언리얼 내장 함수를 사용하여 카메라가 지면(Z=0 평면)을 바라보는 교차점 계산
	// 월드맵 모드에서 카메라 시야 중심을 정확히 스트리밍하기 위함
	FVector IntersectionPoint;
	bool bIntersect = FMath::SegmentPlaneIntersection(
		CameraLoc, 
		CameraLoc + (ForwardDir * 10000.f), 
		FPlane(FVector::UpVector, 0.f), 
		IntersectionPoint
	);

	return bIntersect ? IntersectionPoint : (CameraLoc + ForwardDir * 2000.f);
}

void AProjectHCameraActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
    if (!Subsystem) return;

    UCameraPresetDataAsset* TargetPreset = Subsystem->GetActivePreset();
    if (!TargetPreset) return;

    // --- [1. 상태 변화 감지] ---
    bool bPresetChanged = (LastPreset != TargetPreset);
    LastPreset = TargetPreset;

    bool bHardCut = bIsFirstTick || (bPresetChanged && TargetPreset->BlendTime <= 0.0f);
    bIsFirstTick = false;

    // --- [2. 캐릭터 추적 로직] ---
    if (TargetPreset->bFollowPawn)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector TargetLoc = PC->GetPawn()->GetActorLocation();
            if (bHardCut)
            {
                SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
            }
            else
            {
                SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, 5.0f));
            }
        }
    }

    // --- [3. 카메라 수치 적용] ---
    if (bHardCut)
    {
       // ★ 핵심 해결책: 이번 프레임에서는 래그를 완전히 끄고, 절대 다시 켜지 않습니다!
       SpringArm->bEnableCameraLag = false;
       SpringArm->bEnableCameraRotationLag = false;

       // 수치 즉시 대입
       SpringArm->TargetArmLength = TargetPreset->TargetArmLength;
       SpringArm->SetRelativeRotation(TargetPreset->Rotation);
       MainCamera->SetFieldOfView(TargetPreset->FieldOfView);

       // 카메라 위치 강제 재계산
       SpringArm->UpdateChildTransforms(); 

       // 모션 블러 강제 차단
       if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
       {
          if (PC->PlayerCameraManager)
          {
             PC->PlayerCameraManager->SetGameCameraCutThisFrame();
          }
       }
    }
    else
    {
       // ★ 다음 프레임이 되어서야 래그를 원래 설정대로 복구합니다.
       // 이렇게 해야 스프링암이 미끄러지는 연산을 아예 할 수 없습니다.
       SpringArm->bEnableCameraLag = bUseCameraLag; 
       SpringArm->bEnableCameraRotationLag = true; 

       if (TargetPreset->BlendTime > 0.0f)
       {
           // 기존의 부드러운 전환 로직
           float InterpSpeed = 5.0f / TargetPreset->BlendTime;
           SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetPreset->TargetArmLength, DeltaTime, InterpSpeed);
           SpringArm->SetRelativeRotation(FMath::RInterpTo(SpringArm->GetRelativeRotation(), TargetPreset->Rotation, DeltaTime, InterpSpeed));
           MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetPreset->FieldOfView, DeltaTime, InterpSpeed));
       }
       else
       {
           // BlendTime이 0인 상태가 '유지'될 때는 값만 계속 고정시킴
           SpringArm->TargetArmLength = TargetPreset->TargetArmLength;
           SpringArm->SetRelativeRotation(TargetPreset->Rotation);
           MainCamera->SetFieldOfView(TargetPreset->FieldOfView);
       }
    }

    // 4. 포스트 프로세스 업데이트
    UpdatePostProcessSettings(TargetPreset->ManualFocusDistance, TargetPreset->ApertureFStop, TargetPreset->SensorWidth, 0.f, 0.f, 0.f);
}