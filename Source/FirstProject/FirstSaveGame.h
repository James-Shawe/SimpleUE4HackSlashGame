// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FirstSaveGame.generated.h"


/* structs are identical to classes except that by default the body of a struct is public, while a classes body is private by default.
* Therefore when creating structs we must include the UE macro GENERATED_BODY() which will provide boilerplate code for use by UE
* Reflection System.
*/

USTRUCT(BlueprintType) // BlueprintType allows struct to be used in blueprints
struct FCharacterStats
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float Health;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float Stamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int32 Coins;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString WeaponName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString LevelName;

};

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API UFirstSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UFirstSaveGame();

	UPROPERTY(VisibleAnywhere, Category="Basic") //Category Basic is neccessary for saving these values
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = "Basic")
	int32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = "Basic") // Use the struct to save data
	FCharacterStats CharacterStats;
	
};
