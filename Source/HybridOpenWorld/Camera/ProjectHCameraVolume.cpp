#include "ProjectHCameraVolume.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "ProjectHCameraSubsystem.h" // 매니저 헤더
#include "Data/CameraPresetDataAsset.h"    // 데이터 에셋 헤더

AProjectHCameraVolume::AProjectHCameraVolume()
{
	PrimaryActorTick.bCanEverTick = false; // 볼륨은 연산이 필요 없으므로 끕니다.

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
    
	// 트리거 설정 (물리 충돌은 없고 겹침만 감지)
	CollisionBox->SetCollisionProfileName(TEXT("Trigger"));

	// 이벤트 바인딩
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AProjectHCameraVolume::OnOverlapEnd);
}

void AProjectHCameraVolume::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectHCameraVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 닿은 액터가 플레이어인지 확인
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (PlayerPawn && PlayerPawn->IsPlayerControlled())
	{
		if (UProjectHCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<UProjectHCameraSubsystem>())
		{
			Subsystem->PushCameraPreset(CameraPreset, Priority);
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
			Subsystem->PopCameraPreset(CameraPreset);
		}
	}
}