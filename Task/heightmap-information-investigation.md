# Heightmap Information Panel Investigation

## Workflow

- Level: 2 (new Slate UI state plus image-data analysis).
- Baseline: branch `main`, commit `465718261b4a83ced74f813bb342c85decf04a02`.
- Existing UE5-11 working artifacts and untracked `images/` were preserved.
- Graphify 0.9.8 reused the current graph and identified `SLandscapeHeightmapTrackerPanel::LoadPngTexture`, `Construct`, and `GetImageInfoText` as the owning path.
- CRG 2.3.6 updated successfully: 146 nodes, 1009 edges, 25 files.

## Current behavior

`LoadPngTexture` detects PNG, decodes it directly to BGRA8 for preview, and stores only resolution and the `PNG` label. Source bit depth, color model, value range, and unique grayscale values are discarded.

## Required behavior

After loading a PNG, show source color model, per-channel bit depth, maximum possible grayscale levels, actual unique grayscale values, value range, and UE Landscape compatibility. Both 8-bit (256 levels) and 16-bit (65,536 levels) analysis must be exact.

## Design

- Decode a grayscale sample buffer at the source bit depth before BGRA8 preview conversion.
- Analyze samples in a small independent helper so 8-bit and 16-bit behavior is automation-testable.
- Preserve source format separately. For color PNGs, report that values were obtained after grayscale conversion and mark the source as not recommended for UE Landscape.
- Treat 16-bit single-channel grayscale PNG as the recommended UE Landscape input.
- Reset all metadata with texture release so stale information cannot survive failed/replaced loads.

## Expected blast radius

Direct: panel header/cpp, new analyzer helper, new analyzer tests. Adjacent: ImageWrapper decoding and existing panel load/display path. Mapper, hover, click, reverse marker, and EdMode behavior are out of scope.

