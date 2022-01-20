// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Editor.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Object.h"
#include "MTestUtils.generated.h"

UCLASS()
class MTASKSSAMPLE_API UMTestUtils : public UObject
{
	GENERATED_BODY()

public:
	static UWorld* GetAnyGameWorld()
	{
		if (GEditor && GEditor->GetWorldContexts().Num() && GEditor->GetWorldContexts()[0].World())
		{
			UE_LOG(LogTemp, Verbose, TEXT("Getting world from editor"))
			return GEditor->GetWorldContexts()[0].World();
		}
		if (GEngine && GEngine->GetWorldContexts().Num() && GEngine->GetWorldContexts()[0].World())
		{
			UE_LOG(LogTemp, Verbose, TEXT("Getting world from engine"))
			return GEngine->GetWorldContexts()[0].World();
		}
		if (GEditor)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Creating new world for editor"))
			return FAutomationEditorCommonUtils::CreateNewMap();
		}
		UE_LOG(LogTemp, Warning, TEXT("GEditor was not present, could not create World (map)"))
		return nullptr;
	}
};
