// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
	UPROPERTY(EditAnywhere)
	float AttackRange = 100.f;
	
	UPROPERTY(EditAnywhere)
	float AttackCooldown = 1.f;
	
	UPROPERTY(EditAnywhere)
	int32 AttackForce = 1000;
	
	UPROPERTY(EditAnywhere)
	int32 KnockbackForce = 800;
	
	//Attack
	UFUNCTION()
	void OnHurtboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	FTimerHandle AttackTimer;
	void StartAttack();
	void ExecuteAttack();
	void ResetMovement();

public:	
	virtual void TakeDamage(float DamageAmount) override;
};
