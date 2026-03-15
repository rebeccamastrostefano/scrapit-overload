// Fill out your copyright notice in the Description page of Project Settings.

#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Core/ScrapItGameInstance.h"

#include "Engine/StreamableManager.h"

void UScrapItGameInstance::LoadLevel(TSoftObjectPtr<UWorld> LevelToLoad)
{
	if (LoadingScreenWidget != nullptr)
	{
		//Show loading screen
		LoadingScreen = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidget);
		LoadingScreen->AddToViewport();
	}

	PendingLevelLoading = LevelToLoad;
	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	StreamingHandle = StreamableManager.RequestAsyncLoad(LevelToLoad.ToSoftObjectPath(),
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UScrapItGameInstance::OnLevelLoaded));
}

void UScrapItGameInstance::OnLevelLoaded()
{
	LoadingScreen->RemoveFromParent();
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, PendingLevelLoading);
}
