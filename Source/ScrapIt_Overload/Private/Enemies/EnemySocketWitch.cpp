// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemySocketWitch.h"
#include "NavigationSystem.h"
#include "Core/FunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AEnemySocketWitch::AEnemySocketWitch()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	HurtboxSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HurtboxSphere"));
	HurtboxSphere->SetupAttachment(RootComponent);
}

void AEnemySocketWitch::BeginPlay()
{
	Super::BeginPlay();
}

bool AEnemySocketWitch::AttemptStartShield()
{
	PerformShieldOverlap(GetActorLocation(), ProtectionRange);

	//Filter out enemies that are already shielded
	OverlappedActorsInShieldCheck.RemoveAll([](AActor* OverlappedEnemy)
	{
		const AEnemyBase* Enemy = Cast<AEnemyBase>(OverlappedEnemy);
		return Enemy != nullptr && (Enemy->GetIsShielded());
	});

	if (OverlappedActorsInShieldCheck.Num() > 0)
	{
		// Pick the first valid enemy found
		PrimaryShieldedAlly = Cast<AEnemyBase>(OverlappedActorsInShieldCheck[0]);

		if (PrimaryShieldedAlly)
		{
			//Start Shield
			SetState(EEnemyState::Attacking);
			UpdateShield();

			//Spawn VFX
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
	if (PrimaryShieldedAlly == nullptr)
	{
		return;
	}

	PerformShieldOverlap(PrimaryShieldedAlly->GetActorLocation(), ShieldRadius);

	//Set the allies in the shield to be shielded
	for (AActor* Enemy : OverlappedActorsInShieldCheck)
	{
		if (AEnemyBase* Ally = Cast<AEnemyBase>(Enemy))
		{
			if (!Ally->GetIsShielded())
			{
				Ally->SetShielded(true);
				ShieldedAllies.AddUnique(Ally);
			}
		}
	}

	//Remove all allies that left the shield radius
	ShieldedAllies.RemoveAll([this](AEnemyBase* Ally)
	{
		if (!Ally || !OverlappedActorsInShieldCheck.Contains(Ally))
		{
			if (Ally) Ally->SetShielded(false);
			return true;
		}
		return false;
	});
}

void AEnemySocketWitch::StopShield()
{
	UE_LOG(LogTemp, Warning, TEXT("EnemySocketWitch: Stopping Shield..."));

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
		ActiveShieldVfx = nullptr;
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

	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLocation;

	if (TargetAlly != nullptr && Player != nullptr)
	{
		float DistanceToAlly = FVector::Dist(GetActorLocation(), TargetAlly->GetActorLocation());

		if (DistanceToAlly < ProtectionRange * 0.8f)
		{
			//If we are already close enough to the ally, let's stay there
			return GetActorLocation();
		}

		const FVector DirectionToAlly = (TargetAlly->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		//We don't move to the Ally location, we stay a bit further back
		const float DesiredStopDistance = ProtectionRange * 0.5f;
		const FVector TargetPoint = TargetAlly->GetActorLocation() - (DirectionToAlly * DesiredStopDistance);

		if (NavigationSystem != nullptr && NavigationSystem->ProjectPointToNavigation(TargetPoint, NavLocation))
		{
			return NavLocation.Location;
		}

		//If the point is not reachable, just pick a random point around the ally
		if (NavigationSystem != nullptr && NavigationSystem->GetRandomReachablePointInRadius(
			TargetAlly->GetActorLocation(), ProtectionRange * 0.8f, NavLocation))
		{
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

void AEnemySocketWitch::PerformShieldOverlap(const FVector& Location, const float Radius)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	OverlappedActorsInShieldCheck.Reset();

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Location, Radius, ObjectTypes,
		AEnemyBase::StaticClass(), ActorsToIgnore, OverlappedActorsInShieldCheck
	);

	// Filter out other witches
	OverlappedActorsInShieldCheck.RemoveAll([](const AActor* PotentialAlly)
	{
		return PotentialAlly->IsA<AEnemySocketWitch>();
	});
	OverlappedActorsInShieldCheck.Reset();
}

void AEnemySocketWitch::Die()
{
	StopShield();
	Super::Die();
}
