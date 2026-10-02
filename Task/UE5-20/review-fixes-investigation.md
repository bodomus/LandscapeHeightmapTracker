# UE5-20 review fixes — investigation

Baseline: `e31d5256f7751101aec87657ab423643b7526686`, branch
`codex/ue5-20-contour-3d-markers`, clean working tree. User selected this branch
after the request also mentioned master; local master does not exist.
Workflow level: 2 (multi-file correction of identity and placement).

## Preflight and evidence

- Read root AGENTS.md, PRE_TICKET_WORKFLOW.md and both repository graph skills.
- Graphify help confirmed commands; reused the graph because all relevant
  generator/service/actor/panel symbols were present. Queries:
  `graphify query 'ContourBoxesEditorService ContourBoxPlacement HeightZoneGenerator' --budget 1400`,
  `graphify explain FHeightContour`, `graphify explain GenerateContours`.
- Graphify identifies the contour data shared by generator, range generator,
  panel and service. Source confirms the chain: GenerateContours ->
  FHeightContour.Id -> panel FindActor/CreateOrUpdate/Delete -> actor Matches.
- CRG incremental MCP update at baseline rebuilt repository identity:
  68 files, 701 nodes, 5141 edges, no errors. Scoped impact for generator and
  placement reported 19 changed nodes, 6 impacted nodes, 2 adjacent files,
  7 unresolved call sites. Query `callers_of GenerateContours` returned zero
  despite explicit calls in HeightZoneGenerator.cpp: source overrides that
  incorrect absence claim. CreateOrUpdate name is ambiguous between declaration
  and definition; direct source confirms the UI and test calls.
- Build/ contains generated packages/TestHost and archives, no production code
  implicated here; indexing exclusions remain in force.

## Findings and correction scope

1. MakeContourId uses quantized height + spatial ordinal. Removing the earlier
   same-height component assigns its ID to the surviving component. Actor
   matching uses only Landscape + GUID and can therefore mutate the wrong actor.
   Replace ordinal identity with a namespaced geometry digest: height, closed
   flag and exact canonical undirected UV segments. Preserve unchanged contour
   identities across sibling insertions/removals and traversal changes. Any
   geometry change conservatively requires a new actor; never fuzzy-match old
   shapes. Existing ordinal-ID actors are intentionally not adopted.
2. Closed placement uses the open-line count formula. A 50m perimeter with
   10m boxes and 10m gaps produces three boxes with zero gap across the seam.
   Closed count must be floor(perimeter / (box + gap)); open count stays intact.
   Short closed lines require room for a complete box plus its seam gap.
3. SamplePolyline scans from segment zero three times per box. All requested
   distances are monotonic, including gap=0: use a shared forward segment cursor.

Direct impact: generator, placement, service empty-result diagnostic and tests.
Adjacent impact: range generator, panel actor enablement, actor transactions.
No HISM layout, settings, mapper transforms, UI layout or transaction semantics
need to change. Validate existing lifecycle/Undo/Redo and complete plugin regression.

## Environment and validation

Host: `J:/Projects/UE_Projects/UE57Editor/UE57Editor.uproject`, EngineAssociation
5.7, target UE57EditorEditor, Win64 Development. Verified Build.bat and
UnrealEditor-Cmd.exe under `C:/Program Files/Epic Games/UE_5.7/Engine`.
Plugin is installed in the host's Plugins directory. No dedicated build/test
wrapper; prior report documents the host editor build and automation prefixes.
An existing UnrealEditor process was observed and must not be closed silently.
Run build, ContourBoxes automation, full `LandscapeHeightmapTracker.` regression.
Manual scene inspection and cooked exclusion remain distinct unperformed checks.
