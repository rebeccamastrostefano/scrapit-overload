// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBoltTick.h"

#include "AIController.h"
#include "Mecha/MechaPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AEnemyBoltTick::AEnemyBoltTick()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
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
	
	//Get Mecha Target
	MechaTarget = GetWorld()->GetFirstPlayerController()->GetPawn();
	
	//Get the AIController, if null, create one and possess the enemy
	AIController = Cast<AAIController>(GetController());
	if (AIController == nullptr)
	{
		AIController = GetWorld()->SpawnActor<AAIController>(AIControllerClass);
		AIController->Possess(this);
	}
	
	HurtboxSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBoltTick::OnHurtboxOverlap);
	
	//Start Navigation Update
	GetWorldTimerManager().SetTimer(NavigationTimer, this, &AEnemyBoltTick::UpdateNavigation, NavigationUpdateRate, true);
}

// Called every frame
void AEnemyBoltTick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MechaTarget == nullptr || CurrentHealth <= 0)
	{
		return;
	}
	

	UpdateRotation(DeltaTime);
	
	if (CurrentState == EState::Chasing)
	{
		const float Distance = FVector::Dist(GetActorLocation(), MechaTarget->GetActorLocation());
		if (Distance <= AttackRange)
		{
			StartAttack();
		}
	}
	else if (CurrentState == EState::Cooldown)
	{
		const FVector AwayDirection = (GetActorLocation() - MechaTarget->GetActorLocation()).GetSafeNormal();
		AddMovementInput(AwayDirection, 0.6f);
	}
}

void AEnemyBoltTick::UpdateRotation(const float DeltaTime)
{
	//If we are not moving, rotate towards Mecha
	FVector Direction = (MechaTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, 5.f));
}

void AEnemyBoltTick::UpdateNavigation() const
{
	if (CurrentState == EState::Chasing && MechaTarget != nullptr && AIController != nullptr)
	{
		const float Distance = FVector::Dist(GetActorLocation(), MechaTarget->GetActorLocation());
	
		if (Distance > AttackRange)
		{
			AIController->MoveToActor(MechaTarget, 50.0f);
		}
	}
}

void AEnemyBoltTick::StartAttack()
{
	CurrentState = EState::Attacking;
	
	if (AIController != nullptr)
	{
		AIController->StopMovement();
	}
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
	
	if (AIController != nullptr)
	{
		AIController->StopMovement();
	}
	
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