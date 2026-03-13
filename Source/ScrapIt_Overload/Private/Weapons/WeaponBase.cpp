// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "Core/FunctionLibrary.h"
#include "Core/ScrapItGameInstance.h"

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
	AActor* PotentialTarget = UFunctionLibrary::GetClosestEnemy(this, BaseRange);

	if (PotentialTarget && IsInsideFireCone(PotentialTarget))
	{
		return PotentialTarget;
	}

	return nullptr;
}

bool AWeaponBase::IsInsideFireCone(const AActor* Target) const
{
	if (Target == nullptr)
	{
		return false;
	}

	const FVector Forward = GetSocketRotation().Vector();
	const FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	return FVector::DotProduct(Forward, ToTarget) >= FireConeThreshold;
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
	check(GameInstance != nullptr);

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
