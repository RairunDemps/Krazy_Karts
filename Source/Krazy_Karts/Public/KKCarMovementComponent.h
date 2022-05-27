// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KKCoreTypes.h"
#include "KKCarMovementComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZY_KARTS_API UKKCarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKKCarMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    FCarMove CreateMove(float DeltaTime);
    void SimulateMove(const FCarMove& Move);

    void SetThrottle(float Value) { Throttle = Value; }
    void SetSteeringThrow(float Value) { SteeringThrow = Value; }

    FVector GetVelocity() const { return Velocity; }
    void SetVelocity(const FVector& Vector) { Velocity = Vector; }

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    float Multiplier = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    float Weight = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Velocity")
    float DrivingForce = 10000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation")
    float TurningRadius = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resistance")
    float DragCoefficient = 16.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resistance")
    float RollingCoefficient = 0.0015f;

	virtual void BeginPlay() override;

private:
    float Throttle;
    float SteeringThrow;
    FVector Velocity;

    void UpdatePositionFromVelocity(float DeltaTime);
    void UpdateRotation(float DeltaTime, float MoveSteeringThrow);

    FVector GetAirResistance();
    FVector GetRollingResistance();
};
