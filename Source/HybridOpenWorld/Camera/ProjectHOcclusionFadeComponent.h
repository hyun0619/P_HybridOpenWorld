#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectHOcclusionFadeComponent.generated.h"

class AProjectHCameraActor;

/**
 * 장애물 투명화 컴포넌트
 *
 * 카메라 ↔ 캐릭터 사이에 SphereTrace를 쏴서 가리는 오브젝트의
 * 머티리얼을 반투명으로 전환하고, 캐릭터에 CustomDepth 실루엣을 활성화합니다.
 *
 * [필수 셋업]
 * 1) 투명해질 장애물 메시에 "CameraFade" 태그 추가 (또는 bFadeAllHits=true)
 * 2) 장애물 머티리얼에 스칼라 파라미터 "Opacity" 추가
 * 3) 캐릭터 실루엣은 PostProcess 머티리얼에서 CustomDepth 기반 렌더링
 *
 * [셋업] PlayerController의 C++ 생성자에서 CreateDefaultSubobject로 추가
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class HYBRIDOPENWORLD_API UProjectHOcclusionFadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectHOcclusionFadeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, Category="Occlusion Fade", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FadedOpacity = 0.15f;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade", meta=(ClampMin="1.0"))
	float FadeSpeed = 8.0f;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade", meta=(ClampMin="1.0"))
	float TraceRadius = 30.0f;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade")
	bool bFadeAllHits = false;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade")
	FName OpacityParameterName = TEXT("Opacity");

	UPROPERTY(EditAnywhere, Category="Occlusion Fade")
	bool bEnableCharacterSilhouette = true;

	UPROPERTY(EditAnywhere, Category="Occlusion Fade", meta=(EditCondition="bEnableCharacterSilhouette"))
	int32 SilhouetteStencilValue = 1;

private:
	struct FFadedActorInfo
	{
		TWeakObjectPtr<AActor> Actor;
		TArray<TWeakObjectPtr<UMeshComponent>> MeshComponents;
		TArray<TArray<UMaterialInterface*>> OriginalMaterials;
		TArray<TArray<UMaterialInstanceDynamic*>> DynamicMaterials;
		float CurrentOpacity = 1.0f;
	};

	TArray<FFadedActorInfo> FadedActors;

	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraActor> CachedCamera;

	bool bWasSilhouetteActive = false;

	TArray<AActor*> PerformOcclusionTrace() const;
	void BeginFade(AActor* Actor);
	void CleanupRestoredActors();
	void SetCharacterSilhouette(bool bEnabled);
	int32 FindFadedActorIndex(AActor* Actor) const;
};