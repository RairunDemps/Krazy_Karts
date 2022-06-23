// Fill out your copyright notice in the Description page of Project Settings.

#include "KKMovementReplicationComponent.h"
#include "KKCarPawn.h"
#include "KKCarMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogKKMovementReplicationComponent, All, All);

UKKMovementReplicationComponent::UKKMovementReplicationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicatedByDefault(true);
}

void UKKMovementReplicationComponent::BeginPlay()
{
    Super::BeginPlay();

    AKKCarPawn* const Car = GetOwner<AKKCarPawn>();
    if (!Car) return;

    CarMovementComponent = Car->FindComponentByClass<UKKCarMovementComponent>();
    MeshRootComponent = Car->GetMeshRootComponent();
}

void UKKMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CarMovementComponent) return;

    const FCarMove LastMove = CarMovementComponent->GetLastMove();
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
        case ROLE_AutonomousProxy: AutonomousProxy_OnRep_ServerState(); break;
        case ROLE_SimulatedProxy: SimulatedProxy_OnRep_ServerState(); break;
        default: break;
    }
}

void UKKMovementReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
    if (!CarMovementComponent) return;

    GetOwner()->SetActorTransform(ServerState.Transform);
    CarMovementComponent->SetVelocity(ServerState.Velocity);
    ClearAcknowledgedMoves(ServerState.LastMove);
    for (const FCarMove& UnacknowledgedMove : UnacknowledgedMoves)
    {
        CarMovementComponent->SimulateMove(UnacknowledgedMove);
    }
}

void UKKMovementReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
    TimeBetweenLastUpdates = TimeSinceUpdate;
    TimeSinceUpdate = 0;

    if (!GetOwner() || !CarMovementComponent || !MeshRootComponent) return;

    ClientStartTransform.SetLocation(MeshRootComponent->GetComponentLocation());
    ClientStartTransform.SetRotation(MeshRootComponent->GetComponentQuat());
    ClientStartVelocity = CarMovementComponent->GetVelocity();
    GetOwner()->SetActorTransform(ServerState.Transform);
}

void UKKMovementReplicationComponent::ClearAcknowledgedMoves(FCarMove LastMove)
{
    TArray<FCarMove> CurrentUnacknowledgedMoves;
    for (const FCarMove& Move : UnacknowledgedMoves)
    {
        if (Move.Time > LastMove.Time)
        {
            CurrentUnacknowledgedMoves.Add(Move);
        }
    }
    UnacknowledgedMoves = CurrentUnacknowledgedMoves;
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
    if (TimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;

    const float LerpRatio = TimeSinceUpdate / TimeBetweenLastUpdates;
    FHermiteCubicSpline Spline = FHermiteCubicSpline(ClientStartTransform.GetLocation(),  //
        ClientStartVelocity * GetVelocityToDerivative(),                                  //
        ServerState.Transform.GetLocation(),                                              //
        ServerState.Velocity * GetVelocityToDerivative());                                //
    InterpolateVelocity(Spline, LerpRatio);
    InterpolateLocation(Spline, LerpRatio);
    InterpolateRotation(LerpRatio);
}

float UKKMovementReplicationComponent::GetVelocityToDerivative()
{
    return TimeBetweenLastUpdates * 100.0f;
}

void UKKMovementReplicationComponent::InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio)
{
    if (!CarMovementComponent) return;

    const FVector NewDerivative = Spline.GetInterpolateDerivative(LerpRatio);
    const FVector NextVelocity = NewDerivative / GetVelocityToDerivative();
    CarMovementComponent->SetVelocity(NextVelocity);
}

void UKKMovementReplicationComponent::InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio)
{
    if (!MeshRootComponent) return;

    const FVector NextLocation = Spline.GetInterpolateLocation(LerpRatio);
    MeshRootComponent->SetWorldLocation(NextLocation);
}

void UKKMovementReplicationComponent::InterpolateRotation(float LerpRatio)
{
    if (!MeshRootComponent) return;

    const FQuat NextRotation = FQuat::Slerp(ClientStartTransform.GetRotation(), ServerState.Transform.GetRotation(), LerpRatio);
    MeshRootComponent->SetWorldRotation(NextRotation);
}
