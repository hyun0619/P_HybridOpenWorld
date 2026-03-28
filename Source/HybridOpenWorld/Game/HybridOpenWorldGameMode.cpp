#include "HybridOpenWorldGameMode.h"
#include "Character/HybridOpenWorldCharacter.h"
#include "Player/ProjectHPlayerController.h"

AHybridOpenWorldGameMode::AHybridOpenWorldGameMode()
{
	// C++ 클래스를 기본으로 지정
	PlayerControllerClass = AProjectHPlayerController::StaticClass();
	DefaultPawnClass = AHybridOpenWorldCharacter::StaticClass();
}