#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectHEdgeScrollComponent.generated.h"

class AProjectHCameraActor;

/**
 * 룩어라운드(Look Around) 카메라 컴포넌트
 *
 * 지정 키(기본: 우클릭)를 누른 채 마우스를 화면 중심에서 멀리 이동하면
 * 마우스가 있는 방향과 거리에 비례하여 카메라가 부드럽게 이동합니다.
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class HYBRIDOPENWORLD_API UProjectHEdgeScrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectHEdgeScrollComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 카메라가 뻗어나갈 수 있는 최대 거리 (언리얼 단위: cm)
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="100.0"))
	float MaxPanDistance = 800.0f;

	// 마우스를 움직일 때 카메라가 따라가는 반응 속도 (높을수록 마우스에 딱 붙어서 빠릿하게 움직임)
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0"))
	float PanInterpSpeed = 10.0f;

	// 우클릭을 떼었을 때 카메라가 플레이어 중심으로 돌아오는 속도
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0"))
	float ReturnSpeed = 5.0f;

	// 마우스를 얼마나 멀리 밀어야 MaxPanDistance에 도달할지 결정하는 화면 비율 (1.0 = 화면 끝까지 가야 최대 거리)
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="0.1", ClampMax="2.0"))
	float ScreenDistanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category="Look Around")
	FKey ActivationKey = EKeys::RightMouseButton;

private:
	FVector CurrentOffset = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraActor> CachedCamera;

	AProjectHCameraActor* GetCamera() const;
};