// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SlateUpdatableBuffer.h"
#include "RenderingThread.h"

DECLARE_CYCLE_STAT(TEXT("UpdateInstanceBuffer Time"), STAT_SlateUpdateInstanceBuffer, STATGROUP_Slate);

FSlateUpdatableInstanceBuffer::FSlateUpdatableInstanceBuffer( int32 InitialInstanceCount )
	: FreeBufferIndex(0)
{
	InstanceBufferResource.Init(InitialInstanceCount);
	for( int32 BufferIndex = 0; BufferIndex < SlateRHIConstants::NumBuffers; ++BufferIndex )
	{
		BufferData[BufferIndex].Reserve( InitialInstanceCount );
	}

}

FSlateUpdatableInstanceBuffer::~FSlateUpdatableInstanceBuffer()
{
	InstanceBufferResource.Destroy();
	FlushRenderingCommands();
}

void FSlateUpdatableInstanceBuffer::BindStreamSource(FRHICommandListImmediate& RHICmdList, int32 StreamIndex, uint32 InstanceOffset)
{
	RHICmdList.SetStreamSource(StreamIndex, InstanceBufferResource.VertexBufferRHI, InstanceOffset*sizeof(FVector4));
}

TSharedPtr<class FSlateInstanceBufferUpdate> FSlateUpdatableInstanceBuffer::BeginUpdate()
{
	return MakeShareable(new FSlateInstanceBufferUpdate(*this));
}

uint32 FSlateUpdatableInstanceBuffer::GetNumInstances() const
{
	return NumInstances;
}

void FSlateUpdatableInstanceBuffer::UpdateRenderingData(int32 NumInstancesToUse)
{
	NumInstances = NumInstancesToUse;
	if (NumInstances > 0)
	{
		// Enqueue a command to unlock the draw buffer after all windows have been drawn
		FSlateUpdatableInstanceBuffer* Self = this;
		int32 BufferIndex = FreeBufferIndex;
		ENQUEUE_RENDER_COMMAND(SlateBeginDrawingWindowsCommand)(
			[Self, BufferIndex](FRHICommandListImmediate& RHICmdList)
			{
				Self->UpdateRenderingData_RenderThread(RHICmdList, BufferIndex);
			});
	
		FreeBufferIndex = (FreeBufferIndex + 1) % SlateRHIConstants::NumBuffers;
	}
}

TArray<FVector4>& FSlateUpdatableInstanceBuffer::GetBufferData()
{
	return BufferData[FreeBufferIndex];
}

void FSlateUpdatableInstanceBuffer::UpdateRenderingData_RenderThread(FRHICommandListImmediate& RHICmdList, int32 BufferIndex)
{
	SCOPE_CYCLE_COUNTER( STAT_SlateUpdateInstanceBuffer );

	const TArray<FVector4>& RenderThreadBufferData = BufferData[BufferIndex];
	InstanceBufferResource.PreFillBuffer( RenderThreadBufferData.Num(), false );

	if(!IsRunningRHIInSeparateThread() || RHICmdList.Bypass())
	{
		uint8* InstanceBufferData = (uint8*)InstanceBufferResource.LockBuffer_RenderThread(RenderThreadBufferData.Num());

		FMemory::Memcpy(InstanceBufferData, RenderThreadBufferData.GetData(), RenderThreadBufferData.Num()*sizeof(FVector4) );
	
		InstanceBufferResource.UnlockBuffer_RenderThread();
	}
	else
	{
		FVertexBufferRHIParamRef VertexBufferRHI = InstanceBufferResource.VertexBufferRHI;
		RHICmdList.EnqueueLambda([VertexBufferRHI, &InstanceData = RenderThreadBufferData](FRHICommandListImmediate& RHICmdList)
		{
			SCOPE_CYCLE_COUNTER(STAT_SlateUpdateInstanceBuffer);

			int32 RequiredVertexBufferSize = InstanceData.Num() * sizeof(FVector4);
			uint8* InstanceBufferData = (uint8*)RHICmdList.LockVertexBuffer(VertexBufferRHI, 0, RequiredVertexBufferSize, RLM_WriteOnly);

			FMemory::Memcpy(InstanceBufferData, InstanceData.GetData(), InstanceData.Num() * sizeof(FVector4));

			RHICmdList.UnlockVertexBuffer(VertexBufferRHI);
		});

		RHICmdList.RHIThreadFence(true);
	}
}

