// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom. Pulls towards player is there is a collision.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; //  Camera follows at this length
	CameraBoom->bUsePawnControlRotation = true; //Rotate spring arm based on the player controller component, instead of capsule component.

	GetCapsuleComponent()->InitCapsuleSize(29.05f, 88.f); // Hard code capsule size for character after using blueprint to test for correct size.

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attech the camera to the end of the boom and let the boom adjust to match the controller orientation.
	FollowCamera->bUsePawnControlRotation = false; 
	// The cameraboom will be controlled by the player controller and the camera will be controlled by the cameraboom. 
	// Therefore direct control by player controller is not wanted.

	// Set turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Dont Rotate the character when the player controller rotates. Stops character rotating with camera controls.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// GetCharacterMovement returns the already assigned movement component of the character class. 
	GetCharacterMovement()->bOrientRotationToMovement = true; //enables character to automatically face direction of movement.
	// set the rotation rate of the OrientRotationToMovement. So 540 in the Yaw axis only. So only when turning on the spot and not facing the ground when jumping.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 340.f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 350.f;
	Stamina = 350.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 1000.f;

	bShiftKeyDown = false;
	bInteractKeyDown = false;
	bAttackKeyDown = false;
	bEscKeyDown = false;

	// Initialise Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;
	
	bMovingForward = false;
	bMovingRight = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (Map != "SunTemple")
	{
		LoadGameNoSwitch();

		if (MainPlayerController)
		{
			MainPlayerController->GameModeOnly(); // to prevent weird mouse locking upon load due to loading from hud.
		}
	}
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{

	case EStaminaStatus::ESS_Normal:

		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}

			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			
		}
		else // shift key is up
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:

		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				if (bMovingForward || bMovingRight)
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		} 
		else //Shift key is up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:

		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else // Shift key is up
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:

		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	default:
		;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation()); // the target rotator for RInterpTo
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); // Create the rotation matrix needed for smooth transform between frames.
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AMain::Jump); // Character class already has a jump function
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping); // Character class already has a jump function

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AMain::InteractKeyDown);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Released, this, &AMain::InteractKeyUp);

	FInputActionBinding& PauseButton = PlayerInputComponent->BindAction(TEXT("ESC"), IE_Pressed, this, &AMain::EscKeyDown); 
	PlayerInputComponent->BindAction(TEXT("ESC"), IE_Released, this, &AMain::EscKeyUp);

	PauseButton.bExecuteWhenPaused = true; // To allow pressing the esc key again to unpause the menu.

	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AMain::AttackKeyDown);
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Released, this, &AMain::AttackKeyUp);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMain::MoveRight);
	// All characters inherit from pawn so have this function. No need to define a new function just to call 1 function.
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AMain::LookUpAtRate);
}


void AMain::MoveForward(float Value)
{
	bMovingForward = false; // This is called every frame due to bindaxis (i think)

	// All Characters have a controller variable as its inherited from Pawns.
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		// Find out which way is forward using the direction that the player controller is facing.
		const FRotator Rotation = Controller->GetControlRotation(); // gives direction the controller is facing this frame
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		// create a rotationMatrix from the rotator, which gives access to the local unit vectors of the Rotator.
		// Allows us to get a direction vector from a Rotator, essentially.
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // X axis for forward
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}

}


void AMain::MoveRight(float Value)
{
	bMovingRight = false;

	// All Characters have a controller variable as its inherited from Pawns.
	if ((Controller != nullptr) && (Value != 0.f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		// Find out which way is forward using the direction that the player controller is facing.
		const FRotator Rotation = Controller->GetControlRotation(); // gives direction the controller is facing this frame
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		// create a rotationMatrix from the rotator, which gives access to the local unit vectors of the Rotator.
		// Allows us to get a direction vector from a Rotator, essentially.
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // Y axis for right
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

void AMain::TurnAtRate(float Rate)
{
	// Function made to specifically add rotation in the Yaw axis to the player controller. Rate is player input and is 1.0f or 0.f.
	// This should give degs per second with the number of deg per second given by value of BaseLookUp Rate.
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); 
}


void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::InteractKeyDown()
{
	bInteractKeyDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);

		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr); // item is equipped so the Overlapping Item is no longer "active"
		}
	}
}

void AMain::InteractKeyUp()
{
	bInteractKeyDown = false;
}

void AMain::AttackKeyDown()
{
	bAttackKeyDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	if (EquippedWeapon)
	{
		Attack();
	}
}

void AMain::AttackKeyUp()
{
	bAttackKeyDown = false;
}

void AMain::EscKeyDown()
{
	bEscKeyDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::EscKeyUp()
{
	bEscKeyDown = false;
}

void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}

void AMain::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}

void AMain::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return; // if already dead then return.

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); // get the anim instance attached to main
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);

}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}


void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::ShowPickUpLocations()
{
	for (FVector Location : PickUpLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 12, FLinearColor::Red, 5.f, 2.f);
	}
	
}

void AMain::Attack()
{
	if (!bAttacking && (MovementStatus != EMovementStatus::EMS_Dead)) // prevents animation cancelling from spamming the attack button
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); // get the anim instance attached to main
		if (AnimInstance && CombatMontage)
		{
			int32 Section = FMath::RandRange(0, 2);
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.8f); //play the montage created in editor
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage); //jump to section in montage
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.35f); //play the montage created in editor
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage); //jump to section in montage
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.35f); //play the montage created in editor
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage); //jump to section in montage
				break;

			default:

				break;
			}
			
		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bAttackKeyDown)
	{
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AMain::UpdateCombatTarget()
{
	TArray<AActor*> OutOverlappingActors;
	GetOverlappingActors(OutOverlappingActors, EnemyFilter);

	if (OutOverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OutOverlappingActors[0]);
	
	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation()- Location).Size();

		for (auto Actor : OutOverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();

				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		SetHasCombatTarget(true);
	}

	
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();

	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(CurrentLevel); 

		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}

	}
}


void AMain::SaveGame()
{
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass())); // essentially construct a new FirstSaveGame object

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	// This actually saves the game to the computer memory
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition)
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass())); // essentially construct a new FirstSaveGame object

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex)); // works since we are saving with default values for slot name and index too.

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) // Set from blueprints as AItemStorage_BP
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage); // Spawn an AItemStorage_BP object and store it in variable of parent class type AItemStorage

		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]); //Spawn a weapon object that has the name that was saved, and save in parent var
				WeaponToEquip->Equip(this);
			}
		}
	}

	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(LoadGameInstance->CharacterStats.LevelName);

		SwitchLevel(LevelName);
	}
}

void AMain::LoadGameNoSwitch()
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass())); // essentially construct a new FirstSaveGame object

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex)); // works since we are saving with default values for slot name and index too.

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) // Set from blueprints as AItemStorage_BP
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage); // Spawn an AItemStorage_BP object and store it in variable of parent class type AItemStorage

		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]); //Spawn a weapon object that has the name that was saved, and save in parent var
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}