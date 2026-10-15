# Project rules

> Draft. Content to be agreed with the owner. Rules here are specific to the
> LandscapeHeightmapTracker domain and override general guidance in `AGENTS.md`
> when they conflict.

## Coding conventions
- Follow the Unreal Engine coding standard (prefixes `F`/`U`/`A`/`S`/`E`/`I`/`T`,
  PascalCase, `TObjectPtr`/`TWeakObjectPtr` for UObject references).
- Keep pure logic (mappers, generators, placement math) free of Slate and
  editor-world dependencies so it stays unit-testable.
- Automation test names: `LandscapeHeightmapTracker.<Area>.<Case>`.

## Domain rules
- _TODO: coordinate conventions (heightmap pixel ↔ landscape local ↔ world)._
- _TODO: units (cm in UE vs. m in UI), default values policy._
- _TODO: UI layout / Slate style rules for the panel._
- _TODO: what counts as "visual verification required" for a ticket._

## Out of scope / forbidden
- _TODO._
