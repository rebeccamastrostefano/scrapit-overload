// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scrappable.h"
#include "ScrapActor.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AScrapActor : public AActor, public IScrappable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScrapActor();
	virtual void OnMagnetPulled(FVector MagnetLocation, float PullStrength) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ScrapMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
