// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Engine/OverlapResult.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VitalityComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Vitality Component"));
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	VitalityComponent->InitializeHealth(BaseHealth);
	VitalityComponent->OnDeath.AddDynamic(this, &AEnemyBase::Die);
	
	//Get the AIController, if null, create one and possess the enemy
	AIController = Cast<AAIController>(GetController());
	if (AIController == nullptr)
	{
		AIController = GetWorld()->SpawnActor<AAIController>(AIControllerClass);
		AIController->Possess(this);
	}
	
	AIController->RunBehaviorTree(BehaviorTree);
	
	Player = GetWorld()->GetFirstPlayerController()->GetPawn();
}

void AEnemyBase::TakeDamage(const float DamageAmount)
{
	if (CurrentState == EEnemyState::Dead)
	{
		return;
	}
	
	VitalityComponent->ApplyDamage(DamageAmount);
	SetState(EEnemyState::Hurt);
}

void AEnemyBase::SetState(const EEnemyState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}
	
	const EEnemyState OldState = CurrentState;
	CurrentState = NewState;
	
	OnStateChanged.Broadcast(OldState, CurrentState);
}

bool AEnemyBase::IsFacingPlayer() const {
	if (Player == nullptr)
	{
		return false;
	}
	
	const FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	return FVector::DotProduct(GetActorForwardVector(), ToPlayer) > AttackDirectionThreshold;
}

void AEnemyBase::TriggerDamageTrace()
{
	const FVector Center = GetActorLocation() + (GetActorForwardVector() * DamageTraceDistance);
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	if (GetWorld()->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(DamageTraceRadius), Params))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UDamageable>() && !Overlap.GetActor()->GetClass()->IsChildOf(AEnemyBase::StaticClass()))
			{
				Cast<IDamageable>(Overlap.GetActor())->TakeDamage(Damage);
			}
		}
	}
}

void AEnemyBase::Die()
{
	SetState(EEnemyState::Dead);
	OnDeath.Broadcast(GetActorLocation(), BaseDropAmount);
	Destroy();
}

