#include "ProjectHPortal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ProjectHGameInstance.h"


AProjectHPortal::AProjectHPortal()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectHPortal::OnOverlapBegin);
}

void AProjectHPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어가 포탈에 닿았는지 확인
	if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GI && !TargetLevel.IsNull())
		{
			// 다음 레벨에서 사용할 태그를 GameInstance에 담기
			GI->PendingSpawnTag = TargetSpawnTag;

			// 레벨 이동 시작
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, TargetLevel);
		}
	}
}