// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Objectives/CombatObjective.h"

#include "Rooms/RoomManager.h"

void UCombatObjective::ActivateObjective(ARoomManager* RoomManager)
{
	//Register to Enemy Spawner Events
	RoomManager->EnemySpawner->OnEnemiesCleared.AddDynamic(this, &UCombatObjective::OnClearedEnemies);
}

void UCombatObjective::OnClearedEnemies()
{
	OnObjectiveFinished.Broadcast();
}
