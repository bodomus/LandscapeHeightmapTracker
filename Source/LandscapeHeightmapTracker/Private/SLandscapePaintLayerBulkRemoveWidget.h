#pragma once

#include "CoreMinimal.h"
#include "LandscapePaintLayerTypes.h"
#include "Widgets/SCompoundWidget.h"

class ALandscapeProxy;
class SVerticalBox;

struct FLandscapePaintLayerListItem
{
	FLandscapePaintTargetLayerInfo Info;
	bool bSelected = false;
};

class SLandscapePaintLayerBulkRemoveWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapePaintLayerBulkRemoveWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply Refresh();
	FReply SelectAll();
	FReply SelectNone();
	FReply InvertSelection();
	FReply RemoveSelected();
	FReply RemoveAll();
	void RefreshFromSelection();
	void RebuildRows();
	FReply ConfirmAndRemove(const TArray<FName>& LayerNames, bool bRemoveAll);
	bool CanRemoveSelected() const;
	bool CanRemoveAll() const;
	FText GetLandscapeText() const;
	FText GetStatusText() const;

	TWeakObjectPtr<ALandscapeProxy> SelectedLandscape;
	TArray<TSharedPtr<FLandscapePaintLayerListItem>> Items;
	TSharedPtr<SVerticalBox> RowsBox;
	FText StatusText;
};
