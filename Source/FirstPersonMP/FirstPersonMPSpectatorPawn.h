// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "InputActionValue.h"
#include "FirstPersonMPSpectatorPawn.generated.h"

class UInputAction;

UCLASS()
class FIRSTPERSONMP_API AFirstPersonMPSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
public:
	AFirstPersonMPSpectatorPawn();

	//~Begin UObject
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End UObject

private:
	//~Begin APawn
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	//~End APawn

	//~Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	//~End AActor

	UFUNCTION(Server, unreliable)
	void Server_UpdateOrbitInput(float InYaw, float InPitch);

	void OrbitAroundTarget(const FInputActionValue& InValue);
	void JumpToNextPlayer(const FInputActionValue& InValue);
	void JumpToPrevPlayer(const FInputActionValue& InValue);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* OrbitAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpToNextPlayerAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpToPrevPlayerAction;

	float OrbitYaw, OrbitPitch;

public:
	UPROPERTY(Replicated)
	TObjectPtr<APawn> OrbitalTarget;
};
