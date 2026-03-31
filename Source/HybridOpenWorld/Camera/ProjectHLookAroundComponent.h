#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectHLookAroundComponent.generated.h"

class AProjectHCameraActor;
class AProjectHPlayerController;

/**
 * 좀보이드의 Look Around 기능을 본뜬 카메라 컴포넌트
 * 마우스의 화면 위치를 계산하여 카메라에 추가적인 오프셋 부여
 * 마우스를 화면 중심에서 멀리 이동하면 카메라가 해당 방향으로 패닝
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class HYBRIDOPENWORLD_API UProjectHLookAroundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectHLookAroundComponent();
	
	// 매 프레임 마우스 위치 계산 및 카메라 오프셋 적용
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 컨트롤러에서 우클릭 상태를 전달받을 함수
	void SetLookAroundActive(bool bActive);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="100.0",
		DisplayName="최대 이동 거리", ToolTip="마우스가 화면 끝일 때 카메라가 이동할 월드 거리"))
	float MaxPanDistance = 800.0f;
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0",
		DisplayName="보간 속도", ToolTip="시야를 확장할 때의 부드러운 정도"))
	float PanInterpSpeed = 10.0f;
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="1.0",
		DisplayName="기능 껐을 때 원래 위치로 돌아오는 속도"))
	float ReturnSpeed = 5.0f;
	UPROPERTY(EditAnywhere, Category="Look Around", meta=(ClampMin="0.1", ClampMax="2.0",
		DisplayName="마우스 거리에 따른 민감도 배율", ToolTip="1.0이면 화면 절반 거리에서 최대치 도달"))
	float ScreenDistanceMultiplier = 1.0f;

private:
	FVector CurrentOffset = FVector::ZeroVector; // 현재 적용 중인 카메라 오프셋 값
	
	bool bIsLookAroundActive = false; // 기능 활성화 여부

	// 캐싱된 컨트롤러 포인터, TWeakObjectPtr 사용하여 컨트롤러 파괴되었을 때의 안전성 확보
	UPROPERTY()
	TWeakObjectPtr<AProjectHPlayerController> CachedPC;
	
	AProjectHCameraActor* GetMainCamera() const; // 현재 제어 중인 메인 카메라 액터를 안전하게 가져오는 함수
};