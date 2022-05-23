// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KKCarPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UBoxComponent;

UCLASS()
class KRAZY_KARTS_API AKKCarPawn : public APawn
{
	GENERATED_BODY()

public:
	AKKCarPawn();

	virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* BoxCollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UCameraComponent* CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    float Multiplier = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    float Weight = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    FVector DrivingForce = FVector(0.0f, 10000.0f, 0.0f);

	virtual void BeginPlay() override;
private:
    float Throttle;

    FVector Velocity;
    FVector Acceleration;

    void MoveForward(float Amount);
    void SetNewActorPosition(float DeltaTime);
};
