// Fill out your copyright notice in the Description page of Project Settings.

#include "KKCarPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "KKUtils.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"

DEFINE_LOG_CATEGORY_STATIC(LogKKCarPawn, All, All);

AKKCarPawn::AKKCarPawn()
{
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
    SetRootComponent(BoxCollisionComponent);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SkeletalMeshComponent->SetupAttachment(RootComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SprintArmComponent"));
    SpringArmComponent->TargetOffset = FVector(0.0f, 0.0f, 200.0f);
    SpringArmComponent->TargetArmLength = 600.0f;
    SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
}

void AKKCarPawn::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        NetUpdateFrequency = 1.0f;
    }
}

void AKKCarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (GetLocalRole() == ROLE_AutonomousProxy)
    {
        FCarMove Move = CreateMove(DeltaTime);
        SimulateMove(Move);
        UnacknowledgedMoves.Add(Move);
        Server_SendMove(Move);
    }

    if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
    {
        FCarMove Move = CreateMove(DeltaTime);
        Server_SendMove(Move);
    }

    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        SimulateMove(ServerState.LastMove);
    }

    DrawDebugString(GetWorld(), FVector(0.0f, 0.0f, 100.0f), KKUtils::GetEnumRoleString(GetLocalRole()), this, FColor::White, DeltaTime);
}

void AKKCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AKKCarPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AKKCarPawn::MoveRight);
}

void AKKCarPawn::MoveForward(float Amount)
{
    Throttle = Amount;
}

void AKKCarPawn::MoveRight(float Amount)
{
    SteeringThrow = Amount;
}

void AKKCarPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AKKCarPawn, ServerState);
}

void AKKCarPawn::Server_SendMove_Implementation(FCarMove Move)
{
    SimulateMove(Move);

    ServerState.LastMove = Move;
    ServerState.Transform = GetActorTransform();
    ServerState.Velocity = Velocity;
}

bool AKKCarPawn::Server_SendMove_Validate(FCarMove Move)
{
    return true;
}

void AKKCarPawn::OnRep_ServerState()
{
    SetActorTransform(ServerState.Transform);
    Velocity = ServerState.Velocity;

    ClearAcknowledgedMoves(ServerState.LastMove);

    for (const auto& UnacknowledgedMove : UnacknowledgedMoves)
    {
        SimulateMove(UnacknowledgedMove);
    }
}

void AKKCarPawn::SimulateMove(const FCarMove& Move)
{
    FVector Force = GetActorForwardVector() * DrivingForce * Move.Throttle;
    Force += GetAirResistance();
    Force += GetRollingResistance();

    FVector Acceleration = Force / Weight;
    Velocity = Velocity + Acceleration * Move.DeltaTime;

    UpdateRotation(Move.DeltaTime, Move.SteeringThrow);
    UpdatePositionFromVelocity(Move.DeltaTime);
}

FCarMove AKKCarPawn::CreateMove(float DeltaTime)
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

void AKKCarPawn::ClearAcknowledgedMoves(FCarMove LastMove)
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

void AKKCarPawn::UpdatePositionFromVelocity(float DeltaTime)
{
    FVector Translation = Velocity * Multiplier * DeltaTime;

    FHitResult HitResult;
    AddActorWorldOffset(Translation, true, &HitResult);
    if (HitResult.IsValidBlockingHit())
    {
        Velocity = FVector::ZeroVector;
    }
}

void AKKCarPawn::UpdateRotation(float DeltaTime, float MoveSteeringThrow)
{
    float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
    float RotationAngle = DeltaLocation / TurningRadius * MoveSteeringThrow;
    FQuat Rotation(GetActorUpVector(), RotationAngle);
    AddActorLocalRotation(Rotation);
    Velocity = Rotation.RotateVector(Velocity);
}

FVector AKKCarPawn::GetAirResistance()
{
    return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AKKCarPawn::GetRollingResistance()
{
    if (!GetWorld()) return FVector::ZeroVector;

    float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
    float NormalForce = Weight * AccelerationDueToGravity;

    return -Velocity.GetSafeNormal() * RollingCoefficient * NormalForce;
}
