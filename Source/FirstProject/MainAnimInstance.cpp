// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/PawnMovementComponent.h"
#include "MainAnimInstance.h"
#include "Main.h"

// essentially this is the anim instance contructor
void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); // Animation instance will try to get the pawn that owns it.

		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}

// and this is basically a tick function. It is custom built, name is arbitrary.
void UMainAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f); //Dont care about z axis jump for walking/run anim.
		MovementSpeed = LateralSpeed.Size();

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if (!Main)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}