// Fill out your copyright notice in the Description page of Project Settings.

#include "MTask.h"
#include "MExecutor.h"

EMTaskState UMTask::OnPoll_Implementation(float DeltaTime)
{
	// Default action; just immediately resolve.
	return EMTaskState::Resolved;
}

void UMTask::OnStart_Implementation(UObject* Context)
{
	// Default action; do nothing.
}

void UMTask::Start(UMTaskExecutor* Executor, UObject* Context)
{
	Executor->RunTask(this, Context);
}

void UMTask::Cancel(UMTaskExecutor* Executor)
{
	Executor->CancelTask(this);
}

UMTask* UMTask::Then(EMTaskState OnState, UMTask* Child)
{
	Children.Add(FMTaskChild(OnState, Child));
	Child->Parent = this;
	return this;
}

void UMTask::OnEnd_Implementation()
{
}
