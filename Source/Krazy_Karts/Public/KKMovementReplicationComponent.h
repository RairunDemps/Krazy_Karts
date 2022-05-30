// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KKCoreTypes.h"
#include "KKMovementReplicationComponent.generated.h"

class UKKCarMovementComponent;
class USceneComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZY_KARTS_API UKKMovementReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKKMovementReplicationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendMove(FCarMove Move);

protected:
	virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_ServerState)
    FCarState ServerState;

    TArray<FCarMove> UnacknowledgedMoves;

    UKKCarMovementComponent* CarMovementComponent;
    USceneComponent* MeshRootComponent;
	
    float TimeSinceUpdate;
    float TimeBetweenLastUpdates;
    FTransform ClientStartTransform;
    FVector ClientStartVelocity;

    UFUNCTION()
    void OnRep_ServerState();
    
    void AutonomousProxy_OnRep_ServerState();
    void SimulatedProxy_OnRep_ServerState();

    void ClearAcknowledgedMoves(FCarMove LastMove);

    void UpdateServerState(FCarMove Move);

    void ClientTick(float DeltaTime);
    void InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio);
    void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio);
    void InterpolateRotation(float LerpRatio);
    float GetVelocityToDerivative();
};
