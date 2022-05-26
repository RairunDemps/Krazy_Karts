#pragma once

#include "KKCoreTypes.generated.h"

USTRUCT()
struct FCarMove
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    float Throttle;

    UPROPERTY()
    float SteeringThrow;

    UPROPERTY()
    float DeltaTime;

    UPROPERTY()
    float Time;
};

USTRUCT()
struct FCarState
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    FVector Velocity;

    UPROPERTY()
    FCarMove LastMove;
};
