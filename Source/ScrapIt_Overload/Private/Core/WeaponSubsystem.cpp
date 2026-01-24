// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WeaponSubsystem.h"
#include "Core/WeaponsDatabase.h"

void UWeaponSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UWeaponSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

TSubclassOf<AActor> UWeaponSubsystem::GetWeaponBlueprint(const EScrapType Type)
{
	if (!WeaponsDB)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Database"))
	}
	else if (WeaponsDB->WeaponLibrary.Contains(Type))
	{
		return WeaponsDB->WeaponLibrary[Type];
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Weapon not found in DB"));
	return nullptr;
}
