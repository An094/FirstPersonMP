// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstPersonMPPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract)
class FIRSTPERSONMP_API AFirstPersonMPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AFirstPersonMPPlayerController();

	UFUNCTION(BlueprintCallable)
	void SetPlayerSpectate();
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	UInputMappingContext* SpectatorMappingContext;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	virtual ASpectatorPawn* SpawnSpectatorPawn() override;

	virtual void OnUnPossess() override;

	UFUNCTION(Client, Reliable)
	void Client_ChangeToSpectatorMappingContext();

	TWeakObjectPtr<APawn> LastPossessedPawn;

};
