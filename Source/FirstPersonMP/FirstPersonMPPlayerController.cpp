// Copyright Epic Games, Inc. All Rights Reserved.


#include "FirstPersonMPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "FirstPersonMPCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "FirstPersonMP.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/GameStateBase.h"
#include "FirstPersonMPSpectatorPawn.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

AFirstPersonMPPlayerController::AFirstPersonMPPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AFirstPersonMPCameraManager::StaticClass();
}

void AFirstPersonMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogFirstPersonMP, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AFirstPersonMPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

ASpectatorPawn* AFirstPersonMPPlayerController::SpawnSpectatorPawn()
{
	if (!LastPossessedPawn.IsValid()) return nullptr;

	SetSpawnLocation(LastPossessedPawn->GetActorLocation());

	ASpectatorPawn* SpawnedSpectator = nullptr;

	if ((GetSpectatorPawn() == nullptr) && HasAuthority())
	{
		UWorld* World = GetWorld();

		if (UClass* SpectatorClass = World->GetAuthGameMode()->SpectatorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.ObjectFlags |= RF_Transient;
			SpawnedSpectator = World->SpawnActor<ASpectatorPawn>(SpectatorClass, GetSpawnLocation(), GetControlRotation(), SpawnParams);
			if (SpawnedSpectator)
			{
				if (PlayerState->IsSpectator())
				{
					this->Possess(SpawnedSpectator);
				}
				SpawnedSpectator->DispatchRestart(true);
				if (SpawnedSpectator->PrimaryActorTick.bStartWithTickEnabled)
				{
					SpawnedSpectator->SetActorTickEnabled(true);
				}

				if (AFirstPersonMPSpectatorPawn* TPSP = Cast<AFirstPersonMPSpectatorPawn>(SpawnedSpectator))
				{
					for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
					{
						ACharacter* FoundCharacter = *It;
						AFirstPersonMPPlayerController* FoundPlayerController = Cast<AFirstPersonMPPlayerController>(FoundCharacter->GetController());
						if (FoundPlayerController != nullptr && FoundPlayerController != this)
						{
							TPSP->OrbitalTarget = FoundCharacter;
						}
					}
				}
			}
			else
			{
				UE_LOG(LogPlayerController, Warning, TEXT("Failed to spawn spectator with class %s"), *GetNameSafe(SpectatorClass));
			}
		}
	}

	return SpawnedSpectator;
}

void AFirstPersonMPPlayerController::OnUnPossess()
{
	LastPossessedPawn = GetPawn();
	Super::UnPossess();
}

void AFirstPersonMPPlayerController::Client_ChangeToSpectatorMappingContext_Implementation()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(SpectatorMappingContext, 0);
	}
}

void AFirstPersonMPPlayerController::SetPlayerSpectate()
{
	if (!HasAuthority()) return;

	PlayerState->SetIsSpectator(true);
	ChangeState(NAME_Spectating);
	bPlayerIsWaiting = true;

	if (!(GetNetMode() == ENetMode::NM_ListenServer && HasAuthority()))
	{
		ClientGotoState(NAME_Spectating);
	}

	Client_ChangeToSpectatorMappingContext();
}