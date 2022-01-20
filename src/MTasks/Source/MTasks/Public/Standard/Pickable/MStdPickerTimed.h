// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MStdPicker.h"
#include "MTask.h"
#include "MStdPickerTimed.generated.h"

/**
 * Like standard picker but with an incremental picker mode.
 * ie. OnPicked is only invoked after 5 seconds spent 'picking'.
 * Use this, for example, for crafting or gathering, or any action-over-time.
 */
UCLASS(BlueprintType)
class MTASKS_API UMStdPickerTimed : public UMTask
{
	GENERATED_BODY()

public:
	// Blueprint API
	
	UPROPERTY(BlueprintAssignable, Category="MTasks|Standard|Pickable")
	FMStdPickTick OnTick;

	UPROPERTY(BlueprintReadOnly, Category="MTasks|Standard|Pickable")
	bool DidPickActor;

	UPROPERTY(BlueprintReadOnly, Category="MTasks|Standard|Pickable")
	AActor* PickedActor;

public:
	FInputActionBinding OnSelectPressedBinding;

	FInputActionBinding OnSelectReleasedBinding;
	
	FInputActionBinding OnCancelBinding;
	
	UPROPERTY()
	float PickAfterTime;

	UPROPERTY()
	float PickedTimeSoFar;
	
	UPROPERTY()
	float Elapsed;

	UPROPERTY()
	float Timeout;

	UPROPERTY()
	EMStdPickerState PickerState = EMStdPickerState::Idle;

	UPROPERTY()
	APlayerController* PlayerController = nullptr;

	ETraceTypeQuery TraceType;

	UPROPERTY()
	TSubclassOf<AActor> PickActorType;
	
public:
	/** Create a picker task to select an in-world object */
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard|Pickable", meta = (WorldContext = "WorldContextObject"))
	static UMStdPickerTimed* StdPickerTimed(UObject* WorldContextObject,
	                                        APlayerController* InPlayerController,
	                                        ETraceTypeQuery InTraceType,
	                                        FName SelectAction,
	                                        FName CancelAction,
	                                        float InTimeout,
	                                        float InPickAfterTime,
	                                        TSubclassOf<AActor> InPickActorOfType);

public:
	virtual EMTaskState OnPoll_Implementation(float DeltaTime) override;

private:
	/** Move back to the seeking state */
	void OnSelectReleased();

	/** Start picking this actor */
	void OnSelectPressed();

	/** Abort any attempt to select and reject this task */
	void OnCancelPressed();

	void ClearBindings() const;

	/** Picked an actual actor */
	void PickCurrent();

	/** Lock for something to focus on */
	void ScanForWorldActor();

	/** Failed to pick anything */
	void AbortPick();

};
