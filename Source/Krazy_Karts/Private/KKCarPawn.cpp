// Fill out your copyright notice in the Description page of Project Settings.

#include "KKCarPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogKKCarPawn, All, All);

AKKCarPawn::AKKCarPawn()
{
	PrimaryActorTick.bCanEverTick = true;

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
}

void AKKCarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    FVector Force = GetActorForwardVector() * DrivingForce * Throttle;
    Force += GetResistance();
    Acceleration = Force / Weight;
    Velocity = Velocity + Acceleration * DeltaTime;

    UpdateRotation(DeltaTime);
	UpdatePositionFromVelocity(DeltaTime);
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
    float DegreesToRotate = Degrees * SteeringThrow * DeltaTime;
    FQuat Rotation(GetActorUpVector(), FMath::DegreesToRadians(DegreesToRotate));
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
}

void AKKCarPawn::MoveRight(float Amount)
{
    SteeringThrow = Amount;
}

FVector AKKCarPawn::GetResistance()
{
    return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}
