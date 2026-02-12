// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scraps/ScrapGeneric.h"
#include "WeaponBase.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	EScrapType ScrapType;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float BaseDamage = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float BaseFireRate = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float BaseRange = 800.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float FireConeThreshold = 0.7f;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Settings")
	float RotationSpeed = 5.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon State")
	int32 CurrentWeaponLevel = 0;
	
	UPROPERTY()
	AActor* CurrentTarget;
	
	float Damage;
	float FireRate;
	float Range;
	
	FTimerHandle FireTimer;
	
	AActor* FindNearestEnemy() const;
	void TrackEnemy(const float DeltaTime);
	FRotator GetSocketRotation() const;
	
	void SetLevel(const int32 NewLevel);
	void UpdateFireTimer();
	
	virtual void ApplyUniquePowerUp() PURE_VIRTUAL(AWeaponBase::ApplyUniquePowerUp)
	virtual void Fire() PURE_VIRTUAL(AWeaponBase::Fire);

public:	
	bool TryUpgrade(const int32 TierNumber);
};
