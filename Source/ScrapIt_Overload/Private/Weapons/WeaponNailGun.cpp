// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponNailGun.h"

AWeaponNailGun::AWeaponNailGun()
{
	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(RootComponent);
}

void AWeaponNailGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CurrentTarget = FindNearestEnemy();
	TrackEnemy(DeltaTime);
}

void AWeaponNailGun::Fire()
{
	if (CurrentTarget && ProjectileBP)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = GetInstigator();
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		if (AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileBP, FirePoint->GetComponentTransform(), SpawnParameters))
		{
			NewProjectile->InitializeProjectile(BaseDamage, ProjectileSpeed);
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
