# Height Zone Range conversion implementation plan

1. Extend the world-height cache result with Raw16 samples/range and centralize
   normalized height and local-Z helpers around Unreal's canonical conversion.
2. Retain Raw16 data in the panel next to `HeightMetersCache`; show full-map and
   selected-pixel diagnostics from the same cache entry used by Range.
3. Compare calculated PNG-derived world Z with the traced Landscape surface Z at
   the selected point and log the diagnostic sample.
4. Report inclusive Range pixel count and coverage after Apply.
5. Add focused automation coverage, update CRG, build and run the affected test
   namespaces; do not refresh Graphify unless architecture changes.

