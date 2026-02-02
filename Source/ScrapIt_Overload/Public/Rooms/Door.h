// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;
	
	UPROPERTY(EditAnywhere, Category = "Door Settings")
	ERoomType NextRoomType;
	
	UFUNCTION()
	void OnDoorOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
