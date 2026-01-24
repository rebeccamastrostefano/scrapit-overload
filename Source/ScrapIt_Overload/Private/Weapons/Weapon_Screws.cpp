// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon_Screws.h"

#include "Interfaces/Damageable.h"
#include "Components/BoxComponent.h"

// Sets default values
AWeapon_Screws::AWeapon_Screws()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	
	HitboxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HitboxCollider"));
	HitboxCollider->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon_Screws::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentDamage = BaseDamage;
	HitboxCollider->OnComponentBeginOverlap.AddDynamic(this, &AWeapon_Screws::OverlapBegin);
}

// Called every frame
void AWeapon_Screws::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon_Screws::OverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this)
	{
		return;
	}
	
	if (IDamageable* Target = Cast<IDamageable>(OtherActor))
	{
		Target->TakeDamage(CurrentDamage);
	}
}

