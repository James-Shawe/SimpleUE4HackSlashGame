// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

// essentially this is the anim instance contructor
void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); // Animation instance will try to get the pawn that owns it.
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}
}

// and this is basically a tick function. It is custom built, name is arbitrary.
void UEnemyAnimInstance::UpdateAnimationProperties()
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

		if (!Enemy)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}
}
