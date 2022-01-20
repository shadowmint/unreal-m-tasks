// Fill out your copyright notice in the Description page of Project Settings.


#include "Standard/Pickable/MStdPickable.h"


bool IMStdPickable::ImplementedBy(UObject* Target)
{
	return Target && Target->Implements<UMStdPickable>();
}
