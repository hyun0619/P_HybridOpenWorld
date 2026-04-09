#include "PHPortal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PHGameInstance.h"


APHPortal::APHPortal()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APHPortal::OnOverlapBegin);
}

void APHPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// ★ IsA(APawn) 대신 Cast + IsPlayerControlled 체크
	// AI 폰이나 NPC가 포탈을 밟아도 레벨 이동이 발생하지 않도록 방어
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
 
	UPHGameInstance* GI = Cast<UPHGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI || TargetLevel.IsNull()) return;
 
	GI->PendingSpawnTag = TargetSpawnTag;
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TargetLevel);
}