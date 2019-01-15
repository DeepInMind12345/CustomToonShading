// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SEditorUserWidgetHost.h"
#include "Engine/World.h"

void SEditorUserWidgetHost::Construct(const FArguments& InArgs, UWorld* InWorld)
{
	check(InWorld);
	World = InWorld;
	
	ChildSlot
	[
		SNullWidget::NullWidget
	];
}

void SEditorUserWidgetHost::AddReferencedObjects(FReferenceCollector& Collector)
{
}
