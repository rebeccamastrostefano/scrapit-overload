// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Weapons/WeaponBase.h"
#include "WeaponNailGun.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API AWeaponNailGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AWeaponNailGun();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Settings")
	TSubclassOf<class AProjectile> ProjectileBP;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float ProjectileSpeed = 1500.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* FirePoint;
	
	virtual void Fire() override;
	
	virtual void ApplyUniquePowerUp() override;
	
public:
	virtual void Tick(float DeltaTime) override;
};
