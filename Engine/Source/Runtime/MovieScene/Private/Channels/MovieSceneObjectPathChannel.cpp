// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Channels/MovieSceneObjectPathChannel.h"

bool FMovieSceneObjectPathChannelKeyValue::SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot)
{
	if (Tag.Type == NAME_SoftObjectProperty)
	{
		FSoftObjectPtr OldProperty;
		Slot << OldProperty;

		SoftPtr = OldProperty.ToSoftObjectPath();
		if (OldProperty.ToSoftObjectPath().GetSubPathString().Len() == 0)
		{
			// Forcibly load the old property so we can store it as a hard reference, only if it was not referencing an actor or other sub object
			HardPtr = OldProperty.LoadSynchronous();
		}
		return true;
	}
	return false;
}

FMovieSceneObjectPathChannelKeyValue& FMovieSceneObjectPathChannelKeyValue::operator=(UObject* NewObject)
{
	HardPtr = NewObject;
	SoftPtr = NewObject;
	return *this;
}

UObject* FMovieSceneObjectPathChannelKeyValue::Get() const
{
	if (!HardPtr && !SoftPtr.IsNull())
	{
		HardPtr = SoftPtr.Get();
		if (!HardPtr)
		{
			HardPtr = SoftPtr.LoadSynchronous();
		}
	}

	return HardPtr;
}

bool FMovieSceneObjectPathChannel::Evaluate(FFrameTime InTime, UObject*& OutValue) const
{
	if (Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(Times, InTime.FrameNumber)-1);
		OutValue = Values[Index].Get();
		return true;
	}
	else if (!DefaultValue.GetSoftPtr().IsNull())
	{
		OutValue = DefaultValue.Get();
		return true;
	}

	return false;
}

void FMovieSceneObjectPathChannel::GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FMovieSceneObjectPathChannel::GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
}

void FMovieSceneObjectPathChannel::SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FMovieSceneObjectPathChannel::DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FMovieSceneObjectPathChannel::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	GetData().DeleteKeys(InHandles);
}

void FMovieSceneObjectPathChannel::ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FMovieSceneObjectPathChannel::ComputeEffectiveRange() const 
{
	return GetData().GetTotalRange();
}

int32 FMovieSceneObjectPathChannel::GetNumKeys() const 
{
	return Times.Num();
}

void FMovieSceneObjectPathChannel::Reset() 
{
	return GetData().Reset();
}

void FMovieSceneObjectPathChannel::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

void FMovieSceneObjectPathChannel::Optimize(const FKeyDataOptimizationParams& InParameters)
{}

void FMovieSceneObjectPathChannel::ClearDefault() 
{
	RemoveDefault();
}

