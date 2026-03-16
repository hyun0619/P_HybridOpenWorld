#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"


AProjectHCameraActor::AProjectHCameraActor()
{
	PrimaryActorTick.bCanEverTick = false; // 컨트롤러가 대신 계산해주기에 카메라에선 Tick 필요 x
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false; // 월드맵 뷰에서 장애물에 카메라 튀는 현상 방지
	
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm);
	
	// 스트리밍 소스 컴포넌트 생성
	StreamingSourceComponent = CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>(TEXT("StreamingSourceComponent"));
    
	// ========================================================
	// UE 5.5 주의사항:
	// TargetState와 Shapes는 엔진 내부에서 Private으로 잠겼습니다.
	// 코드로 강제 수정하면 빌드 에러가 발생합니다.
	// 
	// ※ 컴포넌트가 생성되는 순간 기본적으로 'Activated' 상태로 자동 적용됩니다.
	// ※ 50m 반경 설정은 에디터로 돌아가서 디테일 패널에서 설정해야 합니다.
	// ========================================================
}

void AProjectHCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AProjectHCameraActor::GetCameraTargetLocation() const
{
	// 카메라가 바라보는 방향으로 월드맵 로드
	FVector CameraLoc = MainCamera->GetComponentLocation();
	FVector ForwardDir = MainCamera->GetForwardVector();
	
	// 카메라가 하늘이나 정면을 볼 때 분모가 0이 되어 크래시 나는 것을 방지
	float SafeZ = FMath::IsNearlyZero(ForwardDir.Z) ? -0.0001f : ForwardDir.Z;
	// 단순화한 레이캐스트 계산 : (지면 높이 - 카메라 높이) / 방향의 Z값
	float DistanceToGround = -CameraLoc.Z / SafeZ;
	// 너무 멀리 있는 좌표가 나오지 않도록 Clamp 처리
	return CameraLoc + (ForwardDir * FMath::Clamp(DistanceToGround, 0.f, 10000.f));
}

void AProjectHCameraActor::SetCameraMode(bool bIsWorldMap)
{
	// 모드 전환 시 한 번에 값들을 변경
	if (bIsWorldMap)
	{
		MainCamera->SetFieldOfView(WorldMapFOV);
		SpringArm->TargetArmLength = WorldMapArmLength;
	}
	else
	{
		MainCamera->SetFieldOfView(DetailedFOV);
		SpringArm->TargetArmLength = DetailedArmLength;
	}
}
