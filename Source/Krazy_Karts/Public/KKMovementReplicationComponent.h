// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KKCoreTypes.h"
#include "KKMovementReplicationComponent.generated.h"

class UKKCarMovementComponent;

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
	
    UFUNCTION()
    void OnRep_ServerState();

    void ClearAcknowledgedMoves(FCarMove LastMove);

    UKKCarMovementComponent* CarMovementComponent;

    void SetCarMovementComponent();

    void UpdateServerState(FCarMove Move);
};
