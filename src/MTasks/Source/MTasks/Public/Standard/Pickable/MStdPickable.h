// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MStdPickable.generated.h"

class UMStdPicker;

UINTERFACE(Blueprintable, BlueprintType)
class UMStdPickable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this to make an actor pickable in the world.
 */
class MTASKS_API IMStdPickable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MTasks|Standard|Pickable")
	bool CanBeSelected(UObject *ByPicker);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MTasks|Standard|Pickable")
	void OnHighlight(bool HasFocus);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MTasks|Standard|Pickable")
	void OnPicking(float Elapsed, float Total);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MTasks|Standard|Pickable")
	void OnPicked(float TimeSpentPicking);

public:
	/**
	 * Usage:
	 *   if (IMCardPickable::ImplementedBy(Foo)
	 *     IMCardPickable::Execute_CanBeSelected(Foo);
	 */
	static bool ImplementedBy(UObject *Target);
};
