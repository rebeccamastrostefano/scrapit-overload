// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/Enemy_BoltTick.h"

#include "MechaPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AEnemy_BoltTick::AEnemy_BoltTick()
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
void AEnemy_BoltTick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = BaseHealth;
	MechaTarget = GetWorld()->GetFirstPlayerController()->GetPawn();
	HurtboxSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy_BoltTick::OnHurtboxOverlap);
}

// Called every frame
void AEnemy_BoltTick::Tick(float DeltaTime)
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
			case EState::ES_Chasing:
				//If the enemy is in the attack range, start attack
				if (DistanceToMecha <= AttackRange)
				{
					StartAttack();
					return;
				}
			
				AddMovementInput(Direction, 1.0f);
				break;
			case EState::ES_Cooldown:
				//Enemy backs up to attack again
				if (DistanceToMecha <= AttackRange)
				{
					AddMovementInput(Direction, -1.0f);
				}
				break;
			case EState::ES_Attacking:
				break;
			case EState::ES_Hurt:
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

void AEnemy_BoltTick::StartAttack()
{
	CurrentState = EState::ES_Attacking;
	MovementComp->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy_BoltTick::ExecuteAttack, 0.2f, false);
}

void AEnemy_BoltTick::ExecuteAttack()
{
	FVector const AttackDirection = (MechaTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	MovementComp->Velocity = AttackDirection * AttackForce;
	
	GetWorldTimerManager().SetTimer(AttackTimer, [this]()
	{
		CurrentState = EState::ES_Cooldown;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy_BoltTick::ResetMovement, AttackCooldown, false);
	}, 0.2f, false);
}

void AEnemy_BoltTick::ResetMovement()
{
	CurrentState = EState::ES_Chasing;
}

void AEnemy_BoltTick::OnHurtboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this && !OtherActor->Implements<UEnemy>())
	{
		IDamageable* const MechaDamageable = Cast<IDamageable>(OtherActor);
		if (MechaDamageable && CurrentState == EState::ES_Attacking)
		{
			MechaDamageable->TakeDamage(Damage);
		}
	}
}

void AEnemy_BoltTick::TakeDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	
	if (CurrentHealth <= 0)
	{
		Die();
	}
	else
	{
		//knockback slightly
		FVector const KnockbackDirection = (GetActorLocation() - MechaTarget->GetActorLocation()).GetSafeNormal();
		CurrentState = EState::ES_Hurt;
		MovementComp->StopMovementImmediately();
		GetWorldTimerManager().SetTimer(AttackTimer, [this, KnockbackDirection]() 
			{ 
				MovementComp->Velocity = KnockbackDirection * KnockbackForce;
				GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy_BoltTick::ResetMovement, AttackCooldown, false);
			}, 0.1f, false);
		
	}
}

void AEnemy_BoltTick::Die()
{
	OnDeath.ExecuteIfBound(GetActorLocation(), ScrapDrop);
	Destroy();
}

void AEnemy_BoltTick::RegisterToRoomManager(ARoomManager* RoomManager)
{
	OnDeath.BindUObject(RoomManager, &ARoomManager::OnEnemyDeath);
	RoomManager->OnRoomCompleted.AddDynamic(this, &AEnemy_BoltTick::Die);
}

