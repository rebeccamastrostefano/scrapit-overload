// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "EnemyBase.h"
#include "EnemyBoltTick.generated.h"

class UFloatingPawnMovement;

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemyBoltTick : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBoltTick();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BoltTickMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HurtboxSphere;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UFloatingPawnMovement* MovementComp;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float AttackRange = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float AttackCooldown = 1.f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	int32 AttackForce = 1000;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	int32 KnockbackForce = 800;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float NavigationUpdateRate = 0.5f;
	
	UPROPERTY()
	AAIController* AIController;
	
	FTimerHandle AttackTimer;
	FTimerHandle NavigationTimer;
	
	//Functions
	void UpdateNavigation() const;
	void UpdateRotation(const float DeltaTime);
	
	void StartAttack();
	void ExecuteAttack();
	void ResetMovement();
	
	UFUNCTION()
	void OnHurtboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	virtual void TakeDamage(float DamageAmount) override;
};
