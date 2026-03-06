// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/Objectives/RoomObjective.h"
#include "CombatObjective.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UCombatObjective : public URoomObjective
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnClearedEnemies();

public:
	void ActivateObjective(ARoomManager* RoomManager) override;
};
