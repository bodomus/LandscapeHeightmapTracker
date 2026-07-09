# Changelog

## Unreleased

- Added 2D heightmap click to 3D Landscape surface tracking with an editor-only vertical viewport line.
- Added reverse UV-to-local mapping, fitted image hit testing, and assigned-Landscape vertical multi-trace helpers.
- Added automation coverage for reverse mapping, fitted image clicks, and vertical trace segment construction.
- Fixed orthographic editor viewport click traces by using projection-aware trace segment construction.
- Added viewport trace automation tests for Perspective, Orthographic, reversed direction, and invalid direction cases.
- Documented orthographic viewport validation scenarios.

## 0.1.0

- Added initial Unreal Engine 5.7 Editor-only plugin structure.
- Added dockable Slate tab under `Tools -> Landscape Heightmap Tracker`.
- Added selected Landscape assignment, PNG loading, click tracking, UV/pixel diagnostics, Flip X/Y controls, and marker overlay.
- Added pure coordinate mapping automation tests.
- Added architecture, installation, and manual test documentation.
