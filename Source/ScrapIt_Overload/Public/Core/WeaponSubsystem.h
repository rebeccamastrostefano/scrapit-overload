// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScrapActor.h"
#include "WeaponsDatabase.h"
#include "WeaponSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UWeaponSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	UWeaponsDatabase* WeaponsDB;
	
	TSubclassOf<AActor> GetWeaponBlueprint(EScrapType Type);
};
