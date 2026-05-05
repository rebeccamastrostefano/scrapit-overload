// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponNailGun.h"

AWeaponNailGun::AWeaponNailGun()
{
	NailGunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NailGunMesh"));
	NailGunMesh->SetupAttachment(RootComponent);
	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(RootComponent);
}

void AWeaponNailGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTarget = FindNearestEnemy();
	TrackEnemy(DeltaTime, NailGunMesh);
}

void AWeaponNailGun::Fire()
{
	if (CurrentTarget && ProjectileBP)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = GetInstigator();
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		//Override spawn rotation to point at the enemy, ensures correct aim
		const FVector Direction = (CurrentTarget->GetActorLocation() - FirePoint->GetComponentLocation()).
			GetSafeNormal();
		FTransform SpawnTransform = FirePoint->GetComponentTransform();
		SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

		if (AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileBP, SpawnTransform, SpawnParameters))
		{
			NewProjectile->InitializeProjectile(BaseDamage, ProjectileSpeed);
			OnHitEnemy.Broadcast();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn projectile!"));
		}
	}
}

void AWeaponNailGun::ApplyUniquePowerUp()
{
	switch (CurrentWeaponLevel)
	{
	case 1:
		break;
	default:
		break;
		//TODO: powerups
	}
}
