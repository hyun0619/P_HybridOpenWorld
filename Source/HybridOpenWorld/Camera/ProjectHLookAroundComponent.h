#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectHLookAroundComponent.generated.h"

class AProjectHCameraActor;
class AProjectHPlayerController;

/**
 * 좀보이드의 Look Around 기능을 본뜬 카메라 컴포넌트
 * 마우스를 화면 중심에서 멀리 이동하면 카메라가 해당 방향으로 패닝
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class HYBRIDOPENWORLD_API UProjectHLookAroundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectHLookAroundComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 컨트롤러에서 우클릭 상태를 전달받을 함수
	void SetLookAroundActive(bool bActive);

protected:
	// 컴포넌트 시작 시 필요한 레퍼런스를 캐싱하기 위한 함수
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="100.0"))
	float MaxPanDistance = 800.0f;

	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0"))
	float PanInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0"))
	float ReturnSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="0.1", ClampMax="2.0"))
	float ScreenDistanceMultiplier = 1.0f;

private:
	FVector CurrentOffset = FVector::ZeroVector;
	bool bIsLookAroundActive = false;

	// 매 프레임 Cast를 피하기 위한 컨트롤러 약참조 포인터
	UPROPERTY()
	TWeakObjectPtr<AProjectHPlayerController> CachedPC;

	// 내부적으로 카메라를 안전하게 가져오는 헬퍼 함수
	AProjectHCameraActor* GetMainCamera() const;
};