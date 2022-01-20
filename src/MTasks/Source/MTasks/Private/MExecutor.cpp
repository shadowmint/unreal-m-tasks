// Fill out your copyright notice in the Description page of Project Settings.


#include "MExecutor.h"

void UMTaskExecutor::SetActive(bool InActive)
{
	IsActive = InActive;
}

void UMTaskExecutor::Run(UMTask* Task, UObject* TaskContext, UMTask* TaskParent, bool StartNow)
{
	if (Task->State != EMTaskState::Idle && Task->State != EMTaskState::Waiting)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMTaskExecutor: Invalid attempt to run an invalid task"));
		return;
	}

	// Save to the current frame unless specified otherwise
	if (StartNow)
	{
		auto& CurrentTasks = GetCurrent();
		CurrentTasks.Add(FMTaskExecutorManagedTask(Task, TaskContext));
	}
	else
	{
		auto& NextTasks = GetNext();
		NextTasks.Add(FMTaskExecutorManagedTask(Task, TaskContext));
	}

	// Start
	Task->State = EMTaskState::Running;
	Task->OnStart(TaskContext, TaskParent);

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Started: %s"), ElapsedTicks, *Task->GetFullName());
	}
}

void UMTaskExecutor::SetDebug(bool InVerboseLogging)
{
	VerboseLogging = InVerboseLogging;
}

TArray<FMTaskExecutorManagedTask>& UMTaskExecutor::GetCurrent()
{
	return Current == UMTaskExecutorState::A ? A : B;
}

TArray<FMTaskExecutorManagedTask>& UMTaskExecutor::GetNext()
{
	return Current == UMTaskExecutorState::A ? B : A;
}

void UMTaskExecutor::ApplyExecutionPolicy(const FMTaskExecutorManagedTask& Task) const
{
	if (Task.ExecutionDuration > Policy.MaxExecutionDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Expired MTask which exceeded maximum execution duration: %s"), *Task.Task->GetFullName())
		Task.Task->State = EMTaskState::Rejected;
	}
}

bool UMTaskExecutor::ProcessTask(FMTaskExecutorManagedTask& Task, float DeltaTime) const
{
	auto const PreviousState = Task.Task->State;
	Task.ExecutionDuration += DeltaTime;
	Task.Task->State = Task.Task->OnPoll(DeltaTime);

	if (VerboseLogging)
	{
		if (PreviousState != Task.Task->State)
		{
			UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: %s -> %s: %s"),
			       ElapsedTicks,
			       *UEnum::GetValueAsString(PreviousState),
			       *UEnum::GetValueAsString(Task.Task->State),
			       *Task.Task->GetFullName());
		}
	}

	// Any custom per-task configurable logic.
	ApplyExecutionPolicy(Task);

	Task.Completed = Task.Task->State != EMTaskState::Running;
	return !Task.Completed; // ie. Return true if still running.
}

void UMTaskExecutor::ProcessCompletedTask(const FMTaskExecutorManagedTask& Task)
{
	// Dispatch events
	if (Task.Task->Update.IsBound())
	{
		Task.Task->Update.Broadcast(Task.Task);
		Task.Task->Update.Clear();
	}
	if (Task.Task->OnUpdate.IsBound())
	{
		Task.Task->OnUpdate.Broadcast(Task.Task);
		Task.Task->OnUpdate.Clear();
	}

	// Run child tasks
	for (const auto& ChildTask : Task.Task->Children)
	{
		if (ChildTask.Type == Task.Task->State)
		{
			Run(ChildTask.Child, Task.TaskContext, Task.Task, false);
		}
	}

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: %s: %s"), ElapsedTicks, *UEnum::GetValueAsString(Task.Task->State),
		       *Task.Task->GetFullName());
	}
}

void UMTaskExecutor::ProcessTasks(float DeltaTime)
{
	auto CurrentTasks = GetCurrent();
	auto& NextTasks = GetNext();
	
	// Process existing tasks
	for (auto& T : CurrentTasks)
	{
		if (VerboseLogging)
		{
			UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Process: %s"), ElapsedTicks, *T.Task->GetFullName());
		}
		if (ProcessTask(T, DeltaTime))
		{
			NextTasks.Add(T);
		}
		else
		{
			ProcessCompletedTask(T);
		}
	}

	// Clear the current tasks before we rotate the 'next' into 'current'.
	auto &CurrentTasksRef = GetCurrent();
	CurrentTasksRef.Reset();
	
	// Next tick
	Current = Current == UMTaskExecutorState::A ? UMTaskExecutorState::B : UMTaskExecutorState::A;
}

void UMTaskExecutor::Initialize(FMTaskExecutorPolicy InPolicy, bool InActive)
{
	Policy = InPolicy;
	SetActive(InActive);
}

void UMTaskExecutor::Tick(float DeltaTime)
{
	if (!IsActive) return;
	ProcessTasks(DeltaTime);
	ElapsedTicks += 1;
}
