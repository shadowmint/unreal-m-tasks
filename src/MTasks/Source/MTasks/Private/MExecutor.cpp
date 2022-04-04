// Fill out your copyright notice in the Description page of Project Settings.


#include "MExecutor.h"

void UMTaskExecutor::SetActive(bool InActive)
{
	IsActive = InActive;
}

UMTask* UMTaskExecutor::FindFirstUnresolvedParent(UMTask* Task, int MaxIterations) const
{
	auto EffectiveTask = Task;

	// Move up the chain of parents as far as we can
	auto Iteration = 0;
	while (EffectiveTask->Parent.IsValid() && EffectiveTask->Parent.Get()->IsPending())
	{
		EffectiveTask = EffectiveTask->Parent.Get();
		Iteration += 1;
		if (Iteration > MaxIterations)
		{
			UE_LOG(LogTemp, Warning, TEXT("UMTaskExecutor: Hit max iterations find parent. Maybe cycle in %s"), *Task->GetName());
			return nullptr;
		}
	}

	if (Task != EffectiveTask && VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: Using root task %s parent of %s"), *EffectiveTask->GetName(), *Task->GetName());
	}

	return EffectiveTask;
}

void UMTaskExecutor::RunTask(UMTask* Task, UObject* TaskContext)
{
	Task = this->FindFirstUnresolvedParent(Task);
	if (!Task)
	{
		return;
	}

	if (Task->State != EMTaskState::Idle && Task->State != EMTaskState::Waiting)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMTaskExecutor: Invalid attempt to run an invalid task"));
		return;
	}

	// Add to the pending tasks queue.
	PendingTasks.Add(FMTaskExecutorManagedTask(Task, TaskContext));

	// Start
	Task->State = EMTaskState::Running;
	Task->OnStart(TaskContext);

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Started: %s"), ElapsedTicks, *Task->GetName());
	}
}

void UMTaskExecutor::RunCommand(UMCommand* Command, UObject* TaskContext)
{
	if (Command->State != EMTaskState::Idle && Command->State != EMTaskState::Waiting)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMTaskExecutor::Run: Invalid attempt to run an invalid command"));
		return;
	}

	// Add to the pending tasks queue.
	PendingCommands.Add(FMTaskExecutorManagedCommand(Command, TaskContext));

	// Start
	Command->State = EMTaskState::Running;
	Command->OnStart(TaskContext);

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Started: %s"), ElapsedTicks, *Command->GetName());
	}
}

void UMTaskExecutor::CancelTask(UMTask* Task)
{
	Task->State = EMTaskState::Rejected;
	for (auto& TaskRef : RunningTasks)
	{
		if (TaskRef.Task == Task)
		{
			TaskRef.Completed = true;
			return;
		}
	}
	for (auto& TaskRef : PendingTasks)
	{
		if (TaskRef.Task == Task)
		{
			TaskRef.Completed = true;
			return;
		}
	}
}

void UMTaskExecutor::CancelCommand(UMCommand* Command)
{
	Command->State = EMTaskState::Rejected;
	for (auto& Cmd : RunningCommands)
	{
		if (Cmd.Command == Command)
		{
			Cmd.Completed = true;
			return;
		}
	}
	for (auto& Cmd : PendingCommands)
	{
		if (Cmd.Command == Command)
		{
			Cmd.Completed = true;
			return;
		}
	}
}

void UMTaskExecutor::SetDebug(bool InVerboseLogging)
{
	VerboseLogging = InVerboseLogging;
}

void UMTaskExecutor::ApplyExecutionPolicy(const FMTaskExecutorManagedTask& Task) const
{
	if (Task.ExecutionDuration > Policy.MaxExecutionDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Expired MTask which exceeded maximum execution duration: %s"), *Task.Task->GetName())
		Task.Task->State = EMTaskState::Rejected;
	}
}

void UMTaskExecutor::ApplyExecutionPolicy(const FMTaskExecutorManagedCommand& Cmd) const
{
	// Maybe in the future we'll need this.
}

bool UMTaskExecutor::ProcessTask(FMTaskExecutorManagedTask& Task, float DeltaTime) const
{
	if (Task.Completed) return false;
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
			       *Task.Task->GetName());
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
	TaskCompletionInProgress = true;
	Task.Task->OnEnd();
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

	// Remove parent to prevent memory leaks from circular refs
	Task.Task->Parent = nullptr;

	// Run child tasks
	for (const auto& ChildTask : Task.Task->Children)
	{
		if (ChildTask.Type == Task.Task->State)
		{
			if (ChildTask.Child.IsValid())
			{
				RunTask(ChildTask.Child.Get(), Task.TaskContext);
			}
		}
	}

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: %s: %s"), ElapsedTicks, *UEnum::GetValueAsString(Task.Task->State),
		       *Task.Task->GetName());
	}

	TaskCompletionInProgress = false;
}

void UMTaskExecutor::ProcessTasks(float DeltaTime)
{
	// Add any pending tasks to the running tasks
	RunningTasks.Append(PendingTasks);
	PendingTasks.Reset();

	// Process existing tasks
	for (auto& T : RunningTasks)
	{
		if (VerboseLogging)
		{
			if (T.ExecutionDuration == 0)
			{
				UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Process: %s"), ElapsedTicks, *T.Task->GetName());
			}
		}
		if (!ProcessTask(T, DeltaTime))
		{
			ProcessCompletedTask(T);
		}
	}

	// Prune any completed tasks
	RunningTasks.RemoveAll(UMTaskExecutor::IsTaskCompletedPredicate);
}

bool UMTaskExecutor::ProcessCommand(FMTaskExecutorManagedCommand& Cmd, float DeltaTime) const
{
	if (Cmd.Completed) return false;
	auto const PreviousState = Cmd.Command->State;
	Cmd.ExecutionDuration += DeltaTime;
	Cmd.Command->State = Cmd.Command->OnPoll(DeltaTime);

	if (VerboseLogging)
	{
		if (PreviousState != Cmd.Command->State)
		{
			UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: %s -> %s: %s"),
			       ElapsedTicks,
			       *UEnum::GetValueAsString(PreviousState),
			       *UEnum::GetValueAsString(Cmd.Command->State),
			       *Cmd.Command->GetName());
		}
	}

	// Any custom per-task configurable logic.
	ApplyExecutionPolicy(Cmd);

	Cmd.Completed = Cmd.Command->State != EMTaskState::Running;
	return !Cmd.Completed; // ie. Return true if still running.
}

void UMTaskExecutor::ProcessCompletedCommand(const FMTaskExecutorManagedCommand& Cmd)
{
	// Dispatch events
	TaskCompletionInProgress = true;
	Cmd.Command->OnEnd();
	if (Cmd.Command->Update.IsBound())
	{
		Cmd.Command->Update.Broadcast(Cmd.Command);
		Cmd.Command->Update.Clear();
	}
	if (Cmd.Command->OnUpdate.IsBound())
	{
		Cmd.Command->OnUpdate.Broadcast(Cmd.Command);
		Cmd.Command->OnUpdate.Clear();
	}

	// Remove parent to prevent memory leaks from circular refs
	Cmd.Command->Parent = nullptr;

	if (VerboseLogging)
	{
		UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: %s: %s"), ElapsedTicks, *UEnum::GetValueAsString(Cmd.Command->State),
		       *Cmd.Command->GetName());
	}

	TaskCompletionInProgress = false;
}

void UMTaskExecutor::ProcessCommands(float DeltaTime)
{
	// Add any pending commands to the running commands
	RunningCommands.Append(PendingCommands);
	PendingCommands.Reset();

	// Process existing tasks
	for (auto& C : RunningCommands)
	{
		if (VerboseLogging)
		{
			if (C.ExecutionDuration == 0)
			{
				UE_LOG(LogTemp, Display, TEXT("UMTaskExecutor: %d: Process: %s"), ElapsedTicks, *C.Command->GetName());
			}
		}
		if (!ProcessCommand(C, DeltaTime))
		{
			ProcessCompletedCommand(C);
		}
	}

	// Prune any completed tasks
	RunningCommands.RemoveAll(UMTaskExecutor::IsCommandCompletedPredicate);
}

bool UMTaskExecutor::IsTaskCompletedPredicate(const FMTaskExecutorManagedTask& Task)
{
	return Task.Completed;
}

bool UMTaskExecutor::IsCommandCompletedPredicate(const FMTaskExecutorManagedCommand& Cmd)
{
	return Cmd.Completed;
}

void UMTaskExecutor::Initialize(FMTaskExecutorPolicy InPolicy, bool InActive)
{
	Policy = InPolicy;
	SetActive(InActive);
	TaskCompletionInProgress = false;
}

void UMTaskExecutor::Tick(float DeltaTime)
{
	if (!IsActive) return;
	ProcessTasks(DeltaTime);
	ProcessCommands(DeltaTime);
	ElapsedTicks += 1;
}
