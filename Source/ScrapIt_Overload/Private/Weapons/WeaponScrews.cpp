// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponScrews.h"

#include "Interfaces/Damageable.h"
#include "Components/BoxComponent.h"

// Sets default values
AWeaponScrews::AWeaponScrews()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	HitboxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HitboxCollider"));
	HitboxCollider->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponScrews::BeginPlay()
{
	Super::BeginPlay();
	
	HitboxCollider->OnComponentBeginOverlap.AddDynamic(this, &AWeaponScrews::OverlapBegin);
}

void AWeaponScrews::OverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this)
	{
		return;
	}
	
	if (IDamageable* Target = Cast<IDamageable>(OtherActor))
	{
		Target->TakeDamage(BaseDamage);
	}
}

void AWeaponScrews::ApplyUniquePowerUp()
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

void AWeaponScrews::Fire()
{
	//Screws don't fire, empty override
}

