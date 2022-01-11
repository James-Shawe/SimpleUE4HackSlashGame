// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingPlatform.generated.h"

UCLASS()
class FIRSTPROJECT_API AFloatingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatingPlatform();

	/** Mesh for the Platform */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Platform")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Platform")
	FVector StartPoint;

	/* Giving the vector a MakeEditWidget converts the Vector from World Coords to relative Coords*/
	UPROPERTY(EditAnywhere, Category = "Platform", meta=(MakeEditWidget = "true"))
	FVector EndPoint;

	UPROPERTY(EditAnywhere, Category = "Platform")
	float InterpSpeed;

	UPROPERTY(EditAnywhere, Category = "Platform")
	float InterpTime;

	FTimerHandle InterpTimer;

	bool bInterping;

	float Distance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE void ToggleInterping() { bInterping = !bInterping; }

	// pass vectors by reference so we alter directly the values passed in. No need to return as its not pass by copy, and we dont have to use pointers to acheive this. 
	FORCEINLINE void SwapVectors(FVector& VecOne, FVector& VecTwo) { FVector Temp = VecOne; VecOne = VecTwo; VecTwo = Temp; }

};
