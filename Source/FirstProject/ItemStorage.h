// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemStorage.generated.h"

class AWeapon;

UCLASS()
class FIRSTPROJECT_API AItemStorage : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemStorage();


	/* Maps are like python dictionaries. The TMap version is the same as default c++ map and is instantiated with the key and then value in <> */

	UPROPERTY(EditDefaultsOnly, Category="SaveData")
	TMap<FString, TSubclassOf<AWeapon>> WeaponMap;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
