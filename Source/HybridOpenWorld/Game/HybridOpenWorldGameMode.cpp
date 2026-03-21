#include "HybridOpenWorldGameMode.h"
#include "Character/HybridOpenWorldCharacter.h"
#include "Player/ProjectHPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AHybridOpenWorldGameMode::AHybridOpenWorldGameMode()
{
	// C++ 클래스를 기본으로 지정
	PlayerControllerClass = AProjectHPlayerController::StaticClass();
	DefaultPawnClass = AHybridOpenWorldCharacter::StaticClass();
    
	// 캐릭터 블루프린트 찾기
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/HybridOpenWorld/Characters/BP_MainCharacter.BP_MainCharacter_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
    
	// 플레이어 컨트롤러 블루프린트 찾기
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/HybridOpenWorld/01_Game/BP_ProjectHPlayerController.BP_ProjectHPlayerController_C"));
	if (PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}