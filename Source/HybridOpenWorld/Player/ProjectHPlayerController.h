#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Data/LevelSettingsData.h"
#include "ProjectHPlayerController.generated.h"


class UNiagaraSystem;
class AProjectHCameraActor;
class UInputAction;
class UGameMasterAsset;
class UProjectHInputComponent;
/**
 * 플레이어 입력 처리, 레벨별 카메라 결정
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AProjectHPlayerController();
	
	AProjectHCameraActor* GetMainCameraActor() const; // Character가 카메라 방향 읽어가게끔
	
	UFUNCTION(BlueprintCallable, Category="InputState")
	void ChangeInputState(EInputState NewState); // 외부(NPC, UI)에서 입력 상태를 변경할 때 호출
	UFUNCTION(BlueprintCallable, Category="InputState")
	void RevertToDefaultState(); // 원래 레벨 기본 상태로 돌아갈 때 호출
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectHInputComponent* InputManager; // 입력 전담 컴포넌트
	EInputState DefaultState; // 이 레벨의 기본 상태
	EInputState CurrentState; // 현재 내 상태
	
	UPROPERTY(EditDefaultsOnly, Category = "LevelData", meta=(ToolTip = "현재 레벨 데이터 테이블 열"))
	UGameMasterAsset* MasterLevelSettings; // 모든 레벨 데이터가 담긴 마스터 에셋 참조
	FLevelSettingsRow CurrentLevelRow; // 현재 레벨 데이터 테이블 열
	
private:
	void InitEssentialReferences(); // 기초 객체(카메라, GI 등) 캐싱
	void FetchLevelData(); // 현재 레벨 타입에 맞는 데이터를 테이블에서 검색 및 캐싱
	void ApplyInitialLevelSetup(); // 검색된 레벨 환경 설정 (스폰, 카메라, 입력)
	void HandleInitialSpawn(); // 캐릭터 스폰 및 배치 로직
	
	UPROPERTY()
	AProjectHCameraActor* MainCameraActor; // 현재 제어 중인 카메라 참조
	
	float CurrentTrackingSpeed = 5.0f; // 현재 프리셋의 속도값 저장
	bool bCachedFollowPawn = true; // 카메라의 폰 추적 유무를 체크
	bool bHasValidLevelData = false; // 데이터 찾았는지 확인할 플래그
};
