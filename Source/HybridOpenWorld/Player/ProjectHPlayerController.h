#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Data/LevelSettingsData.h"
#include "ProjectHPlayerController.generated.h"


class AProjectHCameraActor;
class UInputAction;
class UInputMappingContext;
class UGameMasterAsset;
class UProjectHInputComponent;
class UProjectHLookAroundComponent;
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
    virtual void SetupInputComponent() override;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
    UInputAction* IA_LookAround; // 둘러보기 액션
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UProjectHInputComponent* InputManager;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UProjectHLookAroundComponent* EdgeScrollComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UProjectHOcclusionFadeComponent* OcclusionFadeComponent;
    
    EInputState DefaultState;
    EInputState CurrentState;

    UPROPERTY(EditDefaultsOnly, Category="LevelData")
    UGameMasterAsset* MasterLevelSettings;
    
    FLevelSettingsRow CurrentLevelRow;

private:
    void OnLookAroundStarted(const FInputActionValue& Value);
    void OnLookAroundCompleted(const FInputActionValue& Value);
    
    void InitEssentialReferences();
    void FetchLevelData();
    void ApplyInitialLevelSetup();
    void HandleInitialSpawn();

    UPROPERTY()
    AProjectHCameraActor* MainCameraActor;

    bool bHasValidLevelData = false;
};