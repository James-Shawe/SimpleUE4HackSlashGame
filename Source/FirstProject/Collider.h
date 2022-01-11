// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class UColliderMovementComponent;

UCLASS()
class FIRSTPROJECT_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Good practive to use UPROPERTY not just for editing in blueprints but to allow it to be garbage collected. C++ will not do this natively. UE4 will when specified with UPROPERTY
	UPROPERTY(VisibleAnywhere, Category="Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Mesh")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UColliderMovementComponent* OurMovementComponent; // MovementComponent Variable is automatically inherited by all Pawns. To avoid shadowing we make our custom one with a unique name.

	virtual UPawnMovementComponent* GetMovementComponent() const override; // Need to overide this here so that Unreal Engine uses our custom MovementComponent.

	// Getters and Setters
	FORCEINLINE UStaticMeshComponent* GetMeshComponent() { return MeshComponent; } 
	FORCEINLINE void SetMeshComponent(UStaticMeshComponent* Mesh) { MeshComponent = Mesh; }
	FORCEINLINE USphereComponent* GetSphereComponent() { return SphereComponent; }
	FORCEINLINE void SetSphereComponent(USphereComponent* Sphere) { SphereComponent = Sphere; }

	FORCEINLINE UCameraComponent* GetCameraComponent() { return Camera; }
	FORCEINLINE void SetCameraComponent(UCameraComponent* InCamera) { Camera = InCamera; }
	FORCEINLINE USpringArmComponent* GetSpringArmComponent() { return SpringArm; }
	FORCEINLINE void SetSphereComponent(USpringArmComponent* InSpringArm) { SpringArm= InSpringArm; }
	// Inline is an optimisation that swaps everywhere there is a functioncall with the function def at compile time.
	// This prevents jumping to different locations in code for a function call, which for small functions like this is more costly than running it here.
	// Short answer, use inline when function is very small like a getter or setter.

private:

	 void MoveForward(float input);

	 void MoveRight(float input);

	 void PitchCamera(float input);

	 void YawCamera(float input);

	 FVector2D CameraInput;

};
