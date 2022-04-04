// Fill out your copyright notice in the Description page of Project Settings.


#include "Standard/MStdPossess.h"

UMStdPossess* UMStdPossess::StdPossess(UObject* WorldContextObject, APlayerController* InPlayerController,
                                       APawn* InTargetPawn, float InOverSeconds, bool LockPlayerInput)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMStdPossess: Invalid world context object"));
		return nullptr;
	}

	const auto Instance = NewObject<UMStdPossess>(WorldContextObject);
	Instance->OverSeconds = InOverSeconds;
	Instance->PlayerController = InPlayerController;
	Instance->TargetPawn = InTargetPawn;
	Instance->LockPlayerInput = LockPlayerInput;
	Instance->Validated = false;

	return Instance;
}

EMTaskState UMStdPossess::OnPoll_Implementation(float DeltaTime)
{
	if (!Validated)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMStdPossess: Invalid arguments"));
		return EMTaskState::Rejected;
	}

	if (InternalState == StdPossessState::NotStarted)
	{
		if (LockPlayerInput)
		{
			// Lock player input	
		}

		auto OriginalTarget = PlayerController->PlayerCameraManager->GetViewTarget();
		auto OriginalCamera = Cast<UCameraComponent>(
			OriginalTarget->GetComponentByClass(UCameraComponent::StaticClass()));
		if (!OriginalCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("UMStdPossess: Controller did not have an original camera"))
			return EMTaskState::Rejected;
		}

		PlayerController->Possess(TargetPawn);

		auto NewTarget = PlayerController->PlayerCameraManager->GetViewTarget();
		NewCamera = Cast<UCameraComponent>(NewTarget->GetComponentByClass(UCameraComponent::StaticClass()));
		if (!NewCamera.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UMStdPossess: Pawn to possess did not have a camera"))
			return EMTaskState::Rejected;
		}

		auto NewRelative = NewCamera->GetRelativeLocation();
		auto OriginPoint = OriginalCamera->GetComponentLocation();
		NewCamera->SetWorldLocation(OriginPoint);
		auto OldRelative = NewCamera->GetRelativeLocation();

		OriginalPosition = OldRelative;
		TargetPosition = NewRelative;

		InternalState = StdPossessState::Running;

		UE_LOG(LogTemp, Display, TEXT("UMStdPossess: Tween camera (%s) -> (%s)"), *OldRelative.ToString(), *NewRelative.ToString());
	}

	if (InternalState == StdPossessState::Running)
	{
		Elapsed += DeltaTime;
		if (Elapsed >= OverSeconds)
		{
			auto Step = 1;
			StepCamera(Step);

			InternalState = StdPossessState::Completed;
			if (LockPlayerInput)
			{
				// Release player input
			}
			return EMTaskState::Resolved;
		}

		auto Step = Elapsed / OverSeconds;
		StepCamera(Step);
	}

	return State;
}

void UMStdPossess::OnStart_Implementation(UObject* Context)
{
	if (OverSeconds < 0)
	{
		OverSeconds = 0;
	}

	// Check args aren't totally stupid
	if (!PlayerController || !TargetPawn)
	{
		return;
	}

	Elapsed = 0;
	InternalState = StdPossessState::NotStarted;

	Validated = true;
}

void UMStdPossess::StepCamera(float Amount)
{
	auto Delta = (TargetPosition - OriginalPosition) * Amount;
	if (NewCamera.IsValid())
	{
		auto NextPos = OriginalPosition + Delta;
		// UE_LOG(LogTemp, Display, TEXT("UMStdPossess::StepCamera: %f -> %s"), Amount, *NextPos.ToString());
		NewCamera->SetRelativeLocation(NextPos);
	}
}
