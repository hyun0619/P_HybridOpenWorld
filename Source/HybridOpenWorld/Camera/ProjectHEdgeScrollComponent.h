#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectHEdgeScrollComponent.generated.h"

class AProjectHCameraActor;

/**
 * 엣지스크롤 컴포넌트
 *
 * 지정 키(기본: 우클릭)를 누른 채 마우스를 화면 가장자리로 이동하면
 * 카메라가 해당 방향으로 부드럽게 이동합니다.
 *
 * [셋업] PlayerController의 C++ 생성자에서 CreateDefaultSubobject로 추가
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class HYBRIDOPENWORLD_API UProjectHEdgeScrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectHEdgeScrollComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, Category="Edge Scroll", meta=(ClampMin="0.01", ClampMax="0.3"))
	float EdgeThreshold = 0.05f;

	UPROPERTY(EditAnywhere, Category="Edge Scroll", meta=(ClampMin="100.0"))
	float ScrollSpeed = 800.0f;

	UPROPERTY(EditAnywhere, Category="Edge Scroll", meta=(ClampMin="100.0"))
	float MaxOffset = 600.0f;

	UPROPERTY(EditAnywhere, Category="Edge Scroll", meta=(ClampMin="1.0"))
	float ReturnSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category="Edge Scroll")
	FKey ActivationKey = EKeys::RightMouseButton;

private:
	FVector CurrentOffset = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraActor> CachedCamera;

	AProjectHCameraActor* GetCamera() const;
};