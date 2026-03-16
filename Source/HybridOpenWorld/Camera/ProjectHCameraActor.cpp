#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"


AProjectHCameraActor::AProjectHCameraActor()
{
	PrimaryActorTick.bCanEverTick = false; // 컨트롤러가 위치를 업데이트하므로 카메라에선 Tick 필요 x
	
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

