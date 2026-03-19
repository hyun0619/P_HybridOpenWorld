#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Data/LevelSettingsData.h"
#include "ProjectHPlayerController.generated.h"


class UNiagaraSystem;
class AProjectHCameraActor;
class UInputMappingContext;
class UInputAction;
class UGameMasterAsset;
/**
 * 플레이어 입력 처리, 레벨별 카메라 결정
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AProjectHPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	void SetInputModeByType(bool bIsWorldMap); // 조작 모드 변경 (true: 월드맵/마우스, false: 세부지역/WASD)
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "LevelData", meta=(ToolTip = "현재 레벨 데이터 테이블 열"))
	UGameMasterAsset* MasterLevelSettings; // 모든 레벨 데이터가 담긴 마스터 에셋 참조
	FLevelSettingsRow CurrentLevelRow; // 현재 레벨 데이터 테이블 열
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "항상 켜있는 기능 (인벤토리, 지도 등)"))
	UInputMappingContext* IMC_Global; // 항상 켜있는 기능 (인벤토리, 지도 등)
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "월드맵 전용 - 마우스 이동"))
	UInputMappingContext* IMC_WorldMap; // 월드맵 전용 - 마우스 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "세부지역 전용 - WASD 이동"))
	UInputMappingContext* IMC_Detailed; // 세부지역 전용 - WASD 이동
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "키보드 이동"))
	UInputAction* IA_Move_KeyBoard; // 키보드 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "마우스 클릭 이동 "))
	UInputAction* IA_Move_MouseClick; // 마우스 클릭 이동 
	/*추후 추가될 기능들 자리*/
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Effect", meta=(ToolTip = "클릭 효과"))
	UNiagaraSystem* FXCursor; // 템플릿에서 가져온 클릭 효과
	
private:
	// 입력 핸들러 - 필요한 시점에만 호출
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
	/*추후 추가될 기능들 자리*/
	void InitEssentialReferences(); // 기초 객체(카메라, GI 등) 캐싱
	void FetchLevelData(); // 현재 레벨 타입에 맞는 데이터를 테이블에서 검색 및 캐싱
	void ApplyInitialLevelSetup(); // 검색된 레벨 환경 설정 (스폰, 카메라, 입력)
	void SetupWorldMapInput(); // 입출력 세부 설정 - 월드맵
	void SetupDetailedInput(); // 입출력 세부 설정 - 세부레벨
	void HandleInitialSpawn(); // 캐릭터 스폰 및 배치 로직
	void ApplyCameraPreset(); // 카메라 프리셋 수치 적용
	
	UPROPERTY()
	AProjectHCameraActor* MainCameraActor; // 현재 제어 중인 카메라 참조
	
	float CurrentTrackingSpeed = 5.0f; // 현재 프리셋의 속도값 저장
	bool bCachedFollowPawn = true; // 카메라의 폰 추적 유무를 체크
	bool bHasValidLevelData = false; // 데이터 찾았는지 확인할 플래그
};
