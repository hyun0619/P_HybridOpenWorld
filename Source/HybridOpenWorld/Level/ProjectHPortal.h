#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectHPortal.generated.h"


class UBoxComponent;
/**
 * 특정 레벨의 특정 태그로 이동시켜주는 포탈 액터
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHPortal : public AActor
{
	GENERATED_BODY()

public:
	AProjectHPortal();

protected:
	UPROPERTY(VisibleAnywhere, Category="Portal")
	UBoxComponent* CollisionBox;
	
	// 에디터 디테일 창에서 설정할 목적지 정보
	UPROPERTY(EditAnywhere, Category="Portal")
	TSoftObjectPtr<UWorld> TargetLevel; // 이동할 레벨
	UPROPERTY(EditAnywhere, Category="Portal")
	FGameplayTag TargetSpawnTag; // 도착지 입구 태그

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
};
