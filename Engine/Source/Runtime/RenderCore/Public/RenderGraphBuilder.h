// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "RenderGraphResources.h"
#include "ShaderParameterMacros.h"
#include "RendererInterface.h"


/** Whether render graph debugging is compiled. */
#define RENDER_GRAPH_DEBUGGING (DO_CHECK)

/** Whether render graph should support draw events or not.
 * RENDER_GRAPH_DRAW_EVENTS == 0 means there is no string processing at all.
 * RENDER_GRAPH_DRAW_EVENTS == 1 means const TCHAR* is only passdown.
 * RENDER_GRAPH_DRAW_EVENTS == 2 means complex formated FString is passdown.
 */
#if WITH_PROFILEGPU
	#define RENDER_GRAPH_DRAW_EVENTS 2
#else
	#define RENDER_GRAPH_DRAW_EVENTS 0
#endif


/** Opaque object to store a draw event. */
class RENDERCORE_API FRDGEventName
{
public:
	inline FRDGEventName() 
	{ }

	#if RENDER_GRAPH_DRAW_EVENTS == 2

	explicit FRDGEventName(const TCHAR* EventFormat, ...);

	#elif RENDER_GRAPH_DRAW_EVENTS == 1

	explicit inline FRDGEventName(const TCHAR* EventFormat)
		: EventName(EventFormat)
	{ }

	#endif

	const TCHAR* GetTCHAR() const
	{
		#if RENDER_GRAPH_DRAW_EVENTS == 2
			return *EventName;
		#elif RENDER_GRAPH_DRAW_EVENTS == 1
			return EventName;
		#else
			return TEXT("UnknownRDVEvent");
		#endif
	}

private:
	#if RENDER_GRAPH_DRAW_EVENTS == 2
		FString EventName;
	#elif RENDER_GRAPH_DRAW_EVENTS == 1
		const TCHAR* EventName;
	#endif
};


/** Hierarchical scope for draw events of passes. */
class RENDERCORE_API FRDGEventScope
{
private:
	// Pointer towards this one is contained in.
	const FRDGEventScope* const ParentScope;

	// Name of the event.
	const FRDGEventName Name;


	FRDGEventScope(const FRDGEventScope* InParentScope, FRDGEventName&& InName)
		: ParentScope(InParentScope), Name(InName)
	{ }


	friend class FRDGBuilder;
	friend class FStackRDGEventScopeRef;
};


/** Flags to anotate passes. */
enum class ERenderGraphPassFlags
{
	None = 0,

	/** Pass uses compute only */
	Compute = 1 << 0,
};

ENUM_CLASS_FLAGS(ERenderGraphPassFlags)


// TODO(RDG): remove from global scope?
struct RENDERCORE_API FShaderParameterStructRef
{
	const void*						Contents;
	const FRHIUniformBufferLayout*	Layout;

	template<typename MemberType>
	MemberType* GetMemberPtrAtOffset(uint16 Offset) const
	{
		return reinterpret_cast<MemberType*>(((uint8*)Contents) + Offset);
	}
};


/** 
 * Base class of a render graph pass
 */
struct RENDERCORE_API FRenderGraphPass
{
	FRenderGraphPass(FRDGEventName&& InName, const FRDGEventScope* InParentScope, FShaderParameterStructRef InParameterStruct, ERenderGraphPassFlags InPassFlags)
		: Name(static_cast<FRDGEventName&&>(InName))
		, ParentScope(InParentScope)
		, ParameterStruct(InParameterStruct)
		, PassFlags(InPassFlags)
	{
		if (IsCompute())
		{
			ensureMsgf(ParameterStruct.Layout->NumRenderTargets() == 0, TEXT("Pass %s was declared as ERenderGraphPassFlags::Compute yet has RenderTargets in its ResourceTable"), GetName());
		}
	}

	virtual ~FRenderGraphPass() {}

	virtual void Execute(FRHICommandListImmediate& RHICmdList) const = 0;

	const TCHAR* GetName() const {
		return Name.GetTCHAR();
	}

	const ERenderGraphPassFlags& GetFlags() const
	{
		return PassFlags;
	}

	bool IsCompute() const {
		return (PassFlags & ERenderGraphPassFlags::Compute) == ERenderGraphPassFlags::Compute;
	}
	
	FShaderParameterStructRef GetParameters() const { return ParameterStruct; }

protected:
	const FRDGEventName Name;
	const FRDGEventScope* const ParentScope;
	const FShaderParameterStructRef ParameterStruct;
	const ERenderGraphPassFlags PassFlags;

	friend class FRDGBuilder;
};

/** 
 * Render graph pass with lambda execute function
 */
template <typename ParameterStructType, typename ExecuteLambdaType>
struct TLambdaRenderPass final : public FRenderGraphPass
{
	TLambdaRenderPass(FRDGEventName&& InName, const FRDGEventScope* InParentScope, FShaderParameterStructRef InParameterStruct, ERenderGraphPassFlags InPassFlags, ExecuteLambdaType&& InExecuteLambda)
		: FRenderGraphPass(static_cast<FRDGEventName&&>(InName), InParentScope, InParameterStruct, InPassFlags)
		, ExecuteLambda(static_cast<ExecuteLambdaType&&>(InExecuteLambda))
	{ }

	~TLambdaRenderPass()
	{
		// Manually call the destructor of the pass parameter, to make sure RHI references are released since the pass parameters are allocated on FMemStack.
		// TODO(RDG): this may lead to RHI resource leaks if a struct allocated in FMemStack does not actually get used through FRDGBuilder::AddPass().
		ParameterStructType* Struct = reinterpret_cast<ParameterStructType*>(const_cast<void*>(FRenderGraphPass::ParameterStruct.Contents));
		Struct->~ParameterStructType();
	}

	virtual void Execute(FRHICommandListImmediate& RHICmdList) const override
	{
		ExecuteLambda(RHICmdList);
	}

	ExecuteLambdaType ExecuteLambda;
};

/** 
 * Builds the per-frame render graph.
 * Resources must be created from the builder before they can be bound to Pass ResourceTables.
 * These resources are descriptors only until the graph is executed, where RHI resources are allocated as needed.
 */
class RENDERCORE_API FRDGBuilder
{
public:
	/** A RHI cmd list is required, if using the immediate mode. */
	FRDGBuilder(FRHICommandListImmediate& InRHICmdList)
		: RHICmdList(InRHICmdList)
	{
		for (int32 i = 0; i < kMaxScopeCount; i++)
			ScopesStack[i] = nullptr;
	}

	~FRDGBuilder()
	{
		DestructPasses();
	}

	/** Register a external texture to be tracked by the render graph. */
	inline FRDGTextureRef RegisterExternalTexture(const TRefCountPtr<IPooledRenderTarget>& ExternalPooledTexture, const TCHAR* Name = TEXT("External"))
	{
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(ExternalPooledTexture.IsValid(), TEXT("Attempted to register NULL external texture: %s"), Name);
		}
		#endif
		FRDGTexture* OutTexture = new(FMemStack::Get()) FRDGTexture(Name, ExternalPooledTexture->GetDesc());
		OutTexture->PooledRenderTarget = ExternalPooledTexture;
		AllocatedTextures.Add(OutTexture, ExternalPooledTexture);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(OutTexture);
		#endif
		return OutTexture;
	}

	/** Create graph tracked resource from a descriptor with a debug name. */
	inline FRDGTextureRef CreateTexture(const FPooledRenderTargetDesc& Desc, const TCHAR* DebugName)
	{
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph texture %s needs to be created before the builder execution."), DebugName);
		}
		#endif
		FRDGTexture* Texture = new(FMemStack::Get()) FRDGTexture(DebugName, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(Texture);
		#endif
		return Texture;
	}

	/** Create graph tracked resource from a descriptor with a debug name. */
	inline FRDGBufferRef CreateBuffer(const FRDGBufferDesc& Desc, const TCHAR* DebugName)
	{
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph buffer %s needs to be created before the builder execution."), DebugName);
		}
		#endif
		FRDGBuffer* Buffer = new(FMemStack::Get()) FRDGBuffer(DebugName, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(Buffer);
		#endif
		return Buffer;
	}

	/** Create graph tracked SRV for a texture from a descriptor. */
	inline FRDGTextureSRVRef CreateSRV(const FRDGTextureSRVDesc& Desc)
	{
		check(Desc.Texture);
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph SRV %s needs to be created before the builder execution."), Desc.Texture->Name);
			ensureMsgf(Desc.Texture->Desc.TargetableFlags & TexCreate_ShaderResource, TEXT("Attempted to create SRV from texture %s which was not created with TexCreate_ShaderResource"), Desc.Texture->Name);
		}
		#endif
		
		FRDGTextureSRV* SRV = new(FMemStack::Get()) FRDGTextureSRV(Desc.Texture->Name, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(SRV);
		#endif
		return SRV;
	}

	/** Create graph tracked SRV for a buffer from a descriptor. */
	inline FRDGBufferSRVRef CreateSRV(const FRDGBufferSRVDesc& Desc)
	{
		check(Desc.Buffer);
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph SRV %s needs to be created before the builder execution."), Desc.Buffer->Name);
		}
		#endif
		
		FRDGBufferSRV* SRV = new(FMemStack::Get()) FRDGBufferSRV(Desc.Buffer->Name, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(SRV);
		#endif
		return SRV;
	}

	/** Create graph tracked UAV for a texture from a descriptor. */
	inline FRDGTextureUAVRef CreateUAV(const FRDGTextureUAVDesc& Desc)
	{
		check(Desc.Texture);
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph UAV %s needs to be created before the builder execution."), Desc.Texture->Name);
			ensureMsgf(Desc.Texture->Desc.TargetableFlags & TexCreate_UAV, TEXT("Attempted to create UAV from texture %s which was not created with TexCreate_UAV"), Desc.Texture->Name);
		}
		#endif
		
		FRDGTextureUAV* UAV = new(FMemStack::Get()) FRDGTextureUAV(Desc.Texture->Name, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(UAV);
		#endif
		return UAV;
	}

	/** Create graph tracked UAV for a buffer from a descriptor. */
	inline FRDGBufferUAVRef CreateUAV(const FRDGBufferUAVDesc& Desc)
	{
		check(Desc.Buffer);
		#if RENDER_GRAPH_DEBUGGING
		{
			ensureMsgf(!bHasExecuted, TEXT("Render graph UAV %s needs to be created before the builder execution."), Desc.Buffer->Name);
		}
		#endif
		
		FRDGBufferUAV* UAV = new(FMemStack::Get()) FRDGBufferUAV(Desc.Buffer->Name, Desc);
		#if RENDER_GRAPH_DEBUGGING
			Resources.Add(UAV);
		#endif
		return UAV;
	}

	/** Allocates parameter struct specifically to survive through the life time of the render graph. */
	template< typename ParameterStructType >
	inline ParameterStructType* AllocParameters() const
	{
		ParameterStructType* OutParameterPtr = new(FMemStack::Get()) ParameterStructType;
		FMemory::Memzero(OutParameterPtr, sizeof(ParameterStructType));
		return OutParameterPtr;
	}

	/** 
	 * Adds a lambda pass to the graph.
	 */
	template<typename ParameterStructType, typename ExecuteLambdaType>
	void AddPass(
		FRDGEventName&& Name, 
		ParameterStructType* ParameterStruct,
		ERenderGraphPassFlags Flags,
		ExecuteLambdaType&& ExecuteLambda)
	{
		#if RENDER_GRAPH_DEBUGGING
		{
			checkf(!bHasExecuted, TEXT("Render graph pass %s needs to be added before the builder execution."), Name.GetTCHAR());
		}
		#endif
		auto NewPass = new(FMemStack::Get()) TLambdaRenderPass<ParameterStructType, ExecuteLambdaType>(
			static_cast<FRDGEventName&&>(Name), CurrentScope,
			{ ParameterStruct, &ParameterStructType::FTypeInfo::GetStructMetadata()->GetLayout() },
			Flags,
			static_cast<ExecuteLambdaType&&>(ExecuteLambda) );
		Passes.Emplace(NewPass);
		if (DO_CHECK)
		{
			DebugPass(NewPass);
		}
	}

	/** Queue a texture extraction. This will set *OutTexturePtr with the internal pooled render target at the Execute().
	 *
	 * Note: even when the render graph uses the immediate debugging mode (executing passes as they get added), the texture extrations
	 * will still happen in the Execute(), to ensure there is no bug caused in code outside the render graph on whether this mode is used or not.
	 */
	inline void QueueTextureExtraction(FRDGTextureRef Texture, TRefCountPtr<IPooledRenderTarget>* OutTexturePtr, bool bTransitionToRead = true)
	{
		check(Texture);
		check(OutTexturePtr);
		#if RENDER_GRAPH_DEBUGGING
		{
			checkf(!bHasExecuted,
				TEXT("Accessing render graph internal texture %s with QueueTextureExtraction() needs to happen before the builder's execution."),
				Texture->Name);
		}
		#endif
		FDeferredInternalTextureQuery Query;
		Query.Texture = Texture;
		Query.OutTexturePtr = OutTexturePtr;
		Query.bTransitionToRead = bTransitionToRead;
		DeferredInternalTextureQueries.Emplace(Query);
	}

	/** 
	 * Executes the queued passes, managing setting of render targets (RHI RenderPasses), resource transitions and queued texture extraction.
	 */
	void Execute();


public:
	/** The RHI command list used for the render graph. */
	FRHICommandListImmediate& RHICmdList;


private:
	static constexpr int32 kMaxScopeCount = 8;

	/** Array of all pass created */
	TArray<FRenderGraphPass*, SceneRenderingAllocator> Passes;

	/** Keep the references over the pooled render target, since FRDGTexture is allocated on FMemStack. */
	TMap<const FRDGTexture*, TRefCountPtr<IPooledRenderTarget>, SceneRenderingSetAllocator> AllocatedTextures;

	/** Keep the references over the pooled render target, since FRDGTexture is allocated on FMemStack. */
	TMap<const FRDGBuffer*, TRefCountPtr<FPooledRDGBuffer>, SceneRenderingSetAllocator> AllocatedBuffers;

	/** Array of all deferred access to internal textures. */
	struct FDeferredInternalTextureQuery
	{
		const FRDGTexture* Texture;
		TRefCountPtr<IPooledRenderTarget>* OutTexturePtr;
		bool bTransitionToRead;
	};
	TArray<FDeferredInternalTextureQuery, SceneRenderingAllocator> DeferredInternalTextureQueries;

	#if RENDER_GRAPH_DRAW_EVENTS == 2
		/** All scopes allocated that needs to be arround to call destructors. */
		TArray<FRDGEventScope*, SceneRenderingAllocator> EventScopes;
	#endif

	/** The current event scope as creating passes. */
	const FRDGEventScope* CurrentScope = nullptr;

	/** Stacks of scopes pushed to the RHI command list. */
	TStaticArray<const FRDGEventScope*, kMaxScopeCount> ScopesStack;

	#if RENDER_GRAPH_DEBUGGING
		/** Whether the Execute() has already been called. */
		bool bHasExecuted = false;

		/** Lists of all created resources */
		TArray<const FRDGResource*, SceneRenderingAllocator> Resources;
	#endif

	void DebugPass(const FRenderGraphPass* Pass);
	void ValidatePass(const FRenderGraphPass* Pass) const;
	void CaptureAnyInterestingPassOutput(const FRenderGraphPass* Pass);

	void WalkGraphDependencies();

	void AllocateRHITextureIfNeeded(const FRDGTexture* Texture, bool bComputePass);
	void AllocateRHITextureSRVIfNeeded(const FRDGTextureSRV* SRV, bool bComputePass);
	void AllocateRHITextureUAVIfNeeded(const FRDGTextureUAV* UAV, bool bComputePass);
	void AllocateRHIBufferIfNeeded(const FRDGBuffer* Texture, bool bComputePass);
	void AllocateRHIBufferSRVIfNeeded(const FRDGBufferSRV* SRV, bool bComputePass);
	void AllocateRHIBufferUAVIfNeeded(const FRDGBufferUAV* UAV, bool bComputePass);


	void TransitionTexture( const FRDGTexture* Texture, EResourceTransitionAccess TransitionAccess, bool bRequiredCompute ) const;
	void TransitionUAV(FUnorderedAccessViewRHIParamRef UAV, const FRDGResource* UnderlyingResource, EResourceTransitionAccess TransitionAccess, bool bRequiredCompute ) const;

	void PushDrawEventStack(const FRenderGraphPass* Pass);
	void ExecutePass( const FRenderGraphPass* Pass );
	void AllocateAndTransitionPassResources(const FRenderGraphPass* Pass, struct FRHIRenderPassInfo* OutRPInfo, bool* bOutHasRenderTargets);
	static void WarnForUselessPassDependencies(const FRenderGraphPass* Pass);

	void ReleaseRHITextureIfPossible(const FRDGTexture* Texture);
	void ReleaseRHIBufferIfPossible(const FRDGBuffer* Buffer);
	void ReleaseUnecessaryResources(const FRenderGraphPass* Pass);

	void ProcessDeferredInternalResourceQueries();
	void DestructPasses();

	friend class FStackRDGEventScopeRef;
}; // class FRDGBuilder


#if RENDER_GRAPH_DRAW_EVENTS

/** Stack reference of render graph scope. */
class RENDERCORE_API FStackRDGEventScopeRef
{
public:
	FStackRDGEventScopeRef() = delete;
	FStackRDGEventScopeRef(const FStackRDGEventScopeRef&) = delete;
	FStackRDGEventScopeRef(FStackRDGEventScopeRef&&) = delete;
	void operator = (const FStackRDGEventScopeRef&) = delete;

	inline FStackRDGEventScopeRef(FRDGBuilder& InGraphBuilder, FRDGEventName&& ScopeName)
		: GraphBuilder(InGraphBuilder)
	{
		checkf(!GraphBuilder.bHasExecuted, TEXT("Render graph bulider has already been executed."));

		auto NewScope = new(FMemStack::Get()) FRDGEventScope(GraphBuilder.CurrentScope, Forward<FRDGEventName>(ScopeName));

		#if RENDER_GRAPH_DRAW_EVENTS == 2
		{
			GraphBuilder.EventScopes.Add(NewScope);
		}
		#endif

		GraphBuilder.CurrentScope = NewScope;
	}

	inline ~FStackRDGEventScopeRef()
	{
		check(GraphBuilder.CurrentScope != nullptr);
		GraphBuilder.CurrentScope = GraphBuilder.CurrentScope->ParentScope;
	}

private:
	FRDGBuilder& GraphBuilder;
};

#endif // RENDER_GRAPH_DRAW_EVENTS


/** Macros for create render graph event names and scopes.
 *
 *		FRDGEventName Name = RDG_EVENT_NAME("MyPass %sx%s", ViewRect.Width(), ViewRect.Height());
 *
 *		RDG_EVENT_SCOPE(GraphBuilder, "MyProcessing %sx%s", ViewRect.Width(), ViewRect.Height());
 */
#if RENDER_GRAPH_DRAW_EVENTS == 2

#define RDG_EVENT_NAME(Format, ...) FRDGEventName(TEXT(Format), ##__VA_ARGS__)
#define RDG_EVENT_SCOPE(GraphBuilder, Format, ...) \
	FStackRDGEventScopeRef PREPROCESSOR_JOIN(__RDG_ScopeRef_,__LINE__) ((GraphBuilder), RDG_EVENT_NAME(Format, ##__VA_ARGS__))

#elif RENDER_GRAPH_DRAW_EVENTS == 1

#define RDG_EVENT_NAME(Format, ...) FRDGEventName(TEXT(Format))
#define RDG_EVENT_SCOPE(GraphBuilder, Format, ...) \
	FStackRDGEventScopeRef PREPROCESSOR_JOIN(__RDG_ScopeRef_,__LINE__) ((GraphBuilder), RDG_EVENT_NAME(Format, ##__VA_ARGS__))

#else // !RENDER_GRAPH_DRAW_EVENTS

#define RDG_EVENT_NAME(Format, ...) FRDGEventName()
#define RDG_EVENT_SCOPE(GraphBuilder, Format, ...) 

#endif // !RENDER_GRAPH_DRAW_EVENTS
