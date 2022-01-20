// Fill out your copyright notice in the Description page of Project Settings.

#include "Standard/MStdDelay.h"

UMStdDelay* UMStdDelay::StdDelay(UObject* WorldContextObject, int Seconds, int Ticks)
{
	auto const Instance = NewObject<UMStdDelay>(WorldContextObject);
	Instance->WaitSeconds = Seconds;
	Instance->WaitTicks = Ticks;
	Instance->ElapsedSeconds = 0;
	Instance->ElapsedTicks = 0;
	return Instance;
}

EMTaskState UMStdDelay::OnPoll_Implementation(float DeltaTime)
{
	if (State != EMTaskState::Running) return State;

	if (WaitTicks < 0 && WaitSeconds < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMStdDelay: Invalid delay time; you must set WaitTicks or WaitSeconds"));
		return EMTaskState::Rejected;
	}

	ElapsedSeconds += DeltaTime;
	ElapsedTicks += 1;

	if (WaitTicks != -1 && ElapsedTicks >= WaitTicks)
	{
		return EMTaskState::Resolved;
	}

	if (WaitSeconds != -1 && ElapsedSeconds >= WaitSeconds)
	{
		return EMTaskState::Resolved;
	}

	return EMTaskState::Running;
}
