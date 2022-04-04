// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MTask.generated.h"

class UMTaskExecutor;

class UMTask;
DECLARE_MULTICAST_DELEGATE_OneParam(FTaskUpdate, UMTask*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTaskUpdateBP, UMTask*, Task);

UENUM(BlueprintType)
enum class EMTaskState : uint8
{
	/** The task has not started yet */
	Idle,

	/** The task is waiting for some other task to complete before it starts. */
	Waiting,

	/** The task is busy running. */
	Running,

	/** The task completed successfully */
	Resolved,

	/** The task failed and was rejected */
	Rejected,
};

USTRUCT()
struct MTASKS_API FMTaskChild
{
	GENERATED_BODY()

	UPROPERTY()
	EMTaskState Type;

	UPROPERTY()
	TWeakObjectPtr<UMTask> Child;

	FMTaskChild()
	{
		Type = EMTaskState::Idle;
		Child = nullptr;
	}
	
	FMTaskChild(EMTaskState InType, UMTask *InChild)
	{
		Type = InType;
		Child = InChild;
	}
};

/**
 * Create a subclass of this for concrete promise types.
 * Generics are not supported by blueprint, so templates can't be used here.
 *
 * Task has a fixed life cycle:
 *
 * - Idle (default state)
 *   -> Running, when assigned to an executor
 *   -> Waiting, when assigned to a parent task
 *
 * - Running:
 *   -> Rejected, when the task marks itself as rejected from OnPoll()
 *   -> Resolved, when the task marks itself as resolved from OnPoll()
 *
 * - Waiting:
 *   -> Rejected, when the parent task is rejected
 *   -> Running, when the parent task is resolved
 *   
 * - Rejected:
 *   - There are no possible state transitions from rejected.
 *
 * - Resolved:
 *   - There are no possible state transitions from rejected.
 *
 * A task can have any number of child tasks which can be added using
 * `Then`.
 *
 * A task broadcasts its state update when it moves into Resolved or
 * Rejected using the OnUpdate delegate.
 *
 * If a task has a parent, it is passed to the OnStart and the implementation
 * must decide what to do with it.
 *
 * If a task has multiple children, they are executed in parallel, not in
 * sequence; but single in a single threaded context; ie. one call to OnPoll
 * per task per tick.
 *
 * Finally, when a task is resolved, it is discarded by the executor. That means
 * if events are bound to a completed or rejected task, they will never be
 * dispatched.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MTASKS_API UMTask : public UObject
{
	GENERATED_BODY()

public:
	/** Invoked when the promise is updated */
	UPROPERTY(BlueprintAssignable, Category = "MTasks")
	FTaskUpdateBP OnUpdate;
	
	FTaskUpdate Update;

	/** The promise state */
	UPROPERTY(BlueprintReadOnly, Category = "MTasks")
	EMTaskState State;

	/** The parent of this task, if any */
	UPROPERTY()
	TWeakObjectPtr<UMTask> Parent = nullptr;
	
	/** Children of this task */
	TArray<FMTaskChild> Children;
	
public:
	// Public API
	
	/** Attach this to a specific executor and start running it */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	void Start(UMTaskExecutor* Executor, UObject *Context = nullptr);

	/** Abort this task */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	void Cancel(UMTaskExecutor* Executor);
	
	/** Add a child task to run when this one is completed */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	UMTask *Then(EMTaskState OnState, UMTask *Child);

	/** Is this task still un-started? ie. Idle or Waiting */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	FORCEINLINE bool IsPending()
	{
		return State == EMTaskState::Waiting || State == EMTaskState::Idle;
	}
	
	/** Is this task finished? ie. Rejected or Resolved */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	bool IsCompleted() {
	return State == EMTaskState::Rejected || State == EMTaskState::Resolved;
	}

	/** Is this task running? */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	bool IsRunning()
	{
		return State == EMTaskState::Running;
	}
	
public:
	// Abstract API

	/**
	 * Invoked when the task enters its `Running` state.
	 * If this task had a parent, the parent is supplied.
	 **/
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	void OnStart(UObject *Context);

	/**
	 * Invoked when the command is completed; regardless of the state.
	 * Use this to do a 'final' cleanup for things.
	 **/
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	void OnEnd();
	
	/** Poll this task to update the state  */
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	EMTaskState OnPoll(float DeltaTime);
};
