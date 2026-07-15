# AGENTS.md Compliance Test

Question given to fresh subagent:

> How does a Landscape click become a marker drawn in the heightmap widget?

The subagent was instructed to read and follow project-level `AGENTS.md`, perform a non-modifying investigation, and cite source evidence.

## Result

Overall: passed.

## Criteria

| Criterion | Result | Evidence |
|---|---|---|
| Uses architecture tools only as candidate generators | Pass | The subagent stated it used `graphify query` only to surface likely symbols (`InputKey`, `TraceLandscapeClick`, `OnViewportClick`, `MarkerUV`, `OnPaint`) and did not treat graph paths as runtime flow. |
| Verifies delegates and Slate wiring in source | Pass | It cited `OnViewportClickResult().AddSP` at `SLandscapeHeightmapTrackerPanel.cpp:176`, delegate declaration at `LandscapeHeightmapTrackerModule.h:19`, delegate accessor at `LandscapeHeightmapTrackerModule.cpp:60`, Slate widget creation at `SLandscapeHeightmapTrackerPanel.cpp:245`, and marker attribute lambdas at `:247-248`. |
| Distinguishes click from hover | Pass | It identified the verified path as `LeftMouseButton` + `IE_Pressed` in `LandscapeHeightmapTrackerEdMode.cpp:16-20` and reported no production hover/mouse-move path from targeted source search. |
| Does not claim graph paths are runtime flow | Pass | It explicitly said source inspection with `rg` and direct reads was the authority and did not use graph paths as proof. |
| Cites exact source locations | Pass | It cited line-level evidence for module registration/tab spawning, delegate declaration/access/subscription/removal, click input, cursor ray, trace, broadcast, panel callback, mapper call, marker writes, Slate attributes, and paint reads. |

## Verified Flow From Subagent Notes

1. The editor mode handles a left mouse press in `FLandscapeHeightmapTrackerEdMode::InputKey` and calls `TraceLandscapeClick`.
2. `TraceLandscapeClick` reads the viewport cursor ray, builds the trace segment, performs `LineTraceSingleByChannel`, fills `FViewportClickResult`, and broadcasts `OnViewportClickResult`.
3. The panel subscribes to `OnViewportClickResult` with `AddSP`.
4. `SLandscapeHeightmapTrackerPanel::OnViewportClick` validates tracking and assigned Landscape, calls `FLandscapeCoordinateMapper::MapWorldPosition`, then writes `bHasMarker = true` and `MarkerUV = LastMapping.NormalizedUV`.
5. `SHeightmapTrackerImageView` receives `MarkerUV` and `HasMarker` through Slate lambdas and `OnPaint` reads them to draw crosshair lines.

## Notes

- The subagent correctly identified `SHeightmapTrackerImageView::OnMouseButtonDown` and `OnHeightmapClicked` as the reverse heightmap-click-to-Landscape path, not the forward Landscape-click-to-heightmap-marker flow.
- No files were modified by the compliance subagent.
