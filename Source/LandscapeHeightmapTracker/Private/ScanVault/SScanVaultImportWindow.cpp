#include "ScanVault/SScanVaultImportWindow.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace LandscapeHeightmapTracker::ScanVault
{
#define LOCTEXT_NAMESPACE "SScanVaultImportWindow"

void SScanVaultImportWindow::Construct(const FArguments& InArgs)
{
	OwnerWindow = InArgs._OwnerWindow;
	ImportPlan = InArgs._ImportPlan;
	OnImportConfirmed = InArgs._OnImportConfirmed;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "ScanVault Import Package"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text(this, &SScanVaultImportWindow::BuildPreviewText)
					.AutoWrapText(true)
				]
				+ SScrollBox::Slot()
				.Padding(0.0f, 12.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SScanVaultImportWindow::BuildIssuesText)
					.AutoWrapText(true)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 12.0f, 0.0f)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(this, &SScanVaultImportWindow::IsConflictPolicyChecked, EConflictPolicy::Cancel)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
					{
						if (State == ECheckBoxState::Checked)
						{
							SetConflictPolicy(EConflictPolicy::Cancel);
						}
					})
					[
						SNew(STextBlock).Text(LOCTEXT("ConflictCancel", "Cancel on conflict"))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(this, &SScanVaultImportWindow::IsConflictPolicyChecked, EConflictPolicy::UniqueSuffix)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
					{
						if (State == ECheckBoxState::Checked)
						{
							SetConflictPolicy(EConflictPolicy::UniqueSuffix);
						}
					})
					[
						SNew(STextBlock).Text(LOCTEXT("ConflictUnique", "Import with unique suffix"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FMargin(4.0f, 0.0f))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("ImportButton", "Import"))
					.IsEnabled_Lambda([this]() { return ImportPlan.bCanImport; })
					.OnClicked(this, &SScanVaultImportWindow::ImportClicked)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("CancelButton", "Cancel"))
					.OnClicked(this, &SScanVaultImportWindow::CancelClicked)
				]
			]
		]
	];
}

FReply SScanVaultImportWindow::ImportClicked()
{
	OnImportConfirmed.ExecuteIfBound(ImportPlan, ConflictPolicy);
	if (const TSharedPtr<SWindow> Window = OwnerWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SScanVaultImportWindow::CancelClicked() const
{
	if (const TSharedPtr<SWindow> Window = OwnerWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

void SScanVaultImportWindow::SetConflictPolicy(EConflictPolicy InPolicy)
{
	ConflictPolicy = InPolicy;
}

ECheckBoxState SScanVaultImportWindow::IsConflictPolicyChecked(EConflictPolicy InPolicy) const
{
	return ConflictPolicy == InPolicy ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText SScanVaultImportWindow::BuildPreviewText() const
{
	const FScanVaultManifest& Manifest = ImportPlan.Manifest;
	const int32 TextureCount = Manifest.Textures.Num();
	const int32 LodCount = Manifest.Mesh.Lods.Num() > 0 ? FMath::Max(0, Manifest.Mesh.Lods.Num() - 1) : 0;
	const FString MasterMaterial = Manifest.Material.MasterMaterialPath.IsEmpty() ? TEXT("(none)") : Manifest.Material.MasterMaterialPath;
	const FString MaterialInstance = MakeMaterialInstanceAssetName(Manifest);

	TStringBuilder<2048> Builder;
	Builder.Appendf(TEXT("Package\n  packageId: %s\n  source asset: %s\n  asset type: %s\n\n"), *Manifest.PackageId, *Manifest.Source.Name, *Manifest.Source.AssetType);
	Builder.Appendf(TEXT("Destination\n  %s\n\n"), *Manifest.Destination.ContentPath);
	Builder.Appendf(TEXT("Mesh\n  primary: %s\n  LOD count: %d\n\n"), Manifest.Mesh.bHasMesh ? TEXT("declared") : TEXT("none"), LodCount);
	Builder.Appendf(TEXT("Textures\n  count: %d\n"), TextureCount);
	for (const FScanVaultTexture& Texture : Manifest.Textures)
	{
		Builder.Appendf(TEXT("  %s: %s\n"), *TextureRoleToString(Texture.Role), *Texture.SourcePath);
	}
	Builder.Appendf(TEXT("\nMaterial\n  profile: %s\n  Master Material: %s\n  Material Instance: %s\n  mappings: %d\n\n"), *Manifest.Material.ProfileName, *MasterMaterial, *MaterialInstance, Manifest.Material.TextureParameterMappings.Num());
	Builder.Appendf(TEXT("Options\n  Import LODs: %s\n  Enable Nanite: %s\n  Create Material Instance: %s\n"),
		Manifest.Options.bImportLods ? TEXT("true") : TEXT("false"),
		Manifest.Options.bEnableNanite ? TEXT("true") : TEXT("false"),
		Manifest.Options.bCreateMaterialInstance ? TEXT("true") : TEXT("false"));
	return FText::FromString(Builder.ToString());
}

FText SScanVaultImportWindow::BuildIssuesText() const
{
	if (ImportPlan.Issues.IsEmpty())
	{
		return LOCTEXT("NoIssues", "Validation\n  No errors or warnings.");
	}

	TStringBuilder<1024> Builder;
	Builder.Append(TEXT("Validation\n"));
	for (const FImportIssue& Issue : ImportPlan.Issues)
	{
		const TCHAR* SeverityText = Issue.Severity == EImportIssueSeverity::Error
			? TEXT("Error")
			: (Issue.Severity == EImportIssueSeverity::Warning ? TEXT("Warning") : TEXT("Info"));
		Builder.Appendf(TEXT("  [%s] %s\n"), SeverityText, *Issue.Message);
	}
	return FText::FromString(Builder.ToString());
}

#undef LOCTEXT_NAMESPACE
}
