// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "Core/ScrapItGameInstance.h"
#include "Interfaces/Enemy.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = MeshComponent;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (FireRate != 0)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AWeaponBase::Fire, FireRate, true);
	}
}

AActor* AWeaponBase::FindNearestEnemy() const
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UEnemy::StaticClass(), FoundEnemies);
	
	AActor* NearestEnemy = nullptr;
	float MinDistSquared = FMath::Square(Range);
	
	const FVector Location = GetActorLocation();
	const FVector ForwardDirection = GetActorForwardVector();
	
	for (AActor* Enemy : FoundEnemies)
	{
		if (Enemy == nullptr) continue;
		
		FVector EnemyLocation = Enemy->GetActorLocation();
		FVector DirectionToEnemy = (EnemyLocation - Location).GetSafeNormal();
		
		//Check if the enemy is in the "Fire Cone" of the weapon
		float const Angle = FVector::DotProduct(ForwardDirection, DirectionToEnemy);
		if (Angle >= FireConeThreshold)
		{
			//Check if the enemy is within range
			float DistSquared = FVector::DistSquared(Location, EnemyLocation);
			if (DistSquared < MinDistSquared)
			{
				MinDistSquared = DistSquared;
				NearestEnemy = Enemy;
			}
		}
	}
	return NearestEnemy;
}

bool AWeaponBase::IsWeaponUpgrading(const int32 TierNumber)
{
	//If the new tier is higher than the current weapon level, upgrade the weapon
	if (TierNumber > CurrentWeaponLevel)
	{
		SetWeaponLevel(TierNumber);
		return true;
	}
	else
	{
		return false;
	}
}

void AWeaponBase::SetWeaponLevel(const int32 NewLevel)
{
	if (const UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance()))
	{
		if (GI->WeaponLevels.Contains(ScrapType))
		{
			UWeaponLevels* WeaponLevels = GI->WeaponLevels[ScrapType];
			if (WeaponLevels->Levels.Contains(NewLevel))
			{
				FWeaponLevelDefinition& Level = WeaponLevels->Levels[NewLevel];
				
				CurrentWeaponLevel = NewLevel;
				
				MeshComponent->SetStaticMesh(Level.WeaponMesh);
				Damage *= Level.DamageMultiplier;
				Range *= Level.RangeMultiplier;
				FireRate *= Level.FireRateMultiplier;
				
				ApplyUniquePowerUp();
			}
		}
	}
	
}

