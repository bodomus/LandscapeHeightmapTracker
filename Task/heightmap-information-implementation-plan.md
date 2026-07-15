# Heightmap Information Panel Implementation Plan

1. Add a pure analyzer for 8-bit and native-endian 16-bit grayscale buffers.
2. Capture PNG source format and bit depth, decode grayscale samples, and populate metadata before creating the BGRA8 preview.
3. Add a bordered `Heightmap Information` area with format, color model, bit depth, possible levels, unique levels, range, and UE compatibility.
4. Clear metadata in `ReleaseTexture` and preserve existing loading, mapping, marker, and reverse behavior.
5. Add analyzer automation tests for 8-bit, 16-bit, and invalid input.
6. Update CRG, inspect the diff, build UE57EditorEditor, and run all plugin automation tests.

