// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KKCoreTypes.h"
#include "KKCarPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class UKKCarMovementComponent;
class UKKMovementReplicationComponent;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UKKCarMovementComponent* CarMovementComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UKKMovementReplicationComponent* MovementReplicationComponent;

	virtual void BeginPlay() override;

private:
    void MoveForward(float Amount);
    void MoveRight(float Amount);
};
