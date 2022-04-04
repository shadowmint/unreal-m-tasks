// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTask.h"
#include "UObject/Object.h"
#include "MCommand.generated.h"

class UMCommand;
DECLARE_MULTICAST_DELEGATE_OneParam(FCommandUpdate, UMCommand*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommandUpdateBP, UMCommand*, Command);

/**
 * A command is like a command in that it is a long running operation; however, a command
 * cannot timeout. Commands continue to run until they are resolved or cancelled.
 *
 * Like what? For example, an AI might have a long running 'follow patrol path' command
 * until it is disturbed (cancelled) for a new behaviour.
 *
 * A game might have a 'tick manager' that updates the players health and state every tick.
 *
 * You could create an actor for these, but a command does the job without polluting the actor
 * hierarchy. 
 */
UCLASS(Abstract, BlueprintAble, BlueprintType)
class MTASKS_API UMCommand : public UObject
{
	GENERATED_BODY()

public:
	/** Invoked when the promise is updated */
	UPROPERTY(BlueprintAssignable, Category = "MTasks")
	FCommandUpdateBP OnUpdate;

	FCommandUpdate Update;

	/** The promise state */
	UPROPERTY(BlueprintReadOnly, Category = "MTasks")
	EMTaskState State;

	/** The parent of this command, if any */
	UPROPERTY()
	TWeakObjectPtr<UMTask> Parent = nullptr;

public:
	// Public API

	/** Attach this to a specific executor and start running it */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	void Start(UMTaskExecutor* Executor, UObject* Context = nullptr);

	/** Cancel or abort this task */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	void Cancel(UMTaskExecutor* Executor);

	/** Is this command still un-started? ie. Idle or Waiting */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	FORCEINLINE bool IsPending()
	{
		return State == EMTaskState::Waiting || State == EMTaskState::Idle;
	}

	/** Is this command finished? ie. Rejected or Resolved */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	bool IsCompleted()
	{
		return State == EMTaskState::Rejected || State == EMTaskState::Resolved;
	}

	/** Is this command running? */
	UFUNCTION(BlueprintCallable, Category = "MTasks")
	bool IsRunning()
	{
		return State == EMTaskState::Running;
	}

public:
	// Abstract API

	/**
	 * Invoked when the command enters its `Running` state.
	 * If this command had a parent, the parent is supplied.
	 **/
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	void OnStart(UObject* Context);

	/**
	 * Invoked when the command is completed; regardless of the state.
	 * Use this to do a 'final' cleanup for things.
	 **/
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	void OnEnd();

	/** Poll this command to update the state  */
	UFUNCTION(BlueprintNativeEvent, Category = "MTasks")
	EMTaskState OnPoll(float DeltaTime);
};
