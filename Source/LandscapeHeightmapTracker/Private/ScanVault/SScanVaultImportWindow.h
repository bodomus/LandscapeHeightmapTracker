#pragma once

#include "CoreMinimal.h"
#include "ScanVault/ScanVaultImportTypes.h"
#include "Widgets/SCompoundWidget.h"

class SWindow;

namespace LandscapeHeightmapTracker::ScanVault
{
DECLARE_DELEGATE_TwoParams(FOnScanVaultImportConfirmed, const FScanVaultImportPlan&, EConflictPolicy);

class SScanVaultImportWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SScanVaultImportWindow) {}
		SLATE_ARGUMENT(TWeakPtr<SWindow>, OwnerWindow)
		SLATE_ARGUMENT(FScanVaultImportPlan, ImportPlan)
		SLATE_EVENT(FOnScanVaultImportConfirmed, OnImportConfirmed)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply ImportClicked();
	FReply CancelClicked() const;
	void SetConflictPolicy(EConflictPolicy InPolicy);
	ECheckBoxState IsConflictPolicyChecked(EConflictPolicy InPolicy) const;
	FText BuildPreviewText() const;
	FText BuildIssuesText() const;

	TWeakPtr<SWindow> OwnerWindow;
	FScanVaultImportPlan ImportPlan;
	FOnScanVaultImportConfirmed OnImportConfirmed;
	EConflictPolicy ConflictPolicy = EConflictPolicy::Cancel;
};
}
