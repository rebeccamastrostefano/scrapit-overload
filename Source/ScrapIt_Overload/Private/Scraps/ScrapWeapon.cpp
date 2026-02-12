// Fill out your copyright notice in the Description page of Project Settings.

#include "Mecha/MechaPawn.h"
#include "Core/ScrapItGameInstance.h"
#include "Scraps/ScrapWeapon.h"

void AScrapWeapon::InitWeaponData(const EScrapType WeaponScrapType, const int32 LevelNumber)
{
	WeaponLevel = LevelNumber;
	ScrapType = WeaponScrapType;
	
	const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScrapWeapon: GameInstance is NOT UScrapItGameInstance!"));
		return;
	}
	
	if (const UWeaponLevels* LevelsPool = GameInstance->WeaponLevels.FindRef(ScrapType))
	{
		if (const FWeaponLevelDefinition* WeaponData = LevelsPool->Levels.Find(LevelNumber))
		{
			ScrapMesh->SetStaticMesh(WeaponData->WeaponMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ScrapWeapon: No WeaponLevel data for %s level %d!"), *UEnum::GetValueAsString(WeaponScrapType), LevelNumber);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ScrapWeapon: No WeaponLevels data for %s!"), *UEnum::GetValueAsString(WeaponScrapType));
	}
}

void AScrapWeapon::OnCollected()
{
	//Equip it to the mecha
	if (const AMechaPawn* Mecha = Cast<AMechaPawn>(PullingActor))
	{
		Mecha->WeaponSystem->NotifyWeaponAcquired(ScrapType, WeaponLevel);
	}
	Destroy();
}