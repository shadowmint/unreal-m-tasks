// Fill out your copyright notice in the Description page of Project Settings.


#include "Standard/Pickable/MStdPicker.h"
#include "Actors/MStdExecutor.h"
#include "Standard/Pickable/MStdPickable.h"

UMStdPicker* UMStdPicker::StdPicker(UObject* WorldContextObject, APlayerController* InPlayerController,
                                    ETraceTypeQuery InTraceType,
                                    FName SelectAction, FName CancelAction, float InTimeout,
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

	const auto Instance = NewObject<UMStdPicker>(WorldContextObject);
	Instance->Elapsed = 0;
	Instance->Timeout = InTimeout < 0 ? 0 : InTimeout;
	Instance->PickerState = EMStdPickerState::Seeking;
	Instance->PlayerController = InPlayerController;
	Instance->TraceType = InTraceType;

	Instance->OnSelectBinding = FInputActionBinding(SelectAction, IE_Pressed);
	Instance->OnSelectBinding.ActionDelegate.BindDelegate(Instance, &UMStdPicker::OnSelectPressed);
	Instance->OnSelectBinding = Instance->PlayerController->InputComponent->AddActionBinding(Instance->OnSelectBinding);

	Instance->OnCancelBinding = FInputActionBinding(CancelAction, IE_Pressed);
	Instance->OnCancelBinding.ActionDelegate.BindDelegate(Instance, &UMStdPicker::OnCancelPressed);
	Instance->OnCancelBinding = Instance->PlayerController->InputComponent->AddActionBinding(Instance->OnCancelBinding);

	Instance->PickedActor = nullptr;
	Instance->PickActorType = InPickActorOfType;

	return Instance;
}

EMTaskState UMStdPicker::OnPoll_Implementation(float DeltaTime)
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

void UMStdPicker::OnSelectPressed()
{
	if (PickedActor)
	{
		PickCurrent();
	}
}

void UMStdPicker::OnCancelPressed()
{
	AbortPick();
}

void UMStdPicker::ScanForWorldActor()
{
	FHitResult Result;
	auto DidMatch = false;
	if (PlayerController->GetHitResultUnderCursorByChannel(TraceType, true, Result))
	{
		const auto Other = Result.GetActor();
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

void UMStdPicker::ClearBindings() const
{
	if (!PlayerController) return;
	PlayerController->InputComponent->RemoveActionBindingForHandle(OnSelectBinding.GetHandle());
	PlayerController->InputComponent->RemoveActionBindingForHandle(OnCancelBinding.GetHandle());
}

void UMStdPicker::PickCurrent()
{
	PickerState = EMStdPickerState::Picked;
	ClearBindings();
}

void UMStdPicker::AbortPick()
{
	PickerState = EMStdPickerState::PickCancelled;
	ClearBindings();
}
