// Fill out your copyright notice in the Description page of Project Settings.


#include "Standard/Pickable/MStdPickerTimed.h"

#include "Standard/Pickable/MStdPickable.h"
#include "Standard/Pickable/MStdPicker.h"

UENUM(BlueprintType, Category="MTasks|Standard|Pickable")
enum class EMStdPickerTimedState : uint8
{
	Idle,
	PickSeek,
	// Seeking a target to pick
	PickStarted,
	// Collecting time towards the given pick
	Picked,
	PickCancelled
};

UMStdPickerTimed* UMStdPickerTimed::StdPickerTimed(UObject* WorldContextObject, APlayerController* InPlayerController, ETraceTypeQuery InTraceType,
                                                   FName SelectAction, FName CancelAction, float InTimeout, float InPickAfterTime,
                                                   TSubclassOf<AActor> InPickActorOfType)
{
	if (!InPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid PlayerController: null"));
		return nullptr;
	}

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid WorldContextObject: null"));
		return nullptr;
	}

	const auto Instance = NewObject<UMStdPickerTimed>(WorldContextObject);
	Instance->Elapsed = 0;
	Instance->Timeout = InTimeout < 0 ? 0 : InTimeout;
	Instance->PickerState = EMStdPickerState::Seeking;
	Instance->PlayerController = InPlayerController;
	Instance->TraceType = InTraceType;
	Instance->PickedTimeSoFar = 0;
	Instance->PickAfterTime = InPickAfterTime;

	Instance->OnSelectPressedBinding = FInputActionBinding(SelectAction, IE_Pressed);
	Instance->OnSelectPressedBinding.ActionDelegate.BindDelegate(Instance, &UMStdPickerTimed::OnSelectPressed);
	Instance->OnSelectPressedBinding = Instance->PlayerController->InputComponent->AddActionBinding(Instance->OnSelectPressedBinding);

	Instance->OnSelectReleasedBinding = FInputActionBinding(SelectAction, IE_Released);
	Instance->OnSelectReleasedBinding.ActionDelegate.BindDelegate(Instance, &UMStdPickerTimed::OnSelectReleased);
	Instance->OnSelectReleasedBinding = Instance->PlayerController->InputComponent->AddActionBinding(Instance->OnSelectPressedBinding);
	
	Instance->OnCancelBinding = FInputActionBinding(CancelAction, IE_Pressed);
	Instance->OnCancelBinding.ActionDelegate.BindDelegate(Instance, &UMStdPickerTimed::OnCancelPressed);
	Instance->OnCancelBinding = Instance->PlayerController->InputComponent->AddActionBinding(Instance->OnCancelBinding);

	Instance->PickedActor = nullptr;
	Instance->PickActorType = InPickActorOfType;

	return Instance;
}

EMTaskState UMStdPickerTimed::OnPoll_Implementation(float DeltaTime)
{
	if (State != EMTaskState::Running) return State;

	// If the pick was cancelled, abort
	if (PickerState == EMStdPickerState::PickCancelled)
	{
		PickedActor = nullptr;
		PlayerController = nullptr;
		return EMTaskState::Rejected;
	}

	// If the pick was completed, resolve
	if (PickerState == EMStdPickerState::Picked)
	{
		PlayerController = nullptr;
		IMStdPickable::Execute_OnPicked(PickedActor, 0);
		return EMTaskState::Resolved;
	}

	// If the pick is in progress, update the total
	if (PickerState == EMStdPickerState::PickStarted)
	{
		PickedTimeSoFar += DeltaTime;
		if (PickedTimeSoFar > PickAfterTime)
		{
			PickedTimeSoFar = PickAfterTime;
		}
		
		IMStdPickable::Execute_OnPicking(PickedActor, PickedTimeSoFar, PickAfterTime);

		// If we actually finished, wait until next frame to resolve
		if (PickedTimeSoFar >= PickAfterTime)
		{
			PickerState = EMStdPickerState::Picked;
			ClearBindings();
			return State;
		}
	}

	Elapsed += DeltaTime;
	if (Elapsed > Timeout)
	{
		AbortPick();
		return State; // Resolve next tick
	}

	// If we're still looking, raycast for actors
	ScanForWorldActor();
	return State;
}

void UMStdPickerTimed::OnSelectReleased()
{
	PickerState = EMStdPickerState::Seeking;
	PickedTimeSoFar = 0;
}


void UMStdPickerTimed::OnSelectPressed()
{
	if (PickedActor)
	{
		PickCurrent();
	}
}

void UMStdPickerTimed::OnCancelPressed()
{
	AbortPick();
}

void UMStdPickerTimed::ScanForWorldActor()
{
	FHitResult Result;
	auto DidMatch = false;
	if (PlayerController->GetHitResultUnderCursorByChannel(TraceType, true, Result))
	{
		const auto Other = Result.Actor.Get();
		const auto IsCorrectType = Other->GetClass()->IsChildOf(PickActorType);
		if (IsCorrectType && IMStdPickable::ImplementedBy(Other))
		{
			const bool CanAcceptPick = IMStdPickable::Execute_CanBeSelected(Other, this);
			if (CanAcceptPick)
			{
				PickedActor = Other;
				IMStdPickable::Execute_OnHighlight(PickedActor, true);
				DidMatch = true;
			}
		}
		OnTick.Broadcast(true, DidMatch, Result.Location, Result.Normal);
	}
	else
	{
		OnTick.Broadcast(false, false, Result.Location, Result.Normal);
	}
	if (!DidMatch)
	{
		PickedActor = nullptr;
	}
}

void UMStdPickerTimed::ClearBindings() const
{
	if (!PlayerController) return;
	PlayerController->InputComponent->RemoveActionBindingForHandle(OnSelectPressedBinding.GetHandle());
	PlayerController->InputComponent->RemoveActionBindingForHandle(OnSelectReleasedBinding.GetHandle());
	PlayerController->InputComponent->RemoveActionBindingForHandle(OnCancelBinding.GetHandle());
}

void UMStdPickerTimed::PickCurrent()
{
	PickerState = EMStdPickerState::PickStarted;
}

void UMStdPickerTimed::AbortPick()
{
	PickerState = EMStdPickerState::PickCancelled;
	ClearBindings();
}
