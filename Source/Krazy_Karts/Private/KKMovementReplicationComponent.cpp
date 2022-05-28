// Fill out your copyright notice in the Description page of Project Settings.

#include "KKMovementReplicationComponent.h"
#include "KKCarPawn.h"
#include "KKCarMovementComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogKKMovementReplicationComponent, All, All);

UKKMovementReplicationComponent::UKKMovementReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicatedByDefault(true);
}

void UKKMovementReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

    SetCarMovementComponent();
}

void UKKMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CarMovementComponent) return;

    FCarMove LastMove = CarMovementComponent->GetLastMove();

    if (GetOwnerRole() == ROLE_AutonomousProxy)
    {
        UnacknowledgedMoves.Add(LastMove);
        Server_SendMove(LastMove);
    }

    if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)  // IsLocallyControlled())
    {
        UpdateServerState(LastMove);
    }

    if (GetOwnerRole() == ROLE_SimulatedProxy)
    {
        CarMovementComponent->SimulateMove(ServerState.LastMove);
    }
}

void UKKMovementReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UKKMovementReplicationComponent, ServerState);
}

void UKKMovementReplicationComponent::Server_SendMove_Implementation(FCarMove Move)
{
    if (!CarMovementComponent) return;

    CarMovementComponent->SimulateMove(Move);

    UpdateServerState(Move);
}

bool UKKMovementReplicationComponent::Server_SendMove_Validate(FCarMove Move)
{
    return true;
}

void UKKMovementReplicationComponent::OnRep_ServerState()
{
    if (!CarMovementComponent) return;

    FString RoleString;
    UEnum::GetValueAsString(GetOwnerRole(), RoleString);

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

void UKKMovementReplicationComponent::SetCarMovementComponent()
{
    const auto Car = GetOwner<AKKCarPawn>();
    if (!Car) return;

    CarMovementComponent = Car->FindComponentByClass<UKKCarMovementComponent>();
}

void UKKMovementReplicationComponent::UpdateServerState(FCarMove Move)
{
    ServerState.LastMove = Move;
    ServerState.Transform = GetOwner()->GetActorTransform();
    ServerState.Velocity = CarMovementComponent->GetVelocity();
}
