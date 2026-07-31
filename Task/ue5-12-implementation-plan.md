# UE5-12 Implementation Plan

1. Add shared height-zone settings/result types.
2. Implement and unit-test the world-height cache builder using UE Landscape's
   canonical height decode and the existing coordinate mapper.
3. Implement and unit-test threshold masks plus interpolated Marching Squares,
   deterministic saddle handling, graph-based segment joining, closed loops, and
   open boundary contours.
4. Extend PNG loading to retain grayscale source data and invalidate/rebuild the
   cache when image, Landscape, transform, bounds, or flip options change.
5. Add Height Zone controls (`Height, m`, mode, Apply, Clear) to the panel.
6. Create/release a transient BGRA overlay texture from the generated mask.
7. Extend the existing image view to draw fill and contours in normalized fitted
   image space below hover/click markers.
8. Update documentation/test plan, refresh CRG, inspect blast radius, run automation
   tests and the UE 5.7 Editor build, and record manual-validation limitations.
9. Produce `Reviews/review-UE5-12.md`, update the YouTrack ticket, and attach the
   ticket Markdown file if the available YouTrack interface permits uploads.

