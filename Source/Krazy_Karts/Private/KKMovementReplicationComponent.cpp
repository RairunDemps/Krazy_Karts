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

    if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
    {
        UpdateServerState(LastMove);
    }

    if (GetOwnerRole() == ROLE_SimulatedProxy)
    {
        ClientTick(DeltaTime);
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
    switch (GetOwnerRole())
    {
        case ROLE_AutonomousProxy:
            AutonomousProxy_OnRep_ServerState();
            break;
        case ROLE_SimulatedProxy:
            SimulatedProxy_OnRep_ServerState();
            break;
        default:
            break;
    }
}

void UKKMovementReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
    if (!CarMovementComponent) return;

    GetOwner()->SetActorTransform(ServerState.Transform);
    CarMovementComponent->SetVelocity(ServerState.Velocity);

    ClearAcknowledgedMoves(ServerState.LastMove);

    for (const auto& UnacknowledgedMove : UnacknowledgedMoves)
    {
        CarMovementComponent->SimulateMove(UnacknowledgedMove);
    }
}

void UKKMovementReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
    TimeBetweenLastUpdates = TimeSinceUpdate;
    TimeSinceUpdate = 0;

    if (!GetOwner() || !CarMovementComponent) return;

    ClientStartTransform = GetOwner()->GetActorTransform();
    ClientStartVelocity = CarMovementComponent->GetVelocity();
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

void UKKMovementReplicationComponent::ClientTick(float DeltaTime)
{
    TimeSinceUpdate += DeltaTime;

    if (!GetOwner() || TimeBetweenLastUpdates < KINDA_SMALL_NUMBER || !CarMovementComponent) return;

    FVector StartLocation = ClientStartTransform.GetLocation();
    FVector TargetLocation = ServerState.Transform.GetLocation();
    float LerpRatio = TimeSinceUpdate / TimeBetweenLastUpdates;
    float VelocityToDerivative = TimeBetweenLastUpdates * 100;
    FVector StartDerivative = ClientStartVelocity * VelocityToDerivative;
    FVector TargetDerivative = ServerState.Velocity * VelocityToDerivative;

    FVector NewDerivative = FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    FVector NextVelocity = NewDerivative / VelocityToDerivative;
    CarMovementComponent->SetVelocity(NextVelocity);

    FVector NextLocation = FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    FQuat NextRotation = FQuat::Slerp(ClientStartTransform.GetRotation(), ServerState.Transform.GetRotation(), LerpRatio);
    
    GetOwner()->SetActorLocation(NextLocation);
    GetOwner()->SetActorRotation(NextRotation);
}
