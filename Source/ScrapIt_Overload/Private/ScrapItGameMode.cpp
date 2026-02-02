// Fill out your copyright notice in the Description page of Project Settings.


#include "ScrapItGameMode.h"
#include "MechaPawn.h"

AScrapItGameMode::AScrapItGameMode()
{
	DefaultPawnClass = AMechaPawn::StaticClass();
}

void AScrapItGameMode::BeginPlay()
{
	Super::BeginPlay();
}
