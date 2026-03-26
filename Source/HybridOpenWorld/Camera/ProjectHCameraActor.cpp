#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "ProjectHCameraSubsystem.h" 
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
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
    SpringArm->CameraLagMaxDistance = 1000.0f; 
}

void AProjectHCameraActor::BeginPlay() { Super::BeginPlay(); }

void AProjectHCameraActor::UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation)
{
    if (SpringArm && MainCamera)
    {
       SpringArm->TargetArmLength = TargetArmLength;
       SpringArm->SetRelativeRotation(Rotation);
       MainCamera->SetFieldOfView(FOV);
    }
}

void AProjectHCameraActor::UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop, float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition)
{
    if (!MainCamera) return;

    FPostProcessSettings& PP_Settings = MainCamera->PostProcessSettings;

    PP_Settings.bOverride_DepthOfFieldFstop = bEnable;
    PP_Settings.bOverride_DepthOfFieldSensorWidth = bEnable;
    PP_Settings.bOverride_DepthOfFieldFocalDistance = bEnable;
    PP_Settings.bOverride_DepthOfFieldNearBlurSize = bEnable;
    PP_Settings.bOverride_DepthOfFieldFarBlurSize = bEnable;
    PP_Settings.bOverride_DepthOfFieldFarTransitionRegion = bEnable;

    if (bEnable)
    {
       PP_Settings.DepthOfFieldFstop = InFStop; 
       PP_Settings.DepthOfFieldSensorWidth = InSensorWidth; 
       PP_Settings.DepthOfFieldFocalDistance = InFocalDist;
       PP_Settings.DepthOfFieldNearBlurSize = InNearBlur;
       PP_Settings.DepthOfFieldFarBlurSize = InFarBlur;
       PP_Settings.DepthOfFieldFarTransitionRegion = InFarTransition;
    }
}

FVector AProjectHCameraActor::GetCameraTargetLocation() const
{
    FVector CameraLoc = MainCamera->GetComponentLocation();
    FVector ForwardDir = MainCamera->GetForwardVector();
    FVector IntersectionPoint;
    
    bool bIntersect = FMath::SegmentPlaneIntersection(CameraLoc, CameraLoc + (ForwardDir * 10000.f), FPlane(FVector::UpVector, 0.f), IntersectionPoint);
    return bIntersect ? IntersectionPoint : (CameraLoc + ForwardDir * 2000.f);
}

void AProjectHCameraActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>();
    if (!Subsystem) return;

    FCameraPresetSettings TargetPreset;
    if (!Subsystem->GetActivePreset(TargetPreset)) return;

    AActor* ActiveVolume = Subsystem->GetActiveInstigator();

    bool bVolumeChanged = (LastVolume != ActiveVolume);
    LastVolume = ActiveVolume;
    bool bHardCut = bIsFirstTick || (bVolumeChanged && TargetPreset.BlendTime <= 0.0f);
    bIsFirstTick = false;

    // ★ 최적화: PlayerController 캐싱 (Tick 내부 반복 호출 제거)
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

    // --- [1. 타겟 목표값 계산] ---
    FVector TargetLoc = ActiveVolume ? ActiveVolume->GetActorLocation() : GetActorLocation(); 
    if (TargetPreset.bFollowPawn && PlayerPawn)
    {
        TargetLoc = PlayerPawn->GetActorLocation();
    }

    FRotator TargetRot = TargetPreset.Rotation;
    if (ActiveVolume)
    {
        TargetRot = ActiveVolume->GetTransform().TransformRotation(TargetPreset.Rotation.Quaternion()).Rotator();
    }

    // --- [2. 실제 값 적용 및 보간] ---
    if (bHardCut)
    {
        SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
        
        SpringArm->TargetArmLength = TargetPreset.TargetArmLength;
        SpringArm->SetWorldRotation(TargetRot); 
        SpringArm->SocketOffset = TargetPreset.CameraOffset;
        MainCamera->SetFieldOfView(TargetPreset.FieldOfView);

        SpringArm->bEnableCameraLag = false;
        SpringArm->bEnableCameraRotationLag = false;
        SpringArm->UpdateChildTransforms(); 
        
        if (PC && PC->PlayerCameraManager) PC->PlayerCameraManager->SetGameCameraCutThisFrame();
    }
    else
    {
        // 최적화: 속도 계산 일원화
        float CamInterpSpeed = TargetPreset.BlendTime > 0.0f ? 5.0f / TargetPreset.BlendTime : 9999.0f;
        float LocInterpSpeed = TargetPreset.bFollowPawn ? TargetPreset.TrackingInterpSpeed : CamInterpSpeed;

        SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, LocInterpSpeed));
        
        SpringArm->bEnableCameraLag = bUseCameraLag; 
        SpringArm->bEnableCameraRotationLag = true; 

        SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetPreset.TargetArmLength, DeltaTime, CamInterpSpeed);
        SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, DeltaTime, CamInterpSpeed));
        SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, TargetPreset.CameraOffset, DeltaTime, CamInterpSpeed);
        MainCamera->SetFieldOfView(FMath::FInterpTo(MainCamera->FieldOfView, TargetPreset.FieldOfView, DeltaTime, CamInterpSpeed));
    }

    // --- [3. 포스트 프로세스] ---
    UpdatePostProcessSettings(TargetPreset.bEnableTiltShift, TargetPreset.ManualFocusDistance, TargetPreset.ApertureFStop, TargetPreset.SensorWidth, TargetPreset.NearBlurRadius, TargetPreset.FarBlurRadius, TargetPreset.FarTransitionRegion);
}