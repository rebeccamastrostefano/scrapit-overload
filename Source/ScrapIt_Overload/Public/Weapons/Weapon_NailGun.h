// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Weapons/WeaponBase.h"
#include "Weapon_NailGun.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API AWeapon_NailGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AWeapon_NailGun();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Settings")
	TSubclassOf<class AProjectile> ProjectileBP;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	float ProjectileSpeed = 1500.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* FirePoint;
	
	AActor* CurrentTarget = nullptr;
	
	virtual void Fire() override;
	
	virtual void ApplyUniquePowerUp() override;
	
	void TrackTarget();
};
