// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Interfaces/Damageable.h"
#include "Core/PersistentManager.h"
#include "MechaPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrapCountChanged, int32, NewScrapCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAcquired, TSubclassOf<AActor>, WeaponClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierChanged, int32, TierNumber, int32, NextTierThreshold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, Health);

UCLASS()
class SCRAPIT_OVERLOAD_API AMechaPawn : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMechaPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/* --- INPUT --- */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ThrustAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MagnetAction;
	
	void ApplyThrust(const FInputActionValue& Value);
	void ApplySteer(const FInputActionValue& Value);
	
	/* --- COMPONENTS --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MechaMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* Camera;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
	class UBoxComponent* Hurtbox;
	
	//Sockets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketFront;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketBack;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketRight;
	
	//Wheels
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Wheels")
	UStaticMeshComponent* WheelFrontLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Wheels")
	UStaticMeshComponent* WheelFrontRight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Wheels")
	UStaticMeshComponent* WheelBackLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Wheels")
	UStaticMeshComponent* WheelBackRight;
	
	/* --- MOVEMENT SETTINGS --- */
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float BaseAccelerationForce = 400000.0f;
	
	float CurrentSteerAngle = 0.0f;
	float TargetSteerAngle = 0.0f;
	float CurrentWheelRoll = 0.0f;
	float CurrentWheelAngle = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float MaxSteerAngle = 35.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float BaseSteeringSpeed = 20.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float HandlingFactor = 0.05f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float GripStrength = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float WheelRadius = 35.f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float MaxWheelAngle = 35.f;
	
	/* --- CAMERA SETTINGS --- */
	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float CameraSmoothness = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float MaxCameraLagDistance = 100.0f;
	
	/* --- MECHA SETTINGS --- */
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	float MagnetStrength = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	float MagnetRadius = 50.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	float CollectionRadius = 150.0f;
	
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.1", ClampMin = "0.1", ClampMax = "1.0"), Category = "Mecha Settings")
	float MagnetSpeedDecrease = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	TArray<FMassTier> MassTiers;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Settings")
	TArray<UStaticMeshComponent*> MassMeshParts;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	float ScrapShieldAbsorption = 1.f; //If 1, 1 Damage = 1 Scrap Removed
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mecha Settings")
	float CoreMaxHealth = 100.f;

	bool bIsMagnetActive = false;
	
	/* --- Current Run State --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	int32 CurrentScraps = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	float CurrentAcceleration = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	float CurrentSteerSpeed = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	FMassTier CurrentTier;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	float CurrentHealth = 100.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha State")
	TArray<FWeaponData> WeaponLoadout;
	
	TMap<EWeaponSocket, AActor*> SocketsToWeapons;
	
	void LoadMechaState();
	
	/* --- Movement Functions --- */
	void UpdateThrust(float Value);
	void UpdateSteer(float DeltaTime);
	void ApplyLateralFriction();
	void AnimateWheels(float DeltaTime);
	
	/* --- Magnet Functions --- */
	void ActivateMagnet();
	void ToggleMagnet();
	
	/* --- Tier Functions --- */
	void CheckTier();
	void UpdateMassStats(const FMassTier& Tier);
	void UpdateMassVisuals(const FMassTier& Tier);
	
	virtual void Die() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//SETTERS
	void AddScrap(int32 Amount);
	void RemoveScrap(int32 Amount);
	
	//GETTERS
	UFUNCTION(BlueprintPure, Category = "Mecha State")
	int32 GetCurrentScrapCount() const
	{
		return CurrentScraps;
	}
	
	UFUNCTION(BlueprintPure, Category = "Mecha State")
	TArray<FWeaponData>& GetCurrentWeaponLoadout()
	{
		return WeaponLoadout;
	}
	
	UFUNCTION(BlueprintPure, Category = "Mecha State")
	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	UFUNCTION(BlueprintPure, Category = "Mecha State")
	FMassTier GetCurrentTier() const
	{
		return CurrentTier;
	}
	
	UFUNCTION(BlueprintPure, Category = "Mecha State")
	TArray<EWeaponSocket> GetAvailableSockets() const
	{
		TArray<EWeaponSocket> AvailableSockets = { EWeaponSocket::Front, EWeaponSocket::Back, EWeaponSocket::Left, EWeaponSocket::Right };
		TArray<EWeaponSocket> NotAvailable;
		SocketsToWeapons.GetKeys(NotAvailable);
		
		for (EWeaponSocket Socket : NotAvailable)
		{
			AvailableSockets.Remove(Socket);
		}
		return AvailableSockets;
	}
	
	//Events
	UPROPERTY(BlueprintAssignable, Category = "Mecha State")
	FOnScrapCountChanged OnScrapCountChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Mecha State")
	FOnTierChanged OnTierChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Mecha Weapons")
	FOnWeaponAcquired OnWeaponAcquired;
	
	UPROPERTY(BlueprintAssignable, Category = "Mecha Stats")
	FOnHealthChanged OnHealthChanged;
	
	//Public Functions
	void EquipWeapon(TSubclassOf<AActor> WeaponClass);
	
	UFUNCTION(BlueprintCallable, Category = "Mecha Weapons")
	void AttachWeaponToSocket(TSubclassOf<AActor> WeaponClass, EWeaponSocket Socket);
	
	virtual void TakeDamage(float Amount) override;
};
