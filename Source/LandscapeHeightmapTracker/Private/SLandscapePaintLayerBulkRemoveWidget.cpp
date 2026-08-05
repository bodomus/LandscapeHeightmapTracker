#include "SLandscapePaintLayerBulkRemoveWidget.h"

#include "LandscapePaintLayerService.h"
#include "LandscapeProxy.h"
#include "Misc/MessageDialog.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SLandscapePaintLayerBulkRemoveWidget"

void SLandscapePaintLayerBulkRemoveWidget::Construct(const FArguments& InArgs)
{
	StatusText = LOCTEXT("InitialStatus", "Select exactly one Landscape and refresh.");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SLandscapePaintLayerBulkRemoveWidget::GetLandscapeText)
					.Font(FAppStyle::GetFontStyle("SmallFontBold"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton).Text(LOCTEXT("Refresh", "Refresh")).OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::Refresh)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton).Text(LOCTEXT("SelectAll", "Select All")).OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::SelectAll)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton).Text(LOCTEXT("SelectNone", "None")).OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::SelectNone)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton).Text(LOCTEXT("Invert", "Invert")).OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::InvertSelection)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().MaxHeight(320.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(RowsBox, SVerticalBox)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
						.Text(LOCTEXT("RemoveSelected", "Remove Selected"))
						.IsEnabled(this, &SLandscapePaintLayerBulkRemoveWidget::CanRemoveSelected)
						.OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::RemoveSelected)
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
						.Text(LOCTEXT("RemoveAll", "Remove All"))
						.IsEnabled(this, &SLandscapePaintLayerBulkRemoveWidget::CanRemoveAll)
						.OnClicked(this, &SLandscapePaintLayerBulkRemoveWidget::RemoveAll)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock).Text(this, &SLandscapePaintLayerBulkRemoveWidget::GetStatusText).AutoWrapText(true)
			]
		]
	];

	RefreshFromSelection();
}

FReply SLandscapePaintLayerBulkRemoveWidget::Refresh()
{
	RefreshFromSelection();
	return FReply::Handled();
}

void SLandscapePaintLayerBulkRemoveWidget::RefreshFromSelection()
{
	Items.Reset();
	SelectedLandscape.Reset();

	FString Error;
	ALandscapeProxy* Landscape = FLandscapePaintLayerService::GetExactlyOneSelectedLandscape(Error);
	if (!Landscape)
	{
		StatusText = FText::FromString(Error);
		RebuildRows();
		return;
	}

	TArray<FLandscapePaintTargetLayerInfo> Layers;
	if (!FLandscapePaintLayerService::GetTargetLayers(Landscape, Layers, Error))
	{
		StatusText = FText::FromString(Error);
		RebuildRows();
		return;
	}

	SelectedLandscape = Landscape;
	for (FLandscapePaintTargetLayerInfo& Layer : Layers)
	{
		TSharedPtr<FLandscapePaintLayerListItem> Item = MakeShared<FLandscapePaintLayerListItem>();
		Item->Info = MoveTemp(Layer);
		Items.Add(MoveTemp(Item));
	}
	StatusText = FText::Format(LOCTEXT("LayerCount", "{0} Paint Target Layer(s)."), FText::AsNumber(Items.Num()));
	RebuildRows();
}

void SLandscapePaintLayerBulkRemoveWidget::RebuildRows()
{
	if (!RowsBox.IsValid())
	{
		return;
	}
	RowsBox->ClearChildren();
	if (Items.IsEmpty())
	{
		RowsBox->AddSlot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("NoLayers", "No Paint Target Layers available."))
		];
		return;
	}

	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items)
	{
		const FText Details = FText::FromString(FString::Printf(
			TEXT("%s | %s | Material: %s%s"),
			*Item->Info.BlendType,
			*Item->Info.LayerInfoPath,
			Item->Info.bExistsInLandscapeMaterial ? TEXT("yes") : TEXT("no"),
			Item->Info.bIsOrphaned ? TEXT(" | ORPHANED") : TEXT("")));
		RowsBox->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(SCheckBox)
				.IsChecked_Lambda([Item]() { return Item->bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([Item](ECheckBoxState State) { Item->bSelected = State == ECheckBoxState::Checked; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(FText::FromName(Item->Info.LayerName))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(Details).Font(FAppStyle::GetFontStyle("SmallFont")).AutoWrapText(true)
					]
				]
		];
	}
}

FReply SLandscapePaintLayerBulkRemoveWidget::SelectAll()
{
	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items) Item->bSelected = true;
	return FReply::Handled();
}

FReply SLandscapePaintLayerBulkRemoveWidget::SelectNone()
{
	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items) Item->bSelected = false;
	return FReply::Handled();
}

FReply SLandscapePaintLayerBulkRemoveWidget::InvertSelection()
{
	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items) Item->bSelected = !Item->bSelected;
	return FReply::Handled();
}

FReply SLandscapePaintLayerBulkRemoveWidget::RemoveSelected()
{
	TArray<FName> LayerNames;
	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items)
	{
		if (Item->bSelected) LayerNames.Add(Item->Info.LayerName);
	}
	return ConfirmAndRemove(LayerNames, false);
}

FReply SLandscapePaintLayerBulkRemoveWidget::RemoveAll()
{
	TArray<FName> LayerNames;
	for (const TSharedPtr<FLandscapePaintLayerListItem>& Item : Items) LayerNames.Add(Item->Info.LayerName);
	return ConfirmAndRemove(LayerNames, true);
}

FReply SLandscapePaintLayerBulkRemoveWidget::ConfirmAndRemove(const TArray<FName>& LayerNames, bool bRemoveAll)
{
	if (!SelectedLandscape.IsValid() || LayerNames.IsEmpty()) return FReply::Handled();

	TArray<FString> Names;
	for (const FName LayerName : LayerNames) Names.Add(LayerName.ToString());
	const FString Warning = FString::Printf(
		TEXT("%s\n\nLandscape: %s\nLayers (%d):\n%s\n\nPaint data for these Target Layers will be permanently removed. LayerInfo assets and the Landscape material will not be deleted or modified. You can undo this as one operation.\n\nContinue?"),
		bRemoveAll ? TEXT("WARNING: REMOVE ALL PAINT TARGET LAYERS") : TEXT("Remove selected Paint Target Layers"),
		*SelectedLandscape->GetActorLabel(),
		LayerNames.Num(),
		*FString::Join(Names, TEXT("\n")));
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Warning)) != EAppReturnType::Yes)
	{
		StatusText = LOCTEXT("Cancelled", "Removal cancelled.");
		return FReply::Handled();
	}

	const FLandscapePaintLayerRemoveResult Result = FLandscapePaintLayerService::RemoveTargetLayers(SelectedLandscape.Get(), LayerNames);
	StatusText = FText::FromString(Result.Message);
	if (Result.bSuccess) RefreshFromSelection();
	return FReply::Handled();
}

bool SLandscapePaintLayerBulkRemoveWidget::CanRemoveSelected() const
{
	return SelectedLandscape.IsValid() && Items.ContainsByPredicate(
		[](const TSharedPtr<FLandscapePaintLayerListItem>& Item) { return Item->bSelected; });
}

bool SLandscapePaintLayerBulkRemoveWidget::CanRemoveAll() const
{
	return SelectedLandscape.IsValid() && !Items.IsEmpty();
}

FText SLandscapePaintLayerBulkRemoveWidget::GetLandscapeText() const
{
	return SelectedLandscape.IsValid()
		? FText::Format(LOCTEXT("SelectedLandscape", "Selected: {0}"), FText::FromString(SelectedLandscape->GetActorLabel()))
		: LOCTEXT("NoSelectedLandscape", "Selected: none");
}

FText SLandscapePaintLayerBulkRemoveWidget::GetStatusText() const
{
	return StatusText;
}

#undef LOCTEXT_NAMESPACE
