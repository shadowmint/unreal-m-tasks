// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTask.h"
#include "UObject/Object.h"
#include "MExecutor.generated.h"

USTRUCT(BlueprintType)
struct MTASKS_API FMTaskExecutorPolicy
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks")
	bool UseCustomPolicy;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks")
	float MaxExecutionDuration;

	FMTaskExecutorPolicy()
	{
		UseCustomPolicy = false;
		MaxExecutionDuration = 0;
	}
};

USTRUCT()
struct MTASKS_API FMTaskExecutorManagedTask
{
	GENERATED_BODY()

	UPROPERTY()
	bool Completed;

	UPROPERTY()
	float ExecutionDuration;

	UPROPERTY()
	UMTask* Task = nullptr;

	/** Save here so it can be passed to child tasks */
	UPROPERTY()
	UObject* TaskContext = nullptr;

	FMTaskExecutorManagedTask()
	{
		Completed = true;
		ExecutionDuration = 0;
		Task = nullptr;
		TaskContext = nullptr;
	}

	FMTaskExecutorManagedTask(UMTask* InTask, UObject *InTaskContext)
	{
		Completed = false;
		ExecutionDuration = 0;
		Task = InTask;
		TaskContext = InTaskContext;
	}
};

enum class UMTaskExecutorState : uint8
{
	A,
	B
};

/**
 * The executor is a single top level process for running tasks.
 */
UCLASS(Blueprintable, BlueprintType)
class MTASKS_API UMTaskExecutor : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks")
	FMTaskExecutorPolicy Policy;

private:
	bool IsActive;

	bool VerboseLogging;

	long ElapsedTicks;
	
	UMTaskExecutorState Current;

	UPROPERTY()
	TArray<FMTaskExecutorManagedTask> A;

	UPROPERTY()
	TArray<FMTaskExecutorManagedTask> B;

public:
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void Initialize(FMTaskExecutorPolicy InPolicy, bool InActive);

	/** Update and process tasks; call this from a tickable object */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void Tick(float DeltaTime);

	/** Start and stop this executor */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void SetActive(bool InActive);

	/**
	 * Manage this task until it completes or fails.
	 * Adding a task sets it to running and starts it if it is idle or waiting.
	 * Otherwise, this is an invalid operation.
	 */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void Run(UMTask* Task, UObject* TaskContext, UMTask* TaskParent, bool StartNow = true);

	UFUNCTION(BlueprintCallable, Category="MTasks")
	void SetDebug(bool InVerboseLogging);

private:
	/** Return the tasks from last tick */
	TArray<FMTaskExecutorManagedTask>& GetCurrent();

	/** Return the tasks entry for next tick */
	TArray<FMTaskExecutorManagedTask>& GetNext();

	/** Apply execution policy rules like timeout after taking too long or whatever */
	void ApplyExecutionPolicy(const FMTaskExecutorManagedTask& Task) const;

	/** Process a task and return true if it will be active next tick */
	bool ProcessTask(FMTaskExecutorManagedTask& Task, float DeltaTime) const;

	/** Process a task which has fully resolved */
	void ProcessCompletedTask(const FMTaskExecutorManagedTask& Task);

	/** Process all tasks which are currently active */
	void ProcessTasks(float DeltaTime);
};
