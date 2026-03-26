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
    CollisionBox->SetCollisionProfileName(TEXT("Trigger")); 

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

void AProjectHCameraVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn->IsPlayerControlled())
    {
       if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
       {
          // ActiveSettings가 아니라 LocalSettings를 넘겨줍니다.
          Subsystem->PushCameraPreset(LocalSettings, Priority, this);
       }
    }
}

void AProjectHCameraVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void AProjectHCameraVolume::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (PreviewSpringArm && PreviewCamera)
    {
        // 뷰포트에서 직접 조절하는 LocalSettings 값을 프리뷰에 반영합니다.
        PreviewSpringArm->TargetArmLength = LocalSettings.TargetArmLength;
        PreviewSpringArm->SetRelativeRotation(LocalSettings.Rotation);
        PreviewSpringArm->SocketOffset = LocalSettings.CameraOffset;
        PreviewCamera->SetFieldOfView(LocalSettings.FieldOfView);

        ApplyPreviewPostProcessing();
    }
}

void AProjectHCameraVolume::ApplyPreviewPostProcessing()
{
    if (!PreviewCamera) return;

    FPostProcessSettings& PP_Settings = PreviewCamera->PostProcessSettings;
    bool bEnable = LocalSettings.bEnableTiltShift;
    
    PP_Settings.bOverride_DepthOfFieldFstop = bEnable;
    PP_Settings.bOverride_DepthOfFieldSensorWidth = bEnable;
    PP_Settings.bOverride_DepthOfFieldFocalDistance = bEnable;
    PP_Settings.bOverride_DepthOfFieldNearBlurSize = bEnable;
    PP_Settings.bOverride_DepthOfFieldFarBlurSize = bEnable;
    PP_Settings.bOverride_DepthOfFieldFarTransitionRegion = bEnable;

    if (bEnable)
    {
       PP_Settings.DepthOfFieldFstop = LocalSettings.ApertureFStop; 
       PP_Settings.DepthOfFieldSensorWidth = LocalSettings.SensorWidth; 
       PP_Settings.DepthOfFieldFocalDistance = LocalSettings.ManualFocusDistance;
       PP_Settings.DepthOfFieldNearBlurSize = LocalSettings.NearBlurRadius;
       PP_Settings.DepthOfFieldFarBlurSize = LocalSettings.FarBlurRadius;
       PP_Settings.DepthOfFieldFarTransitionRegion = LocalSettings.FarTransitionRegion;
    }
}

// ========================================================
// ★ 핵심 기능: 데이터 보존을 위한 동기화 버튼 로직
// ========================================================
void AProjectHCameraVolume::LoadFromDataAsset()
{
    if (LinkedDataAsset)
    {
        LocalSettings = LinkedDataAsset->Settings;
        OnConstruction(GetActorTransform()); // 뷰포트 즉시 갱신
        UE_LOG(LogTemp, Log, TEXT("DA 원본에서 데이터를 불러왔습니다."));
    }
}

void AProjectHCameraVolume::SaveToDataAsset()
{
    if (LinkedDataAsset)
    {
        // 에디터 패키지를 '수정됨(별표)' 상태로 만들어 물리적 파일 저장을 유도합니다.
        LinkedDataAsset->Modify(); 
        
        // 뷰포트에서 열심히 깎은 수치를 DA 원본 파일에 덮어씌웁니다!
        LinkedDataAsset->Settings = LocalSettings;
        UE_LOG(LogTemp, Warning, TEXT("[%s] 에 카메라 셋팅이 영구적으로 저장되었습니다!"), *LinkedDataAsset->GetName());
    }
}