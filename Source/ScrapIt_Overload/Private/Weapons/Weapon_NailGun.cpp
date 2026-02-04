// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon_NailGun.h"

AWeapon_NailGun::AWeapon_NailGun()
{
	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(RootComponent);
}

void AWeapon_NailGun::Fire()
{
	CurrentTarget = FindNearestEnemy();
	TrackTarget();
	if (CurrentTarget && ProjectileBP)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = GetInstigator();
		
		AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileBP, FirePoint->GetComponentTransform(), SpawnParameters);
		
		if (NewProjectile)
		{
			NewProjectile->InitializeProjectile(Damage, ProjectileSpeed);
		}
	}
}

void AWeapon_NailGun::ApplyUniquePowerUp()
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

void AWeapon_NailGun::TrackTarget()
{
	if (!CurrentTarget)
	{
		return;
	}
	
	FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorRotation(Direction.Rotation());
}
