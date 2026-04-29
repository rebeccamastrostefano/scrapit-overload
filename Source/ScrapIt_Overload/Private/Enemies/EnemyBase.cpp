// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Engine/OverlapResult.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Check if we are currently under the effect of a shield
	if (bIsShielded)
	{
		TimeSinceLastPulse += DeltaTime;

		if (TimeSinceLastPulse <= ShieldPulseTimeout)
		{
			TimeInShield += DeltaTime;

			//If we have stayed in the shield long enough, we activate shielding
			if (TimeInShield >= ShieldWarmupTime)
			{
				//Ramp up the shield
				CurrentShieldReduction = FMath::Clamp(CurrentShieldReduction + (DeltaTime * ShieldRampUpSpeed), 0.f,
				                                      MaxShieldReduction);

				//Spawn VFX if not already spawned
				if (ActiveShieldedVfx == nullptr && ShieldedVfx != nullptr)
				{
					ActiveShieldedVfx = UNiagaraFunctionLibrary::SpawnSystemAttached(
						ShieldedVfx,
						RootComponent,
						NAME_None,
						FVector::ZeroVector,
						FRotator::ZeroRotator,
						EAttachLocation::SnapToTarget,
						true
					);
				}

				//Ramp up intensity of VFX
				if (ActiveShieldedVfx != nullptr)
				{
					ActiveShieldedVfx->SetFloatParameter("Intensity", CurrentShieldReduction);
				}
			}
		}
		else
		{
			//If we stepped out of shield before warmup finished, abort
			if (TimeInShield < ShieldWarmupTime)
			{
				bIsShielded = false;
				TimeInShield = 0.f;
				return;
			}

			//Shield is already ongoing, but we are outside the shield area. Check if Grace period is expired, start shield decay (slowly bring the shield to 0)
			if (TimeSinceLastPulse > ShieldGracePeriod)
			{
				CurrentShieldReduction = FMath::Clamp(CurrentShieldReduction - (DeltaTime * ShieldDecaySpeed), 0.f,
				                                      MaxShieldReduction);

				//Reduce intensity of VFX
				if (ActiveShieldedVfx != nullptr)
				{
					ActiveShieldedVfx->SetFloatParameter("Intensity", CurrentShieldReduction);
				}

				//When shield is depleted, remove shield
				if (CurrentShieldReduction <= 0.f)
				{
					bIsShielded = false;
					TimeInShield = 0.f;

					//Destroy VFX
					if (ActiveShieldedVfx != nullptr)
					{
						ActiveShieldedVfx->Deactivate();
						ActiveShieldedVfx->DestroyComponent();
						ActiveShieldedVfx = nullptr;
					}
				}
			}
		}
	}
}

void AEnemyBase::ReceiveDamage(const float DamageAmount)
{
	if (CurrentState == EEnemyState::Dead)
	{
		return;
	}

	//Apply shield reduction if any
	const float FinalDamage = DamageAmount * (1.f - CurrentShieldReduction);

	VitalityComponent->ApplyDamage(FinalDamage);
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

bool AEnemyBase::IsFacingPlayer() const
{
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

	if (GetWorld()->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
	                                         FCollisionObjectQueryParams(ECC_WorldDynamic),
	                                         FCollisionShape::MakeSphere(DamageTraceRadius), Params))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UDamageable>() && !Overlap.GetActor()->GetClass()->IsChildOf(
				AEnemyBase::StaticClass()))
			{
				Cast<IDamageable>(Overlap.GetActor())->ReceiveDamage(Damage);
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

void AEnemyBase::ReceiveShieldPulse(const float MaxReduction, const float RampUpSpeed, const float GracePeriod,
                                    const float DecaySpeed, const float PulseTimeout)
{
	bIsShielded = true;
	TimeSinceLastPulse = 0.f;

	MaxShieldReduction = MaxReduction;
	ShieldRampUpSpeed = RampUpSpeed;
	ShieldGracePeriod = GracePeriod;
	ShieldDecaySpeed = DecaySpeed;
	ShieldPulseTimeout = PulseTimeout;
}
