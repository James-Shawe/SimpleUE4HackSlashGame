// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "PickUp.generated.h"

/**
 * 
 */

class AMain;

UCLASS()
class FIRSTPROJECT_API APickUp : public AItem
{
	GENERATED_BODY()

public:

	APickUp();

	/* When inheriting a function from a parent class, the UFUNCTION and its properties is also inherited. This means we cannot declare the child with UFUNCTION() */
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintImplementableEvent, Category="PickUp")
	void OnPickUpBP(AMain* Target);
	
};
