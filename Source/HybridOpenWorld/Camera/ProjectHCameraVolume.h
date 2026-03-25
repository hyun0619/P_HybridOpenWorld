#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectHCameraVolume.generated.h"

class UBoxComponent;
class UCameraPresetDataAsset;

UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraVolume : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraVolume();

protected:
	virtual void BeginPlay() override;

	// 플레이어가 들어오고 나갈 때 감지하는 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// 에디터에서 영역을 조절할 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* CollisionBox;

	// 이 구역에서 사용할 카메라 설정 데이터
	UPROPERTY(EditAnywhere, Category = "Camera Setup")
	UCameraPresetDataAsset* CameraPreset;

	// 볼륨이 겹칠 때 어떤 뷰를 우선할지 (높을수록 우선)
	UPROPERTY(EditAnywhere, Category = "Camera Setup")
	int32 Priority = 0;
};