// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraCommon.h"
#include "NiagaraRibbonRendererProperties.generated.h"

UENUM()
enum class ENiagaraRibbonFacingMode : uint8
{
	Screen,
	Custom
};

UCLASS(editinlinenew)
class UNiagaraRibbonRendererProperties : public UNiagaraRendererProperties
{
public:
	GENERATED_BODY()

	UNiagaraRibbonRendererProperties()
		: Material(nullptr)
		, FacingMode(ENiagaraRibbonFacingMode::Screen)
		, UV0TilingDistance(0.0f)
		, UV0Scale(FVector2D(1.0f, 1.0f))
		, UV1TilingDistance(0.0f)
		, UV1Scale(FVector2D(1.0f, 1.0f))
	{
	}

	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	ENiagaraRibbonFacingMode FacingMode;

	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	float UV0TilingDistance;
	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	FVector2D UV0Scale;

	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	float UV1TilingDistance;
	UPROPERTY(EditAnywhere, Category = "Ribbon Rendering")
	FVector2D UV1Scale;

	//~ UNiagaraRendererProperties interface
	virtual NiagaraRenderer* CreateEmitterRenderer(ERHIFeatureLevel::Type FeatureLevel) override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials) const override;
	virtual bool IsSimTargetSupported(ENiagaraSimTarget InSimTarget) const override { return (InSimTarget == ENiagaraSimTarget::CPUSim); };
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual const TArray<FNiagaraVariable>& GetRequiredAttributes() override;
	virtual const TArray<FNiagaraVariable>& GetOptionalAttributes() override;
    virtual bool IsMaterialValidForRenderer(UMaterial* Material, FText& InvalidMessage) override;
	virtual void FixMaterial(UMaterial* Material);
#endif

	virtual void PostInitProperties() override;
	static void InitCDOPropertiesAfterModuleStartup();


	/** Which attribute should we use for position when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding PositionBinding;

	/** Which attribute should we use for color when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding ColorBinding;

	/** Which attribute should we use for velocity when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding VelocityBinding;

	/** Which attribute should we use for normalized age when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding NormalizedAgeBinding;

	/** Which attribute should we use for ribbon twist when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding RibbonTwistBinding;

	/** Which attribute should we use for ribbon width when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding RibbonWidthBinding;

	/** Which attribute should we use for ribbon facing when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding RibbonFacingBinding;
	
	/** Which attribute should we use for ribbon id when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding RibbonIdBinding;

	/** Which attribute should we use for RibbonLinkOrder when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding RibbonLinkOrderBinding;

	/** Which attribute should we use for dynamic material parameters when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding DynamicMaterialBinding;

	/** Which attribute should we use for dynamic material parameters when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding DynamicMaterial1Binding;

	/** Which attribute should we use for dynamic material parameters when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding DynamicMaterial2Binding;

	/** Which attribute should we use for dynamic material parameters when generating ribbons?*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Ribbon Rendering")
	FNiagaraVariableAttributeBinding DynamicMaterial3Binding;

	UPROPERTY(Transient)
	int32 SyncId;
protected:
	void InitBindings();
};
