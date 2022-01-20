// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "UObject/Object.h"
#include "MTasksSampleCircle.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MTASKSSAMPLE_API UMTasksSampleCircle : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InnerRadius;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Progress;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D Offset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AngleOffset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Thickness;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Radius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Nodes;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSlateBrush Brush;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor Color;
	
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
