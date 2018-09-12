// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TranslationPickerEditWindow.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Internationalization/Culture.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/SlateTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
#include "TranslationDataManager.h"
#include "TranslationUnit.h"
#include "ILocalizationServiceModule.h"

#define LOCTEXT_NAMESPACE "TranslationPicker"

TSharedPtr<FTranslationPickerSettingsManager> FTranslationPickerSettingsManager::TranslationPickerSettingsManagerInstance;

// Default dimensions of the Translation Picker edit window (floating window also uses these sizes, so it matches roughly)
const int32 STranslationPickerEditWindow::DefaultEditWindowWidth = 500;
const int32 STranslationPickerEditWindow::DefaultEditWindowHeight = 500;

UTranslationPickerSettings::UTranslationPickerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void STranslationPickerEditWindow::Construct(const FArguments& InArgs)
{
	ParentWindow = InArgs._ParentWindow;
	PickedTexts = InArgs._PickedTexts;
	WindowContents = SNew(SBox);
	TSharedRef<SVerticalBox> TextsBox = SNew(SVerticalBox);
	UTranslationPickerSettings* TranslationPickerSettings = FTranslationPickerSettingsManager::Get()->GetSettings();

	bool bShowLocServiceCheckbox = ILocalizationServiceModule::Get().GetProvider().IsEnabled();

	if (!FParse::Param(FCommandLine::Get(), TEXT("AllowTranslationPickerSubmissionsToOneSky")))
	{
		bShowLocServiceCheckbox = false;
		TranslationPickerSettings->bSubmitTranslationPickerChangesToLocalizationService = false;
	}

	// Add a new Translation Picker Edit Widget for each picked text
	for (FText PickedText : PickedTexts)
	{
		TSharedPtr<SEditableTextBox> TextBox;
		int32 DefaultPadding = 0.0f;

		TSharedRef<STranslationPickerEditWidget> NewEditWidget = 
			SNew(STranslationPickerEditWidget)
			.PickedText(PickedText)
			.bAllowEditing(true);

		EditWidgets.Add(NewEditWidget);

		TextsBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(5))
			[
				SNew(SBorder)
				[
					NewEditWidget
				]
			];
	}

	TSharedPtr<SEditableTextBox> TextBox;
	int32 DefaultPadding = 0.0f;

	// Layout the Translation Picker Edit Widgets and some save/close buttons below them
	WindowContents->SetContent(
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			[
				SNew(SScrollBox)
				+SScrollBox::Slot()
				.Padding(FMargin(8, 5, 8, 5))
				[
					TextsBox
				]
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(DefaultPadding)
			[
				SNew(SVerticalBox)
				
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				.Padding(DefaultPadding)
				[
					SNew(SHorizontalBox)
					.Visibility(bShowLocServiceCheckbox ? EVisibility::Visible : EVisibility::Collapsed)
					
					+SHorizontalBox::Slot()
					.Padding(FMargin(3, 3, 3, 3))
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.HAlign(HAlign_Center)
						.IsChecked(TranslationPickerSettings->bSubmitTranslationPickerChangesToLocalizationService ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.ToolTipText(LOCTEXT("SubmitTranslationPickerChangesToLocalizationServiceToolTip", "Submit changes to localization service"))
						.OnCheckStateChanged_Lambda([&](ECheckBoxState CheckedState)
						{
							UTranslationPickerSettings* TranslationPickerSettingsLocal = FTranslationPickerSettingsManager::Get()->GetSettings();
							TranslationPickerSettingsLocal->bSubmitTranslationPickerChangesToLocalizationService = CheckedState == ECheckBoxState::Checked;
							TranslationPickerSettingsLocal->SaveConfig();
						}
						)
					]
					
					+SHorizontalBox::Slot()
					.Padding(FMargin(0, 0, 3, 0))
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SubmitTranslationPickerChangesToLocalizationService", "Save to Localization Service"))
						.ToolTipText(LOCTEXT("SubmitTranslationPickerChangesToLocalizationServiceToolTip", "Submit changes to localization service"))
					]
				]
				
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(FMargin(0, 5))
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
					.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
					.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
					
					+SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(this, &STranslationPickerEditWindow::SaveAllAndClose)
						.Text(LOCTEXT("SaveAllAndClose", "Save All and Close"))
					]
					
					+SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(this, &STranslationPickerEditWindow::Close)
						.Text(LOCTEXT("CancelButton", "Cancel"))
					]
				]
			]
		]
	);

	ChildSlot
	[
		WindowContents.ToSharedRef()
	];
}

FReply STranslationPickerEditWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		Close();
		
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply STranslationPickerEditWindow::Close()
{
	if (ParentWindow.IsValid())
	{
		FSlateApplication::Get().RequestDestroyWindow(ParentWindow.Pin().ToSharedRef());
		ParentWindow.Reset();
	}

	return FReply::Handled();
}

FReply STranslationPickerEditWindow::SaveAllAndClose()
{
	TArray<UTranslationUnit*> TempArray;

	for (TSharedRef<STranslationPickerEditWidget> EditWidget : EditWidgets)
	{
		UTranslationUnit* TranslationUnit = EditWidget->GetTranslationUnitWithAnyChanges();
		if (TranslationUnit != nullptr && EditWidget->CanSave())
		{
			TempArray.Add(TranslationUnit);
		}
	}

	if (TempArray.Num() > 0)
	{
		UTranslationPickerSettings* TranslationPickerSettings = FTranslationPickerSettingsManager::Get()->GetSettings();
		// Save the data via translation data manager
		FTranslationDataManager::SaveSelectedTranslations(TempArray, ILocalizationServiceModule::Get().GetProvider().IsEnabled() && TranslationPickerSettings->bSubmitTranslationPickerChangesToLocalizationService);
	}

	Close();

	return FReply::Handled();
}

void STranslationPickerEditWidget::Construct(const FArguments& InArgs)
{
	PickedText = InArgs._PickedText;
	bAllowEditing = InArgs._bAllowEditing;
	int32 DefaultPadding = 0.0f;

	bool bCultureInvariant = PickedText.IsCultureInvariant();
	bool bShouldGatherForLocalization = FTextInspector::ShouldGatherForLocalization(PickedText);

	// Get all the data we need and format it properly
	TOptional<FString> NamespaceString = FTextInspector::GetNamespace(PickedText);
	TOptional<FString> KeyString = FTextInspector::GetKey(PickedText);
	const FString* SourceString = FTextInspector::GetSourceString(PickedText);
	const FString& TranslationString = FTextInspector::GetDisplayString(PickedText);
	FString LocresFullPath;

	FString ManifestAndArchiveNameString;
	if (NamespaceString && KeyString)
	{
		FString LocResId;
		if (FTextLocalizationManager::Get().GetLocResID(NamespaceString.GetValue(), KeyString.GetValue(), LocResId))
		{
			LocresFullPath = *LocResId;
			ManifestAndArchiveNameString = FPaths::GetBaseFilename(*LocResId);
		}
	}

	FString ArchiveFilePath = FPaths::GetPath(LocresFullPath);
	FString LocResCultureName = FPaths::GetBaseFilename(ArchiveFilePath);

	const FString CleanNamespaceString = TextNamespaceUtil::StripPackageNamespace(NamespaceString.Get(TEXT("")));
	FText Namespace = FText::FromString(CleanNamespaceString);
	FText Key = FText::FromString(KeyString.Get(TEXT("")));
	FText Source = SourceString != nullptr ? FText::FromString(*SourceString) : FText::GetEmpty();
	FText ManifestAndArchiveName = FText::FromString(ManifestAndArchiveNameString);
	FText Translation = FText::FromString(TranslationString);

	// Save the necessary data in UTranslationUnit for later.  This is what we pass to TranslationDataManager to save our edits
	TranslationUnit = NewObject<UTranslationUnit>();
	TranslationUnit->Namespace = CleanNamespaceString;
	TranslationUnit->Source = SourceString != nullptr ? *SourceString : TEXT("");
	TranslationUnit->Translation = TranslationString;
	TranslationUnit->LocresPath = LocresFullPath;

	// Can only save if we have all the required information
	bHasRequiredLocalizationInfoForSaving = NamespaceString.IsSet() && SourceString != nullptr && LocresFullPath.Len() > 0;

	TSharedPtr<SGridPanel> GridPanel;

	// Layout all our data
	ChildSlot
	[
		SNew(SHorizontalBox)
		
		+SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(FMargin(5))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SAssignNew(GridPanel, SGridPanel)
				.FillColumn(1,1)
				
				+SGridPanel::Slot(0,0)
				.Padding(FMargin(2.5))
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "RichTextBlock.Bold")
					.Text(LOCTEXT("SourceLabel", "Source:"))
				]

				+SGridPanel::Slot(0, 1)
				.Padding(FMargin(2.5))
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					// Hide translation if we don't have necessary information to modify, and is same as source
					.Visibility(!bHasRequiredLocalizationInfoForSaving && SourceString->Equals(TranslationString) ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "RichTextBlock.Bold")
						.Text(FText::Format(LOCTEXT("TranslationLabel", "Translation ({0}):"), FText::FromString(LocResCultureName)))
					]
				]
				
				+SGridPanel::Slot(1, 0)
				.Padding(FMargin(2.5))
				[
					SNew(SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.Text(Source)
				]

				+SGridPanel::Slot(1, 1)
				.Padding(FMargin(2.5))
				[
					SNew(SBox)
					// Hide translation if we don't have necessary information to modify, and is same as source
					.Visibility(!bHasRequiredLocalizationInfoForSaving && SourceString->Equals(TranslationString) ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SAssignNew(TextBox, SMultiLineEditableTextBox)
						.IsReadOnly(!bAllowEditing || !bHasRequiredLocalizationInfoForSaving)
						.Text(Translation)
						.HintText(LOCTEXT("TranslationEditTextBox_HintText", "Enter/edit translation here."))
					]
				]
			]
		]
	];

	if (bCultureInvariant)
	{
		GridPanel->AddSlot(0, 2)
			.Padding(FMargin(2.5))
			.ColumnSpan(2)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CultureInvariantLabel", "This text is culture-invariant"))
			];
	}
	else if (!bShouldGatherForLocalization)
	{
		GridPanel->AddSlot(0, 2)
			.Padding(FMargin(2.5))
			.ColumnSpan(2)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NotGatheredForLocalizationLabel", "This text is not gathered for localization"))
			];
	}
	else if (!bHasRequiredLocalizationInfoForSaving)
	{
		GridPanel->AddSlot(0, 2)
			.Padding(FMargin(2.5))
			.ColumnSpan(2)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RequiredLocalizationInfoNotFound", "This text is not ready to be localized."))
			];
	}
	else
	{
		GridPanel->AddSlot(0, 2)
			.Padding(FMargin(2.5))
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "RichTextBlock.Bold")
				.Text(LOCTEXT("NamespaceLabel", "Namespace:"))
			];
		GridPanel->AddSlot(1, 2)
			.Padding(FMargin(2.5))
			[
				SNew(SEditableTextBox)
				.IsReadOnly(true)
				.Text(Namespace)
			];
		GridPanel->AddSlot(0, 3)
			.Padding(FMargin(2.5))
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "RichTextBlock.Bold")
				.Text(LOCTEXT("KeyLabel", "Key:"))
			];
		GridPanel->AddSlot(1, 3)
			.Padding(FMargin(2.5))
			[
				SNew(SEditableTextBox)
				.IsReadOnly(true)
				.Text(Key)
			];
		GridPanel->AddSlot(0, 4)
			.Padding(FMargin(2.5))
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "RichTextBlock.Bold")
				.Text(LOCTEXT("LocresFileLabel", "Target:"))
			];
		GridPanel->AddSlot(1, 4)
			.Padding(FMargin(2.5))
			[
				SNew(SEditableTextBox)
				.IsReadOnly(true)
				.Text(ManifestAndArchiveName)
			];
		GridPanel->AddSlot(0, 5)
			.Padding(FMargin(2.5))
			.ColumnSpan(2)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &STranslationPickerEditWidget::SaveAndPreview)
				.IsEnabled(bHasRequiredLocalizationInfoForSaving)
				.Visibility(bAllowEditing ? EVisibility::Visible : EVisibility::Collapsed)
				.Text(bHasRequiredLocalizationInfoForSaving ? LOCTEXT("SaveAndPreviewButtonText", "Save and Preview") : LOCTEXT("SaveAndPreviewButtonDisabledText", "Cannot Save"))
			];
	}
}

void STranslationPickerEditWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(TranslationUnit);
}

FReply STranslationPickerEditWidget::SaveAndPreview()
{
	// Update translation string from entered text
	TranslationUnit->Translation = TextBox->GetText().ToString();
	UTranslationPickerSettings* TranslationPickerSettings = FTranslationPickerSettingsManager::Get()->GetSettings();

	// Save the data via translation data manager
	TArray<UTranslationUnit*> TempArray;
	TempArray.Add(TranslationUnit);
	FTranslationDataManager::SaveSelectedTranslations(TempArray, ILocalizationServiceModule::Get().GetProvider().IsEnabled() && TranslationPickerSettings->bSubmitTranslationPickerChangesToLocalizationService);

	return FReply::Handled();
}

UTranslationUnit* STranslationPickerEditWidget::GetTranslationUnitWithAnyChanges()
{
	if (TranslationUnit)
	{
		// Update translation string from entered text
		TranslationUnit->Translation = TextBox->GetText().ToString();

		return TranslationUnit;
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
