#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectHPlayerController.generated.h"


class AProjectHCameraActor;

/**
 * 플레이어 입력 처리, 레벨별 카메라 결정
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void PlayerTick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	AProjectHCameraActor* MainCameraActor; // 현재 제어 중인 카메라 참조
};
