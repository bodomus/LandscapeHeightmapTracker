# UE5-20 review fixes — implementation plan

1. Replace ordinal IDs with a canonical exact-geometry digest in the generator.
   Use an explicit version namespace; retain spatial sort for display order.
   Do not adopt legacy actors or match changed geometry by proximity.
2. Reserve one gap per box around a closed contour; leave open-line placement
   unchanged. Explain empty closed placement when the box fits but its gap does not.
3. Use a monotonically advancing segment cursor for start/center/end sampling.
4. Add an editor test using actually generated same-height contours: remove the
   first, update/delete the survivor, verify both actors' identities and instances,
   Undo/Redo and regeneration after adding another earlier component. Add a
   geometry-change test that preserves the old actor and requires Create.
5. Add closed 50m perimeter seam-gap, gap=0, short-loop and dense polyline tests.
6. Update identity documentation, review report and implementation report;
   refresh CRG, inspect source-backed impact and refresh Graphify for the identity
   workflow change. Run host build, targeted tests and plugin regression; record
   exact commands/results and all unperformed checks.
7. Stage all task source, tests and report files, verify staged content and
   whitespace, commit in the selected branch and return its final SHA. No merge.
