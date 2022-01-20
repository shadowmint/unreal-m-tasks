// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MStdExecutor.h"

namespace AMStdExecutorInternals
{
	/** The world we're currently bound to */
	MTASKS_API UWorld* World;

	/** Singleton instance per level */
	MTASKS_API TWeakObjectPtr<AMStdExecutor> Instance;
}

AMStdExecutor::AMStdExecutor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMStdExecutor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Executor->Tick(DeltaTime);
}

UMTaskExecutor* AMStdExecutor::GetStdExecutor(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMStdExecutor::GetStdExecutor: invalid context object"));
		return nullptr;
	}

	const auto World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMStdExecutor::GetStdExecutor: No world associated with context object"));
		return nullptr;
	}

	// Reset if we changed worlds
	if (AMStdExecutorInternals::World != World)
	{
		AMStdExecutorInternals::Instance = nullptr;
	}

	if (AMStdExecutorInternals::Instance.IsValid())
	{
		return AMStdExecutorInternals::Instance.Get()->Executor;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const auto InInstance = World->SpawnActor<AMStdExecutor>(Params);

	InInstance->Initialize();
	AMStdExecutorInternals::Instance = InInstance;
	AMStdExecutorInternals::World = World;

	return InInstance->Executor;
}

FMTaskExecutorPolicy AMStdExecutor::DefaultExecutionPolicy()
{
	FMTaskExecutorPolicy InPolicy;
	InPolicy.UseCustomPolicy = true;
	InPolicy.MaxExecutionDuration = 60;
	return InPolicy;
}

void AMStdExecutor::Initialize()
{
	Executor = NewObject<UMTaskExecutor>(this);

	// If the policy was not defined, spawn a standard policy.
	if (!Policy.UseCustomPolicy)
	{
		UE_LOG(LogTemp, Display, TEXT("AMStdExecutor::Initialize: No execution policy was specified, using default"))
		Policy = DefaultExecutionPolicy();
	}

	Executor->Initialize(Policy, true);
}
