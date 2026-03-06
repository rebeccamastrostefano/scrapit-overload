// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/Objectives/RoomObjective.h"
#include "CollectObjective.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UCollectObjective : public URoomObjective
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnCollected(FWeaponData WeaponData);

public:
	void ActivateObjective(ARoomManager* RoomManager) override;
};
