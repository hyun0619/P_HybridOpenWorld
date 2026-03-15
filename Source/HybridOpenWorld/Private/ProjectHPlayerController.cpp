#include "ProjectHPlayerController.h"
#include "ProjectHCameraActor.h"
#include "Kismet/GameplayStatics.h"


void AProjectHPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AProjectHPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에 배치된 전용 카메라 액터 자동 연결
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectHCameraActor::StaticClass(), FoundCameras);
	
	if (FoundCameras.Num() > 0)
	{
		MainCameraActor = Cast<AProjectHCameraActor>(FoundCameras[0]);
		SetViewTarget(MainCameraActor); // 찾은 카메라로 화면 연결
	}
	
	// 초기 카메라 배정 로직 (나중에 Spawn이나 FindActor 등으로 구현 필요)
}