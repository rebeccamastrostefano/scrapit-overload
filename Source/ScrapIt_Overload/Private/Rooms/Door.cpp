// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Door.h"
#include "Mecha/MechaPawn.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"

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

void ADoor::SetRoomType(const ERoomType NewRoomType)
{
	NextRoomType = NewRoomType;
}

void ADoor::OnDoorOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AMechaPawn* MechaPawn = Cast<AMechaPawn>(OtherActor);
	if (MechaPawn == nullptr)
	{
		return;
	}
	
	UPersistentManager* PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	if (PersistentManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No Persistent Manager"));
		return;
	}
	
	const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No Game Instance"));
		return;
	}
	
	//Save the Player's state before progressing
	PersistentManager->SaveMechaState(
		MechaPawn->GetCurrentScrapCount(),
		MechaPawn->GetCurrentHealth(),
		MechaPawn->GetCurrentTier().TierNumber,
		MechaPawn->GetCurrentWeaponLoadout()
		);
	
	//Advance the room in the persistent manager
	PersistentManager->AdvanceRoom();
	
	//Load the next level based on the room type
	if (const TSoftObjectPtr<UWorld>* LevelPtr = GameInstance->RoomLevels.Find(NextRoomType))
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, *LevelPtr);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Level assigned for Room Type: %d"), NextRoomType);
	}
}

