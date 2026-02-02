// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon_NailGun.h"

AWeapon_NailGun::AWeapon_NailGun()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;
	
	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(RootComponent);
}

void AWeapon_NailGun::Fire()
{
	AActor* Target = FindNearestEnemy();
	if (Target && ProjectileBP)
	{
		TrackTarget(Target);
		
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

void AWeapon_NailGun::TrackTarget(AActor* Target)
{
	if (!Target)
	{
		return;
	}
	
	FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorRotation(Direction.Rotation());
}
