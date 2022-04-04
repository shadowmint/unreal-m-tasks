// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MTask.h"
#include "Camera/CameraComponent.h"
#include "UObject/Object.h"
#include "MStdPossess.generated.h"

/**
 * Possess another pawn; do so using some smoothing:
 * - lock the player input
 * - tween the camera to the camera location of the target
 * - possess the target
 */
UCLASS(BlueprintType)
class MTASKS_API UMStdPossess : public UMTask
{
	GENERATED_BODY()

	enum class StdPossessState
	{
		NotStarted,
		Running,
		Completed
	};

public:
	// Task config

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard")
	float OverSeconds;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard")
	bool LockPlayerInput;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard")
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="MTasks|Standard")
	APawn* TargetPawn;

private:
	// Internal state
	bool Validated;
	FVector OriginalPosition;
	FVector TargetPosition;
	TWeakObjectPtr<UCameraComponent> NewCamera;
	float Elapsed;

	StdPossessState InternalState;

public:
	// Actions

	/** Create a picker task to select an in-world object */
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard|Possess", meta = (WorldContext = "WorldContextObject"))
	static UMStdPossess* StdPossess(UObject* WorldContextObject,
	                                APlayerController* InPlayerController,
	                                APawn* InTargetPawn,
	                                float InOverSeconds,
	                                bool InLockPlayerInput);

	virtual EMTaskState OnPoll_Implementation(float DeltaTime) override;

	virtual void OnStart_Implementation(UObject* Context) override;

private:
	void StepCamera(float Amount);
};
