// Fill out your copyright notice in the Description page of Project Settings.

#include "KKCarPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "KKCarMovementComponent.h"
#include "KKMovementReplicationComponent.h"

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

    CarMovementComponent = CreateDefaultSubobject<UKKCarMovementComponent>(TEXT("CarMovementComponent"));
    
    MovementReplicationComponent = CreateDefaultSubobject<UKKMovementReplicationComponent>(TEXT("MovementReplicationComponent"));
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
        FCarMove Move = CarMovementComponent->CreateMove(DeltaTime);
        CarMovementComponent->SimulateMove(Move);
        MovementReplicationComponent->AddUnacknowledgedMove(Move);
        MovementReplicationComponent->Server_SendMove(Move);
    }

    if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
    {
        FCarMove Move = CarMovementComponent->CreateMove(DeltaTime);
        MovementReplicationComponent->Server_SendMove(Move);
    }

    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        FCarMove LastMove = MovementReplicationComponent->GetServerState().LastMove;
        CarMovementComponent->SimulateMove(LastMove);
    }

    FString RoleString;
    UEnum::GetValueAsString(GetLocalRole(), RoleString);
    DrawDebugString(GetWorld(), FVector(0.0f, 0.0f, 100.0f), RoleString, this, FColor::White, DeltaTime);
}

void AKKCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AKKCarPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AKKCarPawn::MoveRight);
}

void AKKCarPawn::MoveForward(float Amount)
{
    CarMovementComponent->SetThrottle(Amount);
}

void AKKCarPawn::MoveRight(float Amount)
{
    CarMovementComponent->SetSteeringThrow(Amount);
}
