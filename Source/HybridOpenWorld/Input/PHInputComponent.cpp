#include "Input/PHInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"


UPHInputComponent::UPHInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPHInputComponent::ApplyInputState(APlayerController* PC, EInputState NewState)
{
	if (!PC || !PC->GetLocalPlayer()) return;

	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	Subsystem->ClearAllMappings();
	if (IMC_Global) Subsystem->AddMappingContext(IMC_Global, 0);

	switch (NewState)
	{
	case EInputState::WorldMap:
		if (IMC_WorldMap) Subsystem->AddMappingContext(IMC_WorldMap, 1);
		PC->SetInputMode(FInputModeGameAndUI()
			.SetHideCursorDuringCapture(false)
			.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock));
		PC->bShowMouseCursor = true;
		break;

	case EInputState::Detailed:
		// ★ 수정: GameOnly → GameAndUI 로 변경
		// WASD 이동 + 마우스 커서 표시 + UI 상호작용 + 엣지스크롤 가능
		if (IMC_Detailed) Subsystem->AddMappingContext(IMC_Detailed, 1);
		PC->SetInputMode(FInputModeGameAndUI()
			.SetHideCursorDuringCapture(false)
			.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock));
		PC->bShowMouseCursor = true;
		break;

	case EInputState::UIOverlay:
		if (IMC_UIOverlay) Subsystem->AddMappingContext(IMC_UIOverlay, 1);
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		break;

	case EInputState::Dialogue:
		PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(true));
		PC->bShowMouseCursor = false;
		break;

	case EInputState::Cinematic:
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		break;
	}
}