// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MExecutor.h"
#include "GameFramework/Actor.h"
#include "MStdExecutor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MTASKS_API AMStdExecutor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category="MTasks|Standard")
	FMTaskExecutorPolicy Policy = FMTaskExecutorPolicy();
	
	UPROPERTY(BlueprintReadOnly, Category="MTasks|Standard")
	UMTaskExecutor *Executor = nullptr;

public:
	AMStdExecutor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Create a standard executor on this level and process tasks on it every tick */
	UFUNCTION(BlueprintCallable, Category="MTasks|Standard", meta=(WorldContext="WorldContextObject"))
	static UMTaskExecutor *GetStdExecutor(UObject *WorldContextObject);

	/** Policy spawner if none is set */
	static FMTaskExecutorPolicy DefaultExecutionPolicy();
	
private:
	/** Setup post-create with all the internal details */
	void Initialize();
};
