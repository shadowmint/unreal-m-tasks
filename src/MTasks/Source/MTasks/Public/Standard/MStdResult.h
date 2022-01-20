// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTask.h"
#include "UObject/Object.h"
#include "MStdResult.generated.h"

/** Returns the fixed state it is spawned with */
UCLASS(BlueprintType)
class MTASKS_API UMStdResult : public UMTask
{
	GENERATED_BODY()

private:
	EMTaskState State;

public:
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard", meta=(WorldContext="WorldContextObject"))
	static UMStdResult* Resolved(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="MTasks|Standard", meta=(WorldContext="WorldContextObject"))
	static UMStdResult* Rejected(UObject* WorldContextObject);
	
	virtual EMTaskState OnPoll_Implementation(float DeltaTime) override;
};
