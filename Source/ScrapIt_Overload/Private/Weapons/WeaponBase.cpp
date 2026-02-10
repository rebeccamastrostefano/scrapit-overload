// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "Core/ScrapItGameInstance.h"
#include "Engine/OverlapResult.h"
#include "Interfaces/Enemy.h"

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
	
	Damage = BaseDamage;
	FireRate = BaseFireRate;
	Range = BaseRange;
	
	if (FireRate > 0)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AWeaponBase::Fire, FireRate, true);
	}
}

AActor* AWeaponBase::FindNearestEnemy() const
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Scope = FCollisionShape::MakeSphere(BaseRange);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	//Get all pawns in range
	if (!GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, Scope, Params))
	{
		return nullptr;
	}
	
	AActor* NearestEnemy = nullptr;
	float MinDistSquared = FMath::Square(BaseRange);
	const FVector ActorLocation = GetActorLocation();
	const FVector Forward = GetSocketRotation().Vector();
	
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* const OtherActor = Result.GetActor();
		
		//Check for interface
		if (OtherActor == nullptr || !OtherActor->Implements<UEnemy>())
		{
			continue;
		}
		
		const FVector EnemyLocation = OtherActor->GetActorLocation();
		const FVector ToEnemy = (EnemyLocation - ActorLocation).GetSafeNormal();
		
		//Check if enemy is in fire cone
		if (FVector::DotProduct(Forward, ToEnemy) < FireConeThreshold)
		{
			continue;
		}
		
		//Check if in range
		const float DistSquared = FVector::DistSquared(EnemyLocation, ActorLocation);
		if (DistSquared < MinDistSquared)
		{
			MinDistSquared = DistSquared;
			NearestEnemy = OtherActor;
		}
	}
	return NearestEnemy;
}

void AWeaponBase::TrackEnemy(const float DeltaTime)
{
	FRotator TargetRotation;
	if (CurrentTarget)
	{
		const FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		TargetRotation = DirectionToTarget.Rotation();
	}
	else
	{
		TargetRotation = GetSocketRotation();
	}
	
	const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);
}

FRotator AWeaponBase::GetSocketRotation() const
{
	if (const USceneComponent* ParentSocket = GetRootComponent()->GetAttachParent())
	{
		return ParentSocket->GetComponentRotation();
	}
	return GetActorRotation();
}

bool AWeaponBase::TryUpgrade(const int32 TierNumber)
{
	//If the new tier is higher than the current weapon level, upgrade the weapon
	if (TierNumber > CurrentWeaponLevel)
	{
		SetLevel(TierNumber);
		return true;
	}

	return false;
}

void AWeaponBase::SetLevel(const int32 NewLevel)
{
	const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		return;
	}
	
	UWeaponLevels* WeaponLevels = GameInstance->WeaponLevels.FindRef(ScrapType);
	if (WeaponLevels == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: No Data Asset found for ScrapType %d!"), *GetName(), ScrapType)
		return;
	}

	if (!WeaponLevels->Levels.Contains(NewLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: No Weapon Level Definition found for Level %d!"), *GetName(), NewLevel)
		return;
	}
	
	const FWeaponLevelDefinition& LevelDef = WeaponLevels->Levels[NewLevel];
	
	MeshComponent->SetStaticMesh(LevelDef.WeaponMesh);
	Damage = BaseDamage * LevelDef.DamageMultiplier;
	FireRate = BaseFireRate * LevelDef.FireRateMultiplier;
	Range = BaseRange * LevelDef.RangeMultiplier;
	
	CurrentWeaponLevel = NewLevel;
	
	UpdateFireTimer();
	ApplyUniquePowerUp();
}

void AWeaponBase::UpdateFireTimer()
{
	GetWorldTimerManager().ClearTimer(FireTimer);
	if (FireRate > 0)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AWeaponBase::Fire, FireRate, true);
	}
}

