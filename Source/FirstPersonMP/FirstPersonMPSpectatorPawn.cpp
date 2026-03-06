// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonMPSpectatorPawn.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AFirstPersonMPSpectatorPawn::AFirstPersonMPSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	if (UPawnMovementComponent* Move = GetMovementComponent())
	{
		Move->StopMovementImmediately();
		Move->SetComponentTickEnabled(false);
	}
}

void AFirstPersonMPSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPersonMPSpectatorPawn, OrbitalTarget);
}

void AFirstPersonMPSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InInputComponent))
	{
		EnhancedInputComponent->BindAction(OrbitAction, ETriggerEvent::Triggered, this, &AFirstPersonMPSpectatorPawn::OrbitAroundTarget);
		EnhancedInputComponent->BindAction(JumpToNextPlayerAction, ETriggerEvent::Triggered, this, &AFirstPersonMPSpectatorPawn::JumpToNextPlayer);
		EnhancedInputComponent->BindAction(JumpToPrevPlayerAction, ETriggerEvent::Triggered, this, &AFirstPersonMPSpectatorPawn::JumpToPrevPlayer);
	}
}

void AFirstPersonMPSpectatorPawn::Tick(float DeltaSeconds)
{
	//Owning client
	if (IsLocallyControlled())
	{
		Server_UpdateOrbitInput(OrbitYaw, OrbitPitch);
	}

	if (HasAuthority())
	{
		if (OrbitalTarget == nullptr) return;

		FVector TargetLocation = OrbitalTarget->GetActorLocation();

		float PitchRad = FMath::DegreesToRadians(OrbitPitch);
		float YawRad = FMath::DegreesToRadians(OrbitYaw);

		FVector Offset(
			FMath::Cos(PitchRad) * FMath::Cos(YawRad),
			FMath::Cos(PitchRad) * FMath::Sin(YawRad),
			FMath::Sin(PitchRad)
		);

		FVector NewLocation = TargetLocation + Offset * 300.f;

		SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(NewLocation, TargetLocation);
		SetActorRotation(LookAt);

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetControlRotation(LookAt);
		}
	}
}

void AFirstPersonMPSpectatorPawn::OrbitAroundTarget(const FInputActionValue& InValue)
{
	FVector2D OrbitVector = InValue.Get<FVector2D>();

	OrbitYaw += OrbitVector.X;
	OrbitPitch = FMath::Clamp(OrbitPitch - OrbitVector.Y, -85.f, 85.f);
}

void AFirstPersonMPSpectatorPawn::JumpToNextPlayer(const FInputActionValue& InValue)
{
}

void AFirstPersonMPSpectatorPawn::JumpToPrevPlayer(const FInputActionValue& InValue)
{
}

void AFirstPersonMPSpectatorPawn::Server_UpdateOrbitInput_Implementation(float InYaw, float InPitch)
{
	OrbitYaw = InYaw;
	OrbitPitch = InPitch;
}
