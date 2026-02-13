// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBoltTick.h"

#include "Mecha/MechaPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AEnemyBoltTick::AEnemyBoltTick()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoltTickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoltTickMesh"));
	RootComponent = BoltTickMesh;
	
	HurtboxSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HurtboxSphere"));
	HurtboxSphere->SetupAttachment(RootComponent);
	
	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
	MovementComp->MaxSpeed = MoveSpeed;
	MovementComp->Acceleration = 4000.0f;
	MovementComp->Deceleration = 2000.0f;
}

// Called when the game starts or when spawned
void AEnemyBoltTick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = BaseHealth;
	CurrentState = EState::Chasing;
	MechaTarget = GetWorld()->GetFirstPlayerController()->GetPawn();
	HurtboxSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBoltTick::OnHurtboxOverlap);
}

// Called every frame
void AEnemyBoltTick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MechaTarget)
	{
		//Get the direction between the enemy and the mecha and apply rotation so it's looking at us
		FVector const Direction = (MechaTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		SetActorRotation(Direction.Rotation());
		
		float const DistanceToMecha = FVector::Distance(GetActorLocation(), MechaTarget->GetActorLocation());

		switch (CurrentState)
		{
			case EState::Chasing:
				//If the enemy is in the attack range, start attack
				if (DistanceToMecha <= AttackRange)
				{
					StartAttack();
					return;
				}
			
				//Make enemy move towards Mecha
				AddMovementInput(Direction, 1.0f);
				break;
			case EState::Cooldown:
				//Enemy backs up to attack again
				if (DistanceToMecha <= AttackRange)
				{
					AddMovementInput(Direction, -1.0f);
				}
				break;
			case EState::Attacking:
				break;
			case EState::Hurt:
				break;
			default: 
				break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NO PLAYER"))
	}
}

void AEnemyBoltTick::StartAttack()
{
	CurrentState = EState::Attacking;
	MovementComp->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyBoltTick::ExecuteAttack, 0.2f, false);
}

void AEnemyBoltTick::ExecuteAttack()
{
	FVector const AttackDirection = (MechaTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	MovementComp->Velocity = AttackDirection * AttackForce;
	
	GetWorldTimerManager().SetTimer(AttackTimer, [this]()
	{
		CurrentState = EState::Cooldown;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyBoltTick::ResetMovement, AttackCooldown, false);
	}, 0.2f, false);
}

void AEnemyBoltTick::ResetMovement()
{
	CurrentState = EState::Chasing;
}

void AEnemyBoltTick::OnHurtboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this && !OtherActor->GetClass()->IsChildOf(AEnemyBase::StaticClass()))
	{
		IDamageable* const MechaDamageable = Cast<IDamageable>(OtherActor);
		if (MechaDamageable && CurrentState == EState::Attacking)
		{
			MechaDamageable->TakeDamage(Damage);
		}
	}
}

void AEnemyBoltTick::TakeDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	
	//knockback slightly
	FVector const KnockbackDirection = (GetActorLocation() - MechaTarget->GetActorLocation()).GetSafeNormal();
	CurrentState = EState::Hurt;
	MovementComp->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(AttackTimer, [this, KnockbackDirection]() 
	{ 
		MovementComp->Velocity = KnockbackDirection * KnockbackForce;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyBoltTick::ResetMovement, AttackCooldown, false);
	}, 0.1f, false);
	
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

