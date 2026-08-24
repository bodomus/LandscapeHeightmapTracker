<!-- Source: https://bodomus.youtrack.cloud/issue/UE5-18 -->
<!-- Snapshot: 2026-08-15 -->

**# UE57Editor — Refresh Content / Rescan Asset Registry without restarting Unreal Editor**

**## Status**

Ready for implementation.

**## Project**

****UE57Editor****

**## Summary**

When files or Marketplace/Fab packages are copied externally into the project’s physical:

\`\`\`text

\<ProjectRoot\>/Content

\`\`\`

while Unreal Editor is already running, the Content Browser may not immediately display the newly added \`.uasset\`, \`.umap\`, and related Unreal asset files.

Current workaround:

\`\`\`text

close Unreal Editor

→ reopen project

→ Asset Registry rescans content

\`\`\`

Add a small UE57Editor editor command that refreshes project content from inside the running editor.

Important Unreal terminology:

\`\`\`text

Physical path: \<ProjectRoot\>/Content

Virtual Unreal mount path: /Game

\`\`\`

The primary command should rescan \`/Game\`.

**## Goal**

Add a reliable ****Refresh Content**** command to UE57Editor that:

1\. rescans the project Content folder through Asset Registry;

2\. detects externally added Unreal assets without restarting UE;

3\. refreshes Content Browser where required;

4\. shows concise success/failure feedback;

5\. never modifies or deletes assets.

Expected workflow:

\`\`\`text

UE is running

→ copy a package into \<Project\>/Content

→ click Refresh Content

→ new assets become visible in Content Browser

\`\`\`

**## Mandatory pre-ticket workflow**

Treat this as a ****Level 2 editor integration task****.

Before implementation Codex must:

1\. Resolve repository root.

2\. Read \`AGENTS.md\`.

3\. Read \`.codex/PRE_TICKET_WORKFLOW.md\`.

4\. Execute \`$graphify-repository-analysis\`.

5\. Execute \`$code-review-graph-analysis\`.

6\. Inspect the existing UE57Editor command/tool architecture.

7\. Locate the correct editor module and toolbar/menu registration points.

8\. Verify graph findings against source.

9\. Create:

   - \`Task/UE57-RefreshContent/investigation.md\`

   - \`Task/UE57-RefreshContent/implementation-plan.md\`

10\. Implement only after investigation and plan are complete.

11\. Update CRG after implementation.

12\. Inspect blast radius.

13\. Refresh Graphify only if architecture/subsystem boundaries changed.

14\. Create:

   - \`Task/UE57-RefreshContent/implementation-report.md\`

Do not invent Graphify or CRG commands. Preserve unrelated user changes.

**## Required investigation**

Before coding, determine:

1\. Which UE57Editor module owns editor commands.

2\. Whether the project already has toolbar extensions, ToolMenus, notification helpers, and logging categories.

3\. Which Unreal Engine API version is available in the project.

4\. Exact Asset Registry API signatures available for synchronous path scanning and force-rescan.

5\. Whether Asset Registry refresh alone updates Content Browser.

6\. Whether explicit Content Browser refresh/sync is needed.

7\. Whether \`/Game\` includes all desired project user content.

8\. How plugin content differs from \`/Game\`.

9\. Whether newly copied folders are discovered by recursive scanning.

10\. Behavior for:

   - valid \`.uasset\`;

   - valid \`.umap\`;

   - unsupported files;

   - assets from incompatible/newer UE versions;

   - missing dependencies;

   - redirectors;

   - large folders.

Document findings before implementation.

**## Primary feature — Refresh Content**

Add a command:

\`\`\`text

Refresh Content

\`\`\`

Recommended tooltip:

\`\`\`text

Rescan project Content (/Game) and refresh externally added assets without restarting Unreal Editor.

\`\`\`

The command should:

1\. obtain \`IAssetRegistry\`;

2\. force a recursive rescan of:

\`\`\`text

/Game

\`\`\`

3\. refresh Content Browser if required;

4\. report completion;

5\. report errors through UE logging and concise UI notification.

Conceptually:

\`\`\`cpp

IAssetRegistry& AssetRegistry = ...;

TArray\<FString\> Paths;

Paths.Add(TEXT("/Game"));

AssetRegistry.ScanPathsSynchronous(

&nbsp;&nbsp;&nbsp;&nbsp;Paths,

&nbsp;&nbsp;&nbsp;&nbsp;/\* bForceRescan = \*/ true);

\`\`\`

The exact implementation must use APIs available in the project’s actual UE version.

Do not copy signatures from another UE version without checking headers.

**## Physical Content vs virtual /Game**

Document explicitly:

\`\`\`text

\<Project\>/Content = physical filesystem directory

/Game             = mounted Unreal package path

\`\`\`

Do not scan \`/Engine\` by default.

Do not scan all mounted plugins by default.

**## Secondary feature — Refresh Current Folder**

If investigation confirms the currently selected Content Browser path can be obtained reliably using stable public APIs, add:

\`\`\`text

Refresh Current Folder

\`\`\`

Examples:

\`\`\`text

/Game/Megascans

/Game/Environment

/Game/Marketplace/MyPack

\`\`\`

If selected-path retrieval requires brittle/private APIs, defer this feature and implement only \`Refresh Content\`.

**## UI placement**

Integrate into the existing UE57Editor UI architecture.

Preferred order:

1\. existing UE57Editor toolbar;

2\. existing UE57Editor menu;

3\. Content Browser extension only if already architecturally appropriate.

Do not create a new window for one button.

Use an existing Unreal refresh icon only if a stable editor style icon is available; otherwise use text only.

**## Notifications**

Examples:

\`\`\`text

Refreshing Content...

Content refresh completed.

Content refresh completed in 1.8 s.

Content refresh failed. See Output Log.

\`\`\`

If reliable counts are available cheaply, optionally show:

\`\`\`text

Content refreshed: 37 new/updated assets detected.

\`\`\`

Do not invent counts.

**## Logging**

Use the existing UE57Editor log category if available.

Log:

\- start;

\- virtual path;

\- elapsed time;

\- success/failure;

\- optional reliable changed counts.

Avoid per-file spam.

**## Performance and responsiveness**

A full \`/Game\` scan can be expensive.

Requirements:

\- no unnecessary filesystem pre-enumeration;

\- do not scan \`/Engine\`;

\- do not scan unrelated plugin mounts;

\- avoid duplicate full scans;

\- guard against repeated clicks while refresh is running;

\- restore command availability after success/failure;

\- record elapsed time.

If verified Asset Registry APIs require synchronous execution, keep implementation simple and correct rather than introducing unsafe threading.

**## Safety**

This command is read/registry-refresh only.

It must not:

\- delete assets;

\- move assets;

\- rename assets;

\- save packages;

\- resave packages;

\- fix redirectors automatically;

\- migrate packages;

\- rewrite metadata;

\- modify source-controlled files;

\- overwrite assets.

External copying is done by the user outside UE57Editor.

**## Invalid/incompatible assets**

One bad asset must not crash the command.

Examples:

\- package created by newer UE;

\- corrupted \`.uasset\`;

\- missing plugin dependency;

\- missing referenced assets;

\- incompatible custom asset class.

Requirements:

\- rely on UE package/Asset Registry error handling;

\- concise UI status;

\- technical details in Output Log;

\- no automatic repair.

**## Module dependencies**

Investigate and add only dependencies actually required.

Possible editor modules may include:

\`\`\`text

AssetRegistry

ContentBrowser

ToolMenus

UnrealEd

Slate

SlateCore

\`\`\`

Do not add blindly.

Keep this functionality editor-only.

**## Command architecture**

Prefer existing project patterns.

Possible structure:

\`\`\`text

FUE57EditorCommands

&nbsp;&nbsp;&nbsp;&nbsp;RefreshContent

FUE57EditorModule

&nbsp;&nbsp;&nbsp;&nbsp;RegisterMenus()

&nbsp;&nbsp;&nbsp;&nbsp;ExecuteRefreshContent()

&nbsp;&nbsp;&nbsp;&nbsp;CanExecuteRefreshContent()

\`\`\`

Exact naming should match the repository.

**## Re-entrancy**

Prevent overlapping scans.

Recommended state:

\`\`\`text

bIsRefreshingContent

\`\`\`

Behavior:

\`\`\`text

idle → enabled

running → disabled

success/failure → enabled

\`\`\`

Use scope cleanup where practical.

**## Optional keyboard shortcut**

Do not assign a global shortcut by default unless the project already has a shortcut policy.

**## Testing strategy**

**### Unit/automation-test candidates**

Test helpers for:

\- \`/Game\` path normalization;

\- invalid/non-project path rejection for current-folder refresh;

\- re-entrancy state;

\- command availability;

\- path conversion if current-folder support is added.

**### Editor integration validation**

1\. Start UE with the test project.

2\. Confirm a test asset is absent.

3\. Copy a known compatible \`.uasset\` into:

\`\`\`text

\<Project\>/Content/RefreshContentTest/

\`\`\`

without restarting UE.

4\. Confirm it is not yet visible if that is current behavior.

5\. Execute \`Refresh Content\`.

6\. Confirm the asset appears.

7\. Confirm it can be selected.

8\. Confirm normal metadata is visible.

Do not use paid Marketplace/Fab assets in automated tests.

**## Additional manual scenarios**

**### New folder**

Externally create:

\`\`\`text

Content/TestPack/

\`\`\`

with valid assets, refresh, and verify the folder appears.

**### Multiple assets**

Copy several assets and refresh once.

**### Existing asset changed externally**

Investigate and document behavior.

Important distinction:

\`\`\`text

discovering a new asset

!=

safely hot-reloading an already loaded modified package

\`\`\`

This ticket primarily targets newly copied content.

**### Large folder**

Test a reasonably sized package and record approximate duration.

**## Documentation**

Add:

\`\`\`text

Docs/RefreshContent.md

\`\`\`

Document usage:

\`\`\`text

1\. Copy package/folder into \<Project\>/Content.

2\. Return to Unreal Editor.

3\. Click UE57Editor → Refresh Content.

4\. Newly discovered compatible assets appear in Content Browser.

\`\`\`

Document limitations:

\- incompatible/corrupt packages may not load;

\- already loaded modified packages are not guaranteed to hot-reload;

\- plugin content is outside \`/Game\`;

\- full \`/Game\` refresh may take time in very large projects.

**## Acceptance criteria**

\- [ ] UE57Editor contains a visible \`Refresh Content\` command.

\- [ ] Command rescans \`/Game\`.

\- [ ] Scan is forced.

\- [ ] \`\<Project\>/Content\` vs \`/Game\` distinction is documented.

\- [ ] Newly copied compatible \`.uasset\` files can appear without editor restart.

\- [ ] Newly copied compatible \`.umap\` files can be discovered where supported.

\- [ ] Newly copied subfolders can appear after refresh.

\- [ ] Content Browser is refreshed explicitly if required by verified engine behavior.

\- [ ] Repeated clicks cannot start overlapping scans.

\- [ ] Success/failure feedback is visible.

\- [ ] Technical errors go to Output Log.

\- [ ] No assets are modified, deleted, renamed, moved, or saved.

\- [ ] \`/Engine\` is not scanned.

\- [ ] Plugin content is not scanned by default.

\- [ ] Invalid/incompatible content does not crash the command.

\- [ ] Existing UE57Editor commands continue working.

\- [ ] Editor build succeeds.

\- [ ] Relevant tests/editor validation pass.

\- [ ] CRG is updated and blast radius reviewed.

\- [ ] Graphify is refreshed only if justified.

\- [ ] Investigation, plan, and implementation report are produced.

**## Nice-to-have — only if cheap and supported**

Optional:

\- \`Refresh Current Folder\`;

\- reliable detected-change counts.

Do not destabilize the primary command for these extras.

**## Explicit non-goals**

Do not implement:

\- Marketplace/Fab downloading;

\- automatic package installation;

\- file copying into Content;

\- deleting downloaded packs;

\- asset moving/renaming;

\- automatic migration;

\- hot reload of already loaded modified packages;

\- automatic redirector cleanup;

\- plugin mount refresh;

\- Engine content refresh;

\- source-control operations;

\- dependency repair;

\- package-version conversion;

\- a new standalone window.

**## Required deliverables**

\`\`\`text

Task/UE57-RefreshContent/investigation.md

Task/UE57-RefreshContent/implementation-plan.md

Task/UE57-RefreshContent/implementation-report.md

Docs/RefreshContent.md

\`\`\`

Plus required UE57Editor source/build-file changes.

**## Validation**

Codex must discover repository-supported Unreal build commands and record exact commands/results.

At minimum validate:

\- UE57Editor editor module compiles;

\- no new warnings caused by this task;

\- project opens;

\- command registers;

\- command executes;

\- externally added compatible test content is discovered without restart;

\- command safely re-enables after completion/failure.

Do not claim interactive validation unless actually performed.

**## Definition of done**

While Unreal Editor remains open, the user can externally copy a compatible package into:

\`\`\`text

\<Project\>/Content

\`\`\`

then execute:

\`\`\`text

UE57Editor → Refresh Content

\`\`\`

and Unreal rescans the project’s \`/Game\` content so newly added assets become visible without restarting the editor.

