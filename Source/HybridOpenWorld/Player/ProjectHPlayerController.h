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
class UProjectHEdgeScrollComponent;
class UProjectHOcclusionFadeComponent;

UCLASS()
class HYBRIDOPENWORLD_API AProjectHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AProjectHPlayerController();

	AProjectHCameraActor* GetMainCameraActor() const;

	UFUNCTION(BlueprintCallable, Category="InputState")
	void ChangeInputState(EInputState NewState);
	UFUNCTION(BlueprintCallable, Category="InputState")
	void RevertToDefaultState();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectHInputComponent* InputManager;

	/** ★ 엣지스크롤 컴포넌트 — 우클릭+마우스 가장자리로 카메라 이동 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectHEdgeScrollComponent* EdgeScrollComponent;

	/** ★ 장애물 투명화 컴포넌트 — 카메라↔캐릭터 사이 장애물 페이드 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectHOcclusionFadeComponent* OcclusionFadeComponent;

	EInputState DefaultState;
	EInputState CurrentState;

	UPROPERTY(EditDefaultsOnly, Category="LevelData")
	UGameMasterAsset* MasterLevelSettings;
	FLevelSettingsRow CurrentLevelRow;

private:
	void InitEssentialReferences();
	void FetchLevelData();
	void ApplyInitialLevelSetup();
	void HandleInitialSpawn();

	UPROPERTY()
	AProjectHCameraActor* MainCameraActor;

	bool bHasValidLevelData = false;
};