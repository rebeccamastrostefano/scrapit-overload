// Fill out your copyright notice in the Description page of Project Settings.


#include "ScrapItGameMode.h"
#include "MechaPawn.h"
#include "Core/WeaponSubsystem.h"
#include "Scraps/ScrapSubsystem.h"

AScrapItGameMode::AScrapItGameMode()
{
	DefaultPawnClass = AMechaPawn::StaticClass();
}

void AScrapItGameMode::BeginPlay()
{
	Super::BeginPlay();
	UWeaponSubsystem* WeaponSS = GetGameInstance()->GetSubsystem<UWeaponSubsystem>();
	UScrapSubsystem* ScrapSS = GetWorld()->GetSubsystem<UScrapSubsystem>();
	
	if (WeaponSS && WeaponsDatabase)
	{
		WeaponSS->WeaponsDB = WeaponsDatabase;
	}
	
	if (ScrapSS && BasicScrapBP)
	{
		ScrapSS->BasicScrapBlueprint = BasicScrapBP;
	}
}
