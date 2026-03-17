#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "ProjectHGameInstance.generated.h"

/**
 * 게임 종료 전까지 유지되어야 하는 핵심 데이터들
 */
USTRUCT(BlueprintType)
struct FPlayerPersistenceData
{
	GENERATED_BODY()
	
	// 플레이어 월드맵 위치, 방향 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LastWorldLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LastWorldRotation = FRotator::ZeroRotator;
	
	// 마지막 플레이 월드 시간 저장 - 초기값은 -1로 설정해 데이터 없음을 표기
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedTime = -1.0f;
	
	// 추후 현재 HP나 소지 골드 등 추가
};

/**
 * 프로젝트 전체의 전역 데이터와 서브시스템을 관리하는 클래스
 */
UCLASS()
class HYBRIDOPENWORLD_API UProjectHGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category="Persistence")
	FPlayerPersistenceData PlayerData; // 구조체로 데이터 관리
	
	UPROPERTY(BlueprintReadWrite, Category = "LevelMove")
	FGameplayTag PendingSpawnTag; // 다음 레벨에 도착했을 때 찾아갈 위치 태그
};
