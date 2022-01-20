// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MStdPickable.h"
#include "MTask.h"
#include "UObject/Object.h"
#include "MStdPicker.generated.h"

/** Invoked every tick so the caller can render a 3d cursor */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMStdPickTick, bool, HitResult, bool, IsValidTarget, FVector, Location, FVector, Normal);

UENUM(BlueprintType, Category="MTasks|Standard|Pickable")
enum class EMStdPickerState : uint8
{
	Idle,
	Seeking,
	Picked,
	PickCancelled,
	PickStarted,
};

UCLASS(BlueprintType)
class MTASKS_API UMStdPicker : public UMTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="MTasks|Standard|Pickable")
	FMStdPickTick OnTick;

	UPROPERTY(BlueprintReadOnly, Category="MTasks|Standard|Pickable")
	bool DidPickActor;

	UPROPERTY(BlueprintReadOnly, Category="MTasks|Standard|Pickable")
	AActor* PickedActor;

	UPROPERTY()
	EMStdPickerState PickerState = EMStdPickerState::Idle;

	UPROPERTY()
	APlayerController* PlayerController = nullptr;

	UPROPERTY()
	float Elapsed;

	UPROPERTY()
	float Timeout;

	ETraceTypeQuery TraceType;

	FInputActionBinding OnSelectBinding;

	FInputActionBinding OnCancelBinding;

	UPROPERTY()
	TSubclassOf<AActor> PickActorType;

public:
	/** Create a picker task to select an in-world object */
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard|Pickable", meta = (WorldContext = "WorldContextObject"))
	static UMStdPicker* StdPicker(UObject* WorldContextObject,
	                              APlayerController* InPlayerController,
	                              ETraceTypeQuery InTraceType,
	                              FName SelectAction,
	                              FName CancelAction,
	                              float InTimeout,
	                              TSubclassOf<AActor> InPickActorOfType);

public:
	virtual EMTaskState OnPoll_Implementation(float DeltaTime) override;

private:
	void OnSelectPressed();

	void OnCancelPressed();

	void ClearBindings() const;

	/** Picked an actual actor */
	void PickCurrent();

	/** Lock for something to focus on */
	void ScanForWorldActor();

	/** Failed to pick anything */
	void AbortPick();
};
