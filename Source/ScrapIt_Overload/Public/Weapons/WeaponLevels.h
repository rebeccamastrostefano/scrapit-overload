// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponLevels.generated.h"

USTRUCT(BlueprintType)
struct FWeaponLevelDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Level Definition")
	UStaticMesh* WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Level Definition")
	float DamageMultiplier = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Level Definition")
	float FireRateMultiplier = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Level Definition")
	float RangeMultiplier = 1.f;
};

UCLASS()
class SCRAPIT_OVERLOAD_API UWeaponLevels : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TMap<int32, FWeaponLevelDefinition> Levels;
};
