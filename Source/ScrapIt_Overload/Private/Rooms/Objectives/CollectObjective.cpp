// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Objectives/CollectObjective.h"

#include "Kismet/GameplayStatics.h"
#include "Mecha/MechaPawn.h"

void UCollectObjective::ActivateObjective(ARoomManager* RoomManager)
{
	AMechaPawn* Player = Cast<AMechaPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (Player != nullptr)
	{
		Player->WeaponSystem->OnWeaponEquipped.AddDynamic(this, &UCollectObjective::OnCollected);
	}
}

void UCollectObjective::OnCollected(FWeaponData WeaponData)
{
	OnObjectiveFinished.Broadcast();
}
