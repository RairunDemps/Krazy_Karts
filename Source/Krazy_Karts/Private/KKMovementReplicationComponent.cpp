// Fill out your copyright notice in the Description page of Project Settings.

#include "KKMovementReplicationComponent.h"
#include "KKCarPawn.h"
#include "KKCarMovementComponent.h"
#include "Net/UnrealNetwork.h"

UKKMovementReplicationComponent::UKKMovementReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicatedByDefault(true);
}

void UKKMovementReplicationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKKMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UKKMovementReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UKKMovementReplicationComponent, ServerState);
}

void UKKMovementReplicationComponent::Server_SendMove_Implementation(FCarMove Move)
{
    const auto CarMovementComponent = GetCarMovementComponent();
    if (!CarMovementComponent) return;

    CarMovementComponent->SimulateMove(Move);

    ServerState.LastMove = Move;
    ServerState.Transform = GetOwner()->GetActorTransform();
    ServerState.Velocity = CarMovementComponent->GetVelocity();
}

bool UKKMovementReplicationComponent::Server_SendMove_Validate(FCarMove Move)
{
    return true;
}

void UKKMovementReplicationComponent::OnRep_ServerState()
{
    const auto CarMovementComponent = GetCarMovementComponent();
    if (!CarMovementComponent) return;

    GetOwner()->SetActorTransform(ServerState.Transform);
    CarMovementComponent->SetVelocity(ServerState.Velocity);

    ClearAcknowledgedMoves(ServerState.LastMove);

    for (const auto& UnacknowledgedMove : UnacknowledgedMoves)
    {
        CarMovementComponent->SimulateMove(UnacknowledgedMove);
    }
}

void UKKMovementReplicationComponent::ClearAcknowledgedMoves(FCarMove LastMove)
{
    TArray<FCarMove> CurrentUnacknowledgedMoves;

    for (const auto& Move : UnacknowledgedMoves)
    {
        if (Move.Time > LastMove.Time)
        {
            CurrentUnacknowledgedMoves.Add(Move);
        }
    }

    UnacknowledgedMoves = CurrentUnacknowledgedMoves;
}

UKKCarMovementComponent* UKKMovementReplicationComponent::GetCarMovementComponent() const
{
    const auto Car = GetOwner<AKKCarPawn>();
    if (!Car) return nullptr;

    return Car->FindComponentByClass<UKKCarMovementComponent>();
}
