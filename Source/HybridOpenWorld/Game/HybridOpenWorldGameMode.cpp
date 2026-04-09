#include "HybridOpenWorldGameMode.h"
#include "Character/HybridOpenWorldCharacter.h"
#include "Player/PHPlayerController.h"

AHybridOpenWorldGameMode::AHybridOpenWorldGameMode()
{
	// C++ 클래스를 기본으로 지정
	PlayerControllerClass = APHPlayerController::StaticClass();
	DefaultPawnClass = AHybridOpenWorldCharacter::StaticClass();
}