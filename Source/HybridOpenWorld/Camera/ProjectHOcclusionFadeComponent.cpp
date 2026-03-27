#include "ProjectHOcclusionFadeComponent.h"
#include "ProjectHCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

UProjectHOcclusionFadeComponent::UProjectHOcclusionFadeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UProjectHOcclusionFadeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto& Info : FadedActors)
	{
		if (!Info.Actor.IsValid()) continue;
		for (int32 m = 0; m < Info.MeshComponents.Num(); ++m)
		{
			if (!Info.MeshComponents[m].IsValid()) continue;
			for (int32 s = 0; s < Info.OriginalMaterials[m].Num(); ++s)
				if (Info.OriginalMaterials[m][s])
					Info.MeshComponents[m]->SetMaterial(s, Info.OriginalMaterials[m][s]);
		}
	}
	FadedActors.Empty();
	SetCharacterSilhouette(false);
	Super::EndPlay(EndPlayReason);
}

void UProjectHOcclusionFadeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedCamera.IsValid())
	{
		CachedCamera = Cast<AProjectHCameraActor>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProjectHCameraActor::StaticClass()));
		if (!CachedCamera.IsValid()) return;
	}

	TArray<AActor*> CurrentOccluders = PerformOcclusionTrace();

	for (AActor* Occ : CurrentOccluders)
		if (FindFadedActorIndex(Occ) == INDEX_NONE)
			BeginFade(Occ);

	for (auto& Info : FadedActors)
	{
		if (!Info.Actor.IsValid()) continue;
		const bool bStill = CurrentOccluders.Contains(Info.Actor.Get());
		const float Target = bStill ? FadedOpacity : 1.0f;
		Info.CurrentOpacity = FMath::FInterpTo(Info.CurrentOpacity, Target, DeltaTime, FadeSpeed);
		for (auto& MeshDynMats : Info.DynamicMaterials)
			for (auto* DynMat : MeshDynMats)
				if (DynMat) DynMat->SetScalarParameterValue(OpacityParameterName, Info.CurrentOpacity);
	}

	CleanupRestoredActors();
	SetCharacterSilhouette(FadedActors.Num() > 0);
}

TArray<AActor*> UProjectHOcclusionFadeComponent::PerformOcclusionTrace() const
{
	TArray<AActor*> Occluders;
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return Occluders;
	APawn* Pawn = PC->GetPawn();
	if (!Pawn || !CachedCamera.IsValid()) return Occluders;

	const FVector Start = CachedCamera->GetMainCamera()->GetComponentLocation();
	const FVector End = Pawn->GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CachedCamera.Get());
	Params.AddIgnoredActor(Pawn);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
		TraceChannel, FCollisionShape::MakeSphere(TraceRadius), Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* A = Hit.GetActor();
		if (!A || A == Pawn) continue;
		if (!bFadeAllHits && !A->ActorHasTag(TEXT("CameraFade"))) continue;
		Occluders.AddUnique(A);
	}
	return Occluders;
}

void UProjectHOcclusionFadeComponent::BeginFade(AActor* Actor)
{
	if (!Actor) return;
	FFadedActorInfo Info;
	Info.Actor = Actor;
	Info.CurrentOpacity = 1.0f;

	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;
		Info.MeshComponents.Add(Mesh);
		TArray<UMaterialInterface*> Origs;
		TArray<UMaterialInstanceDynamic*> Dyns;
		for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
		{
			Origs.Add(Mesh->GetMaterial(i));
			UMaterialInstanceDynamic* D = Mesh->CreateAndSetMaterialInstanceDynamic(i);
			if (D) D->SetScalarParameterValue(OpacityParameterName, 1.0f);
			Dyns.Add(D);
		}
		Info.OriginalMaterials.Add(Origs);
		Info.DynamicMaterials.Add(Dyns);
	}
	FadedActors.Add(MoveTemp(Info));
}

void UProjectHOcclusionFadeComponent::CleanupRestoredActors()
{
	for (int32 i = FadedActors.Num() - 1; i >= 0; --i)
	{
		auto& Info = FadedActors[i];
		if (!Info.Actor.IsValid()) { FadedActors.RemoveAt(i); continue; }
		if (Info.CurrentOpacity > 0.99f)
		{
			for (int32 m = 0; m < Info.MeshComponents.Num(); ++m)
			{
				if (!Info.MeshComponents[m].IsValid()) continue;
				for (int32 s = 0; s < Info.OriginalMaterials[m].Num(); ++s)
					if (Info.OriginalMaterials[m][s])
						Info.MeshComponents[m]->SetMaterial(s, Info.OriginalMaterials[m][s]);
			}
			FadedActors.RemoveAt(i);
		}
	}
}

void UProjectHOcclusionFadeComponent::SetCharacterSilhouette(bool bEnabled)
{
	if (!bEnableCharacterSilhouette || bEnabled == bWasSilhouetteActive) return;
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;
	ACharacter* Char = Cast<ACharacter>(PC->GetPawn());
	if (!Char) return;
	if (USkeletalMeshComponent* M = Char->GetMesh())
	{
		M->SetRenderCustomDepth(bEnabled);
		if (bEnabled) M->SetCustomDepthStencilValue(SilhouetteStencilValue);
	}
	bWasSilhouetteActive = bEnabled;
}

int32 UProjectHOcclusionFadeComponent::FindFadedActorIndex(AActor* Actor) const
{
	for (int32 i = 0; i < FadedActors.Num(); ++i)
		if (FadedActors[i].Actor.Get() == Actor) return i;
	return INDEX_NONE;
}