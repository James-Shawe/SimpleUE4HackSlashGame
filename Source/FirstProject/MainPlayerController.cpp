// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset); // we used the tsubclassof in order to use this version of the function

		HUDOverlay->AddToViewport();
		HUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);

		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);

			FVector2D Alignment(0.f, 0.f);
			EnemyHealthBar->SetAlignmentInViewport(Alignment);
		}
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);

		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
			bPauseMenuVisible = false;
		}
	}

}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}	
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUI; // InputModeGameOnly is default, GameAndUI allows for both. UIOnly wont allow key presses so we cant use it as then esc wont work.

		SetInputMode(InputModeGameAndUI);
		SetShowMouseCursor(true);

		UGameplayStatics::SetGamePaused(GetWorld(), true); // Pauses entire world instead of just disabling the main char 
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		// SET visibility is called in blueprints so that transition can be seen.

		GameModeOnly();

		SetShowMouseCursor(false);

		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu(); // use RemovePauseMenu_Implementation(); for c++ implementation only. otherwise use RemovePauseMenu(); for blueprint version
	}
	else
	{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly; // InputModeGameOnly is default, GameAndUI allows for both. UIOnly wont allow key presses so we cant use it as then esc wont work.

	SetInputMode(InputModeGameOnly);
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar)
	{
		FVector2D OutPositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, OutPositionInViewport);
		OutPositionInViewport.Y -= 85.f;

		FVector2D SizeInViewport = FVector2D(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(OutPositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);



	}
}