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

	virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_ServerState)
    FCarState ServerState;

    TArray<FCarMove> UnacknowledgedMoves;

    void MoveForward(float Amount);
    void MoveRight(float Amount);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendMove(FCarMove Move);

    UFUNCTION()
    void OnRep_ServerState();

    void ClearAcknowledgedMoves(FCarMove LastMove);
};
