// Fill out your copyright notice in the Description page of Project Settings.

#include "KKCarPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "KKUtils.h"
#include "Net/UnrealNetwork.h"

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

    NetUpdateFrequency = 1.0f;
}

void AKKCarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    FVector Force = GetActorForwardVector() * DrivingForce * Throttle;
    Force += GetAirResistance();
    Force += GetRollingResistance();

    Acceleration = Force / Weight;
    Velocity = Velocity + Acceleration * DeltaTime;

    UpdateRotation(DeltaTime);
	UpdatePositionFromVelocity(DeltaTime);

    if (HasAuthority())
    {
        ReplicationTransform = GetActorTransform();
    }

    DrawDebugString(GetWorld(), FVector(0.0f, 0.0f, 100.0f), KKUtils::GetEnumRoleString(GetLocalRole()), this, FColor::White, DeltaTime);
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

void AKKCarPawn::UpdateRotation(float DeltaTime)
{
    float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
    float RotationAngle = DeltaLocation / TurningRadius * SteeringThrow;
    FQuat Rotation(GetActorUpVector(), RotationAngle);
    AddActorLocalRotation(Rotation);
    Velocity = Rotation.RotateVector(Velocity);
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
    Server_MoveForward(Amount);
}

void AKKCarPawn::MoveRight(float Amount)
{
    SteeringThrow = Amount;
    Server_MoveRight(Amount);
}

void AKKCarPawn::Server_MoveForward_Implementation(float Amount)
{
    Throttle = Amount;
}

bool AKKCarPawn::Server_MoveForward_Validate(float Amount)
{
    return FMath::Abs(Amount) <= 1;
}

void AKKCarPawn::Server_MoveRight_Implementation(float Amount)
{
    SteeringThrow = Amount;
}

bool AKKCarPawn::Server_MoveRight_Validate(float Amount)
{
    return FMath::Abs(Amount) <= 1;
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

void AKKCarPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AKKCarPawn, ReplicationTransform);
    DOREPLIFETIME(AKKCarPawn, Velocity);
    DOREPLIFETIME(AKKCarPawn, SteeringThrow);
    DOREPLIFETIME(AKKCarPawn, Throttle);
}

void AKKCarPawn::Rep_ReplicatedTransform()
{
    SetActorTransform(ReplicationTransform);
}
