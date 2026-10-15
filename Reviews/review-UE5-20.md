# Review UE5-20

## Review corrections — 2026-10-02

Reviewed baseline: `e31d5256f7751101aec87657ab423643b7526686`.
Implementation commit: `3f0f87b433cc5a17245b6278d5a199487bc5d88c`.
Corrections are in `codex/ue5-20-contour-3d-markers`; no merge was performed.

- P1 addressed: IDs now digest exact canonical segment geometry, boundary height
  and closed state, rather than spatial ordinal. Unchanged contours retain their
  identity across sibling removal/insertion; changed geometry gets a new ID and
  cannot adopt an old actor. Legacy ordinal-ID actors remain in the Outliner for
  explicit removal. GeneratedTopologyIdentity tests real generator output and
  verifies Update/Delete isolation, original transforms and Undo/Redo.
- P2 addressed: closed loops reserve the final-to-first gap. A 50m loop with
  10m boxes and 10m gaps now creates two boxes; the seam gap is 30m. Gap=0
  produces five boxes. Tests check every gap via arc distances and short loops.
- Sampling addressed: one monotonically advancing segment cursor serves all
  start/center/end samples. Complexity is O(segments + instances). DenseSegments
  compares a 15,000-segment path against the same sparse geometry, including
  corners, duplicate vertices and gap=0.
- HISM, default dimensions, Slate layout, actor lookup and transaction behavior
  are preserved. The empty-placement message now explains the closing gap.

Current validation: host editor build succeeded; ContourBoxes 9/9 succeeded;
plugin regression 40/40 completed (38 without warnings, two UI rendering checks
warned under NullRHI). Separate rendering verification and complete command
evidence are recorded in `Task/UE5-20/review-fixes-report.md`.

The initial implementation review below is retained with its historical evidence;
the corrections and current validation above supersede its identity assessment.

## Verdict

Implementation is ready for manual Unreal Editor visual verification. No blocking
source-level defects were found in the reviewed diff. Merge is intentionally not
approved yet because the ticket explicitly requires user visual verification.

## Scope reviewed

- deterministic connected-contour identity;
- accumulated-length box placement and validation;
- editor-only HISM actor state;
- exact actor lookup and transactional create/update/delete;
- Slate row wiring and enablement;
- regression and lifecycle automation tests;
- documentation and package/build behavior.

## CRG analysis

- Database status: rebuilt after implementation.
- Scope: branch changes relative to `main` plus the newly added source files.
- Confirmed commands:
  - `code-review-graph update --base main --brief`
  - `code-review-graph build`
  - `code-review-graph detect-changes --base main --brief`
- Indexed coverage: 60 files, 412 nodes, 3,684 edges.
- Key dependency finding: the new subsystem remains connected through the
  existing Slate panel and does not add a runtime-module dependency.
- Source validation: direct callbacks, service calls, mapper use, HISM rebuild,
  transactions, actor destruction, and tests were inspected in source.
- Tests examined: placement unit tests, editor lifecycle test, contour generator
  identity assertions, UI screenshot test, and the full plugin namespace run.
- Blast radius: editor panel, contour model/generator, new editor actor/service,
  documentation, and tests.
- Unexpected dependants: none.

## Correctness notes

- Default geometry is 10 m length × 2 m thickness × 10 m height with a 10 m gap
  and therefore a 20 m placement step.
- Placement is computed from accumulated world-XY distance, uses yaw only, and
  rejects incomplete short fragments and excessive instance counts.
- Lookup is keyed by both source Landscape and contour GUID, preventing one
  same-height contour from updating or deleting another.
- Identical repeated Create returns before starting a transaction or dirtying the
  level.
- Create and Delete Undo/Redo, update isolation, label/folder preservation, manual
  absence safety, same-height isolation, collision state, and editor-only state
  are covered by the lifecycle test.

## Validation evidence

- Host UE57Editor Development build: passed.
- Strict `BuildPlugin -StrictIncludes` package build: passed.
- Contour-box tests: 6/6 passed.
- Full plugin automation: 37/37 completed successfully; two UI tests reported
  expected rendering-session warnings under NullRHI.
- Offscreen rendered UI screenshot test: passed without warnings.
- `git diff --check`: passed.

## Non-blocking risks / pending checks

- Material color and world placement must be inspected on a real Landscape; the
  automation environment verifies state and transforms but not the final rendered
  3D result.
- Geometry edits intentionally create a new identity and leave old actors for
  manual Outliner removal. This conservative policy prevents ambiguous reassignment.
- PIE/cooked exclusion is implemented through editor-only actor/module behavior
  and strict packaging succeeds, but final acceptance still calls for an explicit
  manual PIE/cooked observation.

## Required manual acceptance

1. Generate at least one open and one closed contour on a real Landscape.
2. Create boxes and compare their XY path and Z base against the 2D contour.
3. Confirm default size/gap and yaw-only orientation around corners.
4. Verify Outliner label/folder and one actor/HISM per connected contour.
5. Exercise repeated Create, parameter Update, Delete, Undo, and Redo.
6. Manually delete an actor and confirm the row safely returns to Create state.
7. Verify markers are hidden in PIE and absent from cooked output.
