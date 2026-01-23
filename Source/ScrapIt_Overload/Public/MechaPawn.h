// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "MechaPawn.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AMechaPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMechaPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// INPUT
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
	
	// COMPONENTS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MechaMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* Camera;
	
	// MOVEMENT SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float AccelerationForce = 500000.0f;
	
	float CurrentSteerAngle = 0.0f;
	float TargetSteerAngle = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float MaxSteerAngle = 35.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float SteeringSpeed = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float HandlingFactor = 0.05f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float GripStrength = 0.5f;
	
	// CAMERA SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float CameraSmoothness = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float MaxCameraLagDistance = 100.0f;
	
	// MAGNET SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float MagnetStrength = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float MagnetRadius = 50.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float CollectionRadius = 150.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float SpeedDecrease = 3.0f;
	
	bool bIsMagnetActive = false;

	// Movement Functions
	void UpdateThrust(float Value);
	void UpdateSteer(float DeltaTime);
	void ApplyLateralFriction();
	
	//Magnet Functions
	void ActivateMagnet();
	void ToggleMagnetMovement();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
