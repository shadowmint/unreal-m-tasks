// Fill out your copyright notice in the Description page of Project Settings.


#include "MCommand.h"
#include "MExecutor.h"

void UMCommand::Start(UMTaskExecutor* Executor, UObject* Context)
{
	Executor->RunCommand(this, Context);
}

void UMCommand::Cancel(UMTaskExecutor* Executor)
{
	Executor->CancelCommand(this);
}

void UMCommand::OnEnd_Implementation()
{
}

EMTaskState UMCommand::OnPoll_Implementation(float DeltaTime)
{
	// Default action; just immediately resolve.
	return EMTaskState::Resolved;
}

void UMCommand::OnStart_Implementation(UObject* Context)
{
	// Default action; do nothing.
}
