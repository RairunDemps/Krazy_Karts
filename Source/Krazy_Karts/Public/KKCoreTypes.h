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

struct FHermiteCubicSpline
{
    FVector StartLocation;
    FVector StartDerivative;
    FVector TargetLocation;
    FVector TargetDerivative;

    FHermiteCubicSpline() {};
    
    FHermiteCubicSpline(FVector StartLocationValue, FVector StartDerivativeValue, FVector TargetLocationValue, FVector TargetDerivativeValue)
    {
        StartLocation = StartLocationValue;
        StartDerivative = StartDerivativeValue;
        TargetLocation = TargetLocationValue;
        TargetDerivative = TargetDerivativeValue;
    };

    FVector GetInterpolateDerivative(float LerpRatio) const
    {
        return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    }

    FVector GetInterpolateLocation(float LerpRatio) const
    {
        return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    }
};
