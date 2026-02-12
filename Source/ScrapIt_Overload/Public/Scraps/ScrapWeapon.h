// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scraps/ScrapBase.h"
#include "ScrapWeapon.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AScrapWeapon : public AScrapBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "WeaponData")
	int32 WeaponLevel = 0;
	
	virtual void OnCollected() override;
	
public:
	void InitWeaponData(EScrapType WeaponScrapType, const int32 LevelNumber);
	
	void SetWeaponLevel(const int32 Level)
	{
		WeaponLevel = Level;
	}
	
	int32 GetWeaponLevel() const
	{
		return WeaponLevel;
	}
};
