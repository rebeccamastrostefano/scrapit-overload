// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Damageable.h"
#include "Scrappable.h"
#include "Components/SphereComponent.h"
#include "Enemy_BoltTick.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemy_BoltTick : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemy_BoltTick();
	virtual void TakeDamage(float DamageAmount) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BoltTickMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HurtboxSphere;
	
	UPROPERTY(EditAnywhere)
	float BaseHealth;
	
	UPROPERTY(EditAnywhere)
	int32 ScrapDrop;
	
	UPROPERTY(VisibleAnywhere)
	float CurrentHealth;
	
	void SpawnScrap();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
