# UE5-20 — review corrections report

Date: 2026-10-02. Status: 🟢 PASS for implemented fixes, build and automation.
Branch: `codex/ue5-20-contour-3d-markers`.
Baseline: `e31d5256f7751101aec87657ab423643b7526686`.
Implementation commit: `3f0f87b433cc5a17245b6278d5a199487bc5d88c`.
Final SHA is returned in the completion response. Resolve the containing commit
with `git log -1 --format=%H -- Task/UE5-20/review-fixes-report.md`.
No merge or push performed.

## Workflow and investigation

Level 2; initial working tree clean. Both repository graph skills were applied.
Preflight findings and correction plan are preserved in
`review-fixes-investigation.md` and `review-fixes-plan.md`.
Repository and command working directory:
`J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
Host: `J:/Projects/UE_Projects/UE57Editor/UE57Editor.uproject`.
Engine: verified installed Unreal Engine 5.7.4, Win64 Development editor target
`UE57EditorEditor`. Plugin is installed directly in the host project.

## Corrections

### P1 — safe actor identity

MakeContourId now derives a GUID from a versioned digest of exact boundary
height, closed flag and canonical undirected UV segments. It does not include
the contour ordinal, label, color or settings. Segment endpoint normalization
and sorting ignore traversal direction and the start point of a closed trace;
coordinates are hashed without lossy height/position rounding or struct padding.

Removing/inserting a sibling cannot transfer its identity to an unchanged
contour. Changes to the contour geometry conservatively get a new identity;
Update requires an existing actor for that identity, Delete of a missing identity
changes nothing, and Create produces a new set. No heuristic proximity matching
or automatic legacy actor adoption is introduced.

The new GeneratedTopologyIdentity editor test uses actual generator output for
two same-height islands. It removes the first island, checks the survivor ID,
updates/deletes only the second actor, verifies the first actor's identity and
complete world transforms, and exercises Undo/Redo. It also checks insertion
of an earlier sibling, rejection of Update/Delete against a changed shape,
and explicit creation/deletion of a new set while preserving old sets.

Legacy height/ordinal-ID actors and actors belonging to changed geometry remain
untouched. Users remove these old sets explicitly in the Outliner; current rows
cannot accidentally update/delete them. This is the intended conservative policy.

### P2 — gap across the closed seam

Closed count is now `floor(perimeter / (BoxLength + GapLength))`. The open-line
formula is unchanged. A 50m perimeter with a 10m box and 10m gap produces two
boxes; the remaining gap through the seam is 30m, at least the requested 10m.
Gap=0 produces five boxes. ClosedSeamGap verifies all neighbor/seam arc gaps,
repeated/non-repeated closure vertices, sub-box loops, loops that fit a box but
not its closing gap, exactly one box+gap and gap=0 short-loop behavior.
The empty-placement diagnostic now includes the required closing gap.

### Sampling cost

SamplePolyline accepts one shared forward cursor. All start/center/end samples
have nondecreasing arc distance, including coincident samples for gap=0.
Each segment is traversed at most once: O(segments + instances) sampling rather
than repeated linear scans. DenseSegments compares transforms on a 15,000-segment
path and its three-segment equivalent, including corners, duplicated vertices
and zero gap. No timing threshold or performance benchmark claim is made.

HISM, defaults, yaw-only orientation, height offset, instance limit, Landscape
matching, transaction handling, user labels/folders and UI layout are preserved.
Documentation and the existing implementation/review reports were updated.
The duplicated ContourBoxes test-plan entries were consolidated.

## Graph validation and impact

- CRG preflight incremental update rebuilt identity at the baseline: 68 files,
  701 nodes, 5141 edges, no errors. Post-change updates were run with base
  `e31d525`; after staging, the new test file was indexed too, without errors.
- The implementation commit's hook completed indexing but the CLI's rich output
  failed with UnicodeEncodeError under cp1251. The commit succeeded. A subsequent
  MCP update returned status ok and no graph errors; the final report commit uses
  PYTHONUTF8=1 for its hooks. This was an output-encoding issue, not a build/test
  failure. No repository-wide encoding configuration was changed.
- Post-change review context: medium risk, seven impacted nodes in one adjacent
  file when querying the three production files. Final radius for all five
  changed source/test files: 38 directly changed nodes, no additional resolved
  impacted nodes, 19 unresolved call sites. This zero does not prove isolation.
- Source explicitly confirms calls from Generate to GenerateContours, the range
  generator to Generate, panel callbacks to CreateOrUpdate/Delete/FindActor,
  service to placement and actor identity/transaction APIs. These source-backed
  paths define the impact; no public headers or lifecycle registration changed.
- CRG `callers_of GenerateContours` returned zero despite explicit source calls;
  this discrepancy was recorded and source took precedence. Graph test-gap
  heuristics also do not replace executed Unreal automation coverage.
- Graphify preflight reused covered symbols. Post-change `graphify update .`
  initially failed with WinError 5 in the sandbox; permitted execution succeeded:
  148 files extracted, 1922 nodes, 2802 edges, 144 communities. Queries for
  MakeContourId and SamplePolyline succeeded after refresh. Existing graph data
  was backed up by Graphify itself. No credentials/provider/model changes.
- Direct impact: identity, placement and empty-result diagnostic. Adjacent
  impact: height ranges, panel actor enablement and actor transactions. Tests
  cover placement, generated identity and lifecycle. Broader plugin regression
  covers mapper, ranges, viewport, paint-layer, refresh-content and ScanVault.

## Commands and actual results

All commands below ran from the repository directory stated above. Process exit
code was 0 for the build and each automation run; test counts come from exported
`index.json` reports, not only the process exit code.

### Host build — 🟢 PASS

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development '-Project=J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReloadFromIDE
```

18 actions, including compilation of the new test file, linking the plugin and
post-build packaging; `Result: Succeeded`, 47.19 seconds.
Build log preserved at `Saved/UE5-20-review-fixes/build.log`.

### ContourBoxes — 🟢 PASS

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoP4 -NoSplash -NullRHI -NoSound '-ExecCmds=Automation RunTests LandscapeHeightmapTracker.ContourBoxes' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\ContourBoxes' '-AbsLog=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\contourboxes.log' -stdout -FullStdOutLogOutput
```

9 succeeded, 0 warnings, 0 failed, 0 not run. Includes the three new tests:
ClosedSeamGap, DenseSegments and GeneratedTopologyIdentity.

### Plugin regression — 🟢 PASS

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoP4 -NoSplash -NullRHI -NoSound '-ExecCmds=Automation RunTests LandscapeHeightmapTracker.' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\Regression' '-AbsLog=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\regression.log' -stdout -FullStdOutLogOutput *> 'Saved/UE5-20-review-fixes/regression-stdout.log'
```

40 tests completed: 38 succeeded without warnings, two succeeded with warnings,
0 failed, 0 not run. MultipleHeightRangesScreenshot and PaintLayersTabSingleton
warned that their rendering checks require a rendering-capable session.

### Rendered UI checks — 🟢 PASS

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoP4 -NoSplash -RenderOffscreen -NoSound '-ExecCmds=Automation RunTests LandscapeHeightmapTracker.UI.' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\UI' '-AbsLog=J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\Saved\UE5-20-review-fixes\ui.log' -stdout -FullStdOutLogOutput *> 'Saved/UE5-20-review-fixes/ui-stdout.log'
```

Both UI tests succeeded with rendering enabled: 2 succeeded, 0 warnings,
0 failed, 0 not run. This covers the two checks bypassed in the NullRHI run.
The targeted startup log separately recorded an AssetRegistry cache-copy error;
no test errors occurred. Full startup logs are retained for inspection.

`git diff --check` and `git diff --cached --check` succeeded. New source, test,
investigation, plan and report files are included in the commit; generated graphs,
logs, caches and binaries are excluded according to repository policy.

## Explicitly unperformed checks and remaining risks

- No manual real-Landscape scene inspection of alignment, color, corner gaps or
  Outliner operations. Arc-length gap guarantees allow the pre-existing permitted
  visual overlap at sharp corners; arbitrary 3D box collision spacing is not claimed.
- No manual PIE or cooked-output observation.
- No fresh strict standalone BuildPlugin/StrictIncludes run in this correction
  task. The host build and its normal post-build archive did run; historical
  strict packaging results in the original report are not new validation.
- No timings asserting performance improvement; the complexity change and dense
  path correctness are verified in source and automation.
- Geometry edits intentionally orphan old sets until explicit Outliner removal.
  The already running interactive editor was not closed or restarted; its loaded
  DLL may require restart to use the newly built changes.

UE5-20 is ready for verification, not merge. A successful fix/automation verdict
does not substitute for the ticket's remaining user visual acceptance.
The YouTrack State field was successfully updated to `To Verify`.
