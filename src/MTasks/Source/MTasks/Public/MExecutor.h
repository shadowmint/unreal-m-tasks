// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MCommand.h"
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

	FMTaskExecutorManagedTask(UMTask* InTask, UObject* InTaskContext)
	{
		Completed = false;
		ExecutionDuration = 0;
		Task = InTask;
		TaskContext = InTaskContext;
	}
};

USTRUCT()
struct MTASKS_API FMTaskExecutorManagedCommand
{
	GENERATED_BODY()

	UPROPERTY()
	bool Completed;

	UPROPERTY()
	float ExecutionDuration;

	UPROPERTY()
	UMCommand* Command = nullptr;

	/** Save here so it can be passed to child tasks */
	UPROPERTY()
	UObject* TaskContext = nullptr;

	FMTaskExecutorManagedCommand()
	{
		Completed = true;
		ExecutionDuration = 0;
		Command = nullptr;
		TaskContext = nullptr;
	}

	FMTaskExecutorManagedCommand(UMCommand* InCommand, UObject* InTaskContext)
	{
		Completed = false;
		ExecutionDuration = 0;
		Command = InCommand;
		TaskContext = InTaskContext;
	}
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

	UPROPERTY()
	TArray<FMTaskExecutorManagedTask> RunningTasks;

	UPROPERTY()
	TArray<FMTaskExecutorManagedTask> PendingTasks;

	UPROPERTY()
	TArray<FMTaskExecutorManagedCommand> RunningCommands;

	UPROPERTY()
	TArray<FMTaskExecutorManagedCommand> PendingCommands;
	
	// If this is true, new tasks must go into 'next' not 'current' or they will get lost
	// due to being in the middle of a processing loop.
	bool TaskCompletionInProgress;

public:
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void Initialize(FMTaskExecutorPolicy InPolicy, bool InActive);

	/** Update and process tasks; call this from a tickable object */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void Tick(float DeltaTime);

	/** Start and stop this executor */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void SetActive(bool InActive);

	UFUNCTION(BlueprintCallable, Category="MTasks")
	void SetDebug(bool InVerboseLogging);

	/**
	 * Manage this task until it completes or fails.
	 * Otherwise, this is an invalid operation.
	 */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void RunTask(UMTask* Task, UObject* TaskContext);

	/**
	 * Manage this command until it completes or fails.
	 * Otherwise, this is an invalid operation.
	 */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void RunCommand(UMCommand* Command, UObject* TaskContext);

	/** Cancel an active task */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void CancelTask(UMTask* Task);
	
	/** Cancel an active command */
	UFUNCTION(BlueprintCallable, Category="MTasks")
	void CancelCommand(UMCommand* Command);

private:
	/** Apply execution policy rules like timeout after taking too long or whatever for tasks */
	void ApplyExecutionPolicy(const FMTaskExecutorManagedTask& Task) const;

	/** Apply execution policy rules like timeout after taking too long or whatever for commands */
	void ApplyExecutionPolicy(const FMTaskExecutorManagedCommand& Cmd) const;

	/**
	 * Process a single tick on a task.
	 * Returns true if the task is still running.
	 **/
	bool ProcessTask(FMTaskExecutorManagedTask& Task, float DeltaTime) const;

	/** Process a task which has fully resolved */
	void ProcessCompletedTask(const FMTaskExecutorManagedTask& Task);

	/** Process all tasks which are currently active */
	void ProcessTasks(float DeltaTime);

	/**
	 * Process a single tick on a command.
	 * Returns true if the command is still running.
	 **/
	bool ProcessCommand(FMTaskExecutorManagedCommand& Cmd, float DeltaTime) const;

	/** Process a command which has fully resolved */
	void ProcessCompletedCommand(const FMTaskExecutorManagedCommand& Cmd);
	
	/** Process all commands which are currently active */
	void ProcessCommands(float DeltaTime);

	/**
	 * When a task is set to run, we actually need to run the first task in that chain.
	 * This way a task source can return the 'final' task for event handling, and running
	 * that task runs the *first* task in that sequence.
	 *
	 * We recurse upwards until either we hit the first unresolved parent (if any), or
	 * we hit max iterations, which probably means there is a cycle in the task chain.
	 */
	UMTask* FindFirstUnresolvedParent(UMTask* Task, int MaxIterations = 256) const;

	/** Filter used to prune completed tasks */
	static bool IsTaskCompletedPredicate(const FMTaskExecutorManagedTask& Task);

	/** Filter used to prune completed commands */
	static bool IsCommandCompletedPredicate(const FMTaskExecutorManagedCommand& Cmd);
};
