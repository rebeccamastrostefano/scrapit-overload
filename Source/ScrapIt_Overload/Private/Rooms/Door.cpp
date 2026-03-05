// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Door.h"
#include "Mecha/MechaPawn.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Rooms/LevelsManager.h"

// Sets default values
ADoor::ADoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnDoorOverlap);
}

void ADoor::SetRoomID(const int32 RoomID)
{
	TargetRoomID = RoomID;
}

void ADoor::OnDoorOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                          int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AMechaPawn* MechaPawn = Cast<AMechaPawn>(OtherActor);
	if (MechaPawn == nullptr)
	{
		return;
	}

	UPersistentManager* PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	check(PersistentManager != nullptr);

	ULevelsManager* LevelsManager = GetGameInstance()->GetSubsystem<ULevelsManager>();
	check(LevelsManager != nullptr);

	//Save the Player's state before progressing
	PersistentManager->SaveMechaState(
		MechaPawn->GetCurrentScrapCount(),
		MechaPawn->GetCurrentHealth(),
		MechaPawn->GetCurrentTier().TierNumber,
		MechaPawn->GetCurrentWeaponLoadout()
	);

	//Advance the room in the persistent manager
	PersistentManager->AdvanceRoom();

	//Save the direction we are leaving from
	LevelsManager->SetLastExitDirection(DoorDirection);

	//Load the next level based on the target room id
	LevelsManager->SetCurrentRoomID(TargetRoomID);
	LevelsManager->LoadRoomByID(TargetRoomID);
}

void ADoor::Open()
{
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//TODO: Play animation/mesh change
}

void ADoor::Close()
{
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//TODO: Play animation/mesh change
}
