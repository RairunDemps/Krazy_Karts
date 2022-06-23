// Fill out your copyright notice in the Description page of Project Settings.

#include "KKCarMovementComponent.h"
#include "GameFramework/GameState.h"

UKKCarMovementComponent::UKKCarMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKKCarMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKKCarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
    {
        LastMove = CreateMove(DeltaTime);
        SimulateMove(LastMove);
    }
}

FCarMove UKKCarMovementComponent::CreateMove(float DeltaTime)
{
    FCarMove Move;
    Move.Throttle = Throttle;
    Move.SteeringThrow = SteeringThrow;
    Move.DeltaTime = DeltaTime;

    if (GetWorld())
    {
        Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
    }

    return Move;
}

void UKKCarMovementComponent::SimulateMove(const FCarMove& Move)
{
    if (!GetOwner()) return;

    FVector Force = GetOwner()->GetActorForwardVector() * DrivingForce * Move.Throttle;
    Force += GetAirResistance();
    Force += GetRollingResistance();

    const FVector Acceleration = Force / Weight;
    Velocity = Velocity + Acceleration * Move.DeltaTime;

    UpdateRotation(Move.DeltaTime, Move.SteeringThrow);
    UpdatePositionFromVelocity(Move.DeltaTime);
}

void UKKCarMovementComponent::UpdatePositionFromVelocity(float DeltaTime)
{
    if (!GetOwner()) return;

    const FVector Translation = Velocity * Multiplier * DeltaTime;

    FHitResult HitResult;
    GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);
    if (HitResult.IsValidBlockingHit())
    {
        Velocity = FVector::ZeroVector;
    }
}

void UKKCarMovementComponent::UpdateRotation(float DeltaTime, float MoveSteeringThrow)
{
    if (!GetOwner()) return;

    const float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
    const float RotationAngle = DeltaLocation / TurningRadius * MoveSteeringThrow;
    const FQuat Rotation(GetOwner()->GetActorUpVector(), RotationAngle);
    GetOwner()->AddActorLocalRotation(Rotation);
    Velocity = Rotation.RotateVector(Velocity);
}

FVector UKKCarMovementComponent::GetAirResistance()
{
    return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UKKCarMovementComponent::GetRollingResistance()
{
    if (!GetWorld()) return FVector::ZeroVector;

    const float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
    const float NormalForce = Weight * AccelerationDueToGravity;

    return -Velocity.GetSafeNormal() * RollingCoefficient * NormalForce;
}
