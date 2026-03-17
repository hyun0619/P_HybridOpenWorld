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
	
	UPROPERTY(EditAnywhere, Category="Portal", meta=(DisplayName="목적지 레벨") ,meta=(ToolTip = "설정한 레벨로 스폰"))
	TSoftObjectPtr<UWorld> TargetLevel;
	UPROPERTY(EditAnywhere, Category="Portal", meta=(DisplayName="스폰 지점 태그") ,meta=(ToolTip = "설정한 태그의 좌표로 스폰"))
	FGameplayTag TargetSpawnTag;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
};
