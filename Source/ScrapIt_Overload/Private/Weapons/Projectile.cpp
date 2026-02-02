// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Interfaces/Damageable.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement Component"));
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->InitialSpeed = 1500.f;
	MovementComp->MaxSpeed = 1500.f;
	MovementComp->ProjectileGravityScale = 0.f;
	
	InitialLifeSpan = 3.f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);
}

void AProjectile::InitializeProjectile(float WeaponDamage, float Speed)
{
	Damage = WeaponDamage;
	MovementComp->InitialSpeed = Speed;
	MovementComp->MaxSpeed = Speed;
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner())
	{
			if (IDamageable* Target = Cast<IDamageable>(OtherActor))
			{
				Target->TakeDamage(Damage);
			}
	}
	Destroy();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

