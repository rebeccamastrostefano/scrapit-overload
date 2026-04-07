// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemySocketWitch.h"
#include "NavigationSystem.h"
#include "Core/FunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void AEnemySocketWitch::BeginPlay()
{
	Super::BeginPlay();
}

bool AEnemySocketWitch::AttemptStartShield()
{
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(this);

	const bool bFoundSomething = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		ProtectionRange,
		ObjectTypes,
		AEnemyBase::StaticClass(), // Only look for classes that are AEnemyBase
		Ignore,
		OverlappedActors
	);

	if (bFoundSomething && OverlappedActors.Num() > 0)
	{
		// Pick the first valid enemy found
		PrimaryShieldedAlly = Cast<AEnemyBase>(OverlappedActors[0]);

		if (PrimaryShieldedAlly)
		{
			SetState(EEnemyState::Attacking);

			if (ShieldVfx != nullptr)
			{
				ActiveShieldVfx = UNiagaraFunctionLibrary::SpawnSystemAttached(
					ShieldVfx,
					PrimaryShieldedAlly->GetRootComponent(),
					NAME_None,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true
				);
			}

			UE_LOG(LogTemp, Warning, TEXT("SocketWitch: Shielding %s"), *PrimaryShieldedAlly->GetName());
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: No valid target found for shielding"));
	return false;
}

void AEnemySocketWitch::UpdateShield()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	TArray<FHitResult> Hits;

	if (UKismetSystemLibrary::SphereTraceMulti(GetWorld(), PrimaryShieldedAlly->GetActorLocation(),
	                                           PrimaryShieldedAlly->GetActorLocation(), ShieldRadius,
	                                           UEngineTypes::ConvertToTraceType(ECC_Pawn), false, ActorsToIgnore,
	                                           EDrawDebugTrace::None, Hits, true))
	{
		TArray<AEnemyBase*> CurrentEnemiesInSphere;

		for (FHitResult Hit : Hits)
		{
			if (AEnemyBase* Ally = Cast<AEnemyBase>(Hit.GetActor()))
			{
				CurrentEnemiesInSphere.Add(Ally);
				if (!Ally->GetIsShielded())
				{
					Ally->SetShielded(true);
					ShieldedAllies.AddUnique(Ally);
				}
			}
		}

		for (AEnemyBase* Ally : ShieldedAllies)
		{
			if (!CurrentEnemiesInSphere.Contains(Ally))
			{
				Ally->SetShielded(false);
				ShieldedAllies.Remove(Ally);
			}
		}
	}
}

void AEnemySocketWitch::StopShield()
{
	for (AEnemyBase* Ally : ShieldedAllies)
	{
		if (Ally != nullptr)
		{
			Ally->SetShielded(false);
		}
	}

	ShieldedAllies.Empty();
	PrimaryShieldedAlly = nullptr;
	SetState(EEnemyState::Idle); //Use 'Idle' for not protecting state
	if (ActiveShieldVfx != nullptr)
	{
		ActiveShieldVfx->DestroyComponent();
	}
}

FVector AEnemySocketWitch::GetFleeLocation() const
{
	if (Player == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemySocketWitch: Player is nullptr"));
		return GetActorLocation();
	}

	const FVector DirectionFromPlayer = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
	FVector TargetPoint = GetActorLocation() + (DirectionFromPlayer * ProtectionRange);

	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLocation;

	if (NavSys && NavSys->ProjectPointToNavigation(TargetPoint, NavLocation))
	{
		return NavLocation.Location;
	}

	return GetActorLocation();
}

FVector AEnemySocketWitch::GetLocationNearAlly() const
{
	const AActor* TargetAlly = (PrimaryShieldedAlly != nullptr)
		                           ? Cast<AActor>(PrimaryShieldedAlly)
		                           : UFunctionLibrary::GetClosestEnemy(this, RoomSearchRadius);
	UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: Target ally is %s"),
	       (TargetAlly != nullptr) ? *TargetAlly->GetName() : TEXT("None"))

	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLocation;

	if (TargetAlly != nullptr && Player != nullptr)
	{
		float DistanceToAlly = FVector::Dist(GetActorLocation(), TargetAlly->GetActorLocation());

		if (DistanceToAlly < ProtectionRange * 0.8f)
		{
			//If we are already close enough to the ally, let's stay there
			UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: Already close enough to ally, staying put"))
			return GetActorLocation();
		}

		const FVector DirectionToAlly = (TargetAlly->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		//We don't move to the Ally location, we stay a bit further back
		const float DesiredStopDistance = ProtectionRange * 0.5f;
		const FVector TargetPoint = TargetAlly->GetActorLocation() - (DirectionToAlly * DesiredStopDistance);

		if (NavigationSystem != nullptr && NavigationSystem->ProjectPointToNavigation(TargetPoint, NavLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: Moving to position near %s"), *TargetAlly->GetName())
			return NavLocation.Location;
		}

		//If the point is not reachable, just pick a random point around the ally
		if (NavigationSystem != nullptr && NavigationSystem->GetRandomReachablePointInRadius(
			TargetAlly->GetActorLocation(), ProtectionRange * 0.8f, NavLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: Moving to random point near %s"), *TargetAlly->GetName())
			return NavLocation.Location;
		}

		//If everything fails, just move to the ally
		return TargetAlly->GetActorLocation();
	}

	UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: No Enemy to shield found"))

	if (NavigationSystem && NavigationSystem->GetRandomReachablePointInRadius(
		GetActorLocation(), ProtectionRange, NavLocation))
	{
		return NavLocation.Location;
	}

	return GetActorLocation();
}
