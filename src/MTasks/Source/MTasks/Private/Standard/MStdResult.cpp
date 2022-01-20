// Fill out your copyright notice in the Description page of Project Settings.


#include "Standard/MStdResult.h"

UMStdResult* UMStdResult::Resolved(UObject* WorldContextObject)
{
	auto const Instance = NewObject<UMStdResult>(WorldContextObject);
	Instance->State = EMTaskState::Resolved;
	return Instance;
}

UMStdResult* UMStdResult::Rejected(UObject* WorldContextObject)
{
	auto const Instance = NewObject<UMStdResult>(WorldContextObject);
	Instance->State = EMTaskState::Rejected;
	return Instance;
}

EMTaskState UMStdResult::OnPoll_Implementation(float DeltaTime)
{
	return State;
}
