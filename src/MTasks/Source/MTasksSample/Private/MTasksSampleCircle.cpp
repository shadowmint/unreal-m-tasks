// Fill out your copyright notice in the Description page of Project Settings.


#include "MTasksSampleCircle.h"

int32 UMTasksSampleCircle::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                                       FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                                       bool bParentEnabled) const
{
	auto InDrawEffects = ESlateDrawEffect::None;

	const auto Geom = AllottedGeometry.ToPaintGeometry(FVector2D(Radius * 2, Radius * 2), FSlateLayoutTransform(1.0f, Offset));

	TArray<FVector2D> Points;
	constexpr auto MaxAngle = PI * 2;
	auto const EffectiveProgress = Progress > Nodes ? Nodes : (Progress < 0 ? 0 : Progress);

	// Ring
	Points.Reset();
	for (auto i = 0; i <= EffectiveProgress; i++)
	{
		const auto Partial = AngleOffset + MaxAngle * (i / static_cast<float>(Nodes));
		auto const X = FMath::Sin(Partial) * Radius;
		auto const Y = FMath::Cos(Partial) * Radius;
		Points.Add(FVector2D(X, Y));
	}
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, Geom, Points, InDrawEffects, Color, true, Thickness);

	// Internal
	if (EffectiveProgress > 0)
	{
		for (auto i = 0; i <= EffectiveProgress; i++)
		{
			const auto Partial = AngleOffset + MaxAngle * (i / static_cast<float>(Nodes));
			auto const X = FMath::Sin(Partial) * Radius;
			auto const Y = FMath::Cos(Partial) * Radius;
			auto const Sx = FMath::Sin(Partial) * InnerRadius;
			auto const Sy = FMath::Cos(Partial) * InnerRadius;

			Points.Reset();
			Points.Add(FVector2D(Sx, Sy));
			Points.Add(FVector2D(X, Y));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, Geom, Points, InDrawEffects, Color, true, Thickness);
		}
	}

	return LayerId;
}
