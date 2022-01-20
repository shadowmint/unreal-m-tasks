// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTask.h"
#include "MStdDelay.generated.h"

UCLASS(BlueprintType)
class MTASKS_API UMStdDelay : public UMTask
{
	GENERATED_BODY()

private:
	int ElapsedTicks = -1;

	float ElapsedSeconds = -1;

public:
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard", meta=(WorldContext="WorldContextObject"))
	static UMStdDelay* StdDelay(UObject* WorldContextObject, int Seconds, int Ticks);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard Tasks")
	int WaitTicks;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard Tasks")
	float WaitSeconds;

	virtual EMTaskState OnPoll_Implementation(float DeltaTime) override;
};
