// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/ScrapItGameInstance.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"

void UScrapItGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UScrapItGameInstance::BeginLoadingScreen);
}

void UScrapItGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreenAttributes;
		LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;

		LoadingScreen = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidget);
		LoadingScreenAttributes.WidgetLoadingScreen = LoadingScreen->TakeWidget();

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	}
}

void UScrapItGameInstance::LoadLevel(TSoftObjectPtr<UWorld> LevelToLoad)
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToLoad);
}
