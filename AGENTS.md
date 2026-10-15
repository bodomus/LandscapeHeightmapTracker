# AGENTS.md

Tool-neutral rules for any AI agent working in this repository (Claude Code,
Codex, others). Agent-specific additions live in `CLAUDE.md` and `.claude/`.

The end-to-end process (ticket → implementation → review → merge) is defined in
[`Docs/WORKFLOW.md`](Docs/WORKFLOW.md). Read it before any non-trivial task.

Project-specific domain rules: [`Docs/PROJECT_RULES.md`](Docs/PROJECT_RULES.md).

## Project scope

This repository contains an Unreal Engine 5.7 C++ **editor-only** plugin
(`LandscapeHeightmapTracker`, module type `Editor`).

Analysis and implementation must account for:

- Unreal module startup and shutdown;
- editor modes and editor viewport behavior;
- delegates, delegate handles, subscription, broadcast, and cleanup;
- Slate widget construction, attributes, callbacks, and ownership;
- Unreal macros and reflection-style APIs;
- object lifetime, weak references, garbage collection, and editor-only state;
- transactions (`FScopedTransaction`, `Modify()`), Undo/Redo and package dirtying;
- coordinate transforms, bounds, landscape components, heightmap mapping, and
  marker state;
- host-project and Unreal Engine version constraints.

## Repository layout

- `Source/` — production plugin source; automation tests in `Source/*/Private/Tests/`.
- `Resources/`, `Config/` — plugin resources and configuration.
- `Docs/` — maintained project documentation and process (`WORKFLOW.md`, `AUTOMATION.md`).
- `Scripts/` — build, packaging and validation scripts.
- `Tickets/<ID>.md` — ticket specification and acceptance criteria (mirror of YouTrack).
- `Task/<ID>/` — working artifacts of the implementer: `plan.md`, `report.md`.
- `Reviews/review-<ID>.md` — review reports. **Written only by the reviewer.**
- `evaluation/` — historical Graphify/CRG benchmark (read-only archive).
- `.code-review-graph/` — generated CRG database (gitignored).
- `.claude/` — Claude Code skills, agents and settings.
- `.github/workflows/` — CI and automated review.

## Generated and non-source directories

Never treat these as production source and never edit them by hand:
`.git/`, `.idea/`, `.vs/`, `Binaries/`, `Build/`, `DerivedDataCache/`,
`Intermediate/`, `Saved/`, `.code-review-graph/`, `graphify-out/`.

`Build/` contains local packaging output and full copies of the plugin
(including a nested `.git`). It is gitignored and must be excluded from every
search and index.

## Code intelligence

- **code-review-graph (CRG)** is the structural index: symbols, callers, callees,
  inheritance, dependants, review context, impact radius. Prefer its MCP tools
  over reading many files — it saves context.
- **Graphify is not used** in the current workflow. Do not run or rebuild it.
  `graphify-out/` and `evaluation/` are historical.
- Graph results are *candidates*. Exact callers, registrations, delegate wiring,
  Slate attributes, lambdas, macros and lifecycle must be verified in source
  (`rg` + reading the file). When the graph and source disagree, source wins.
- Use targeted exhaustive search (`rg`) for absence claims.
- Check graph freshness before trusting it (`code-review-graph update`).

## Unreal runtime-flow verification

Prove runtime paths with source evidence: direct calls, delegate subscription
and removal, broadcast, Slate callbacks and attributes, widget construction and
ownership, lifecycle registration, explicit state writes/reads, command
registration, editor-mode activation, viewport event routing.
Graph connectivity alone is insufficient.

## Change safety

- Do not modify production code during investigation-only tasks.
- Do not overwrite, reset, clean, stash, revert, or otherwise destroy pre-existing
  user changes.
- Keep scope aligned with the ticket; no unrelated refactoring.
- Preserve public behavior unless the ticket explicitly changes it.
- Mapper changes require inspection of forward mapping, reverse mapping, UI marker
  state, transforms, bounds handling, and tests.
- Every new or changed behavior gets an automation test unless the ticket says
  otherwise or it is genuinely untestable (state why in the report).

## Git conventions

- One branch per ticket: `feature/<ID>-<short-kebab-name>` (bugs: `fix/<ID>-...`,
  process/tooling: `chore/...`). Never commit directly to `main`.
- Conventional commit prefixes: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`,
  `chore:`, `ci:`. Include the ticket ID: `feat: UE5-21 add ...`.
- Merge to `main` only through a PR, after green CI, reviewer verdict `APPROVE`
  and — when the ticket requires it — the owner's manual visual verification.
  The merge itself is done by the owner.

## Build and validation

- Engine: Unreal Engine 5.7 (Win64). Builds and tests run only on Windows with
  the engine installed; they cannot run in Linux/cloud sandboxes.
- Use `Scripts/Invoke-Validation.ps1` (build + automation tests). Machine-specific
  paths come from environment variables (`LHT_UE_ROOT`, `LHT_HOST_UPROJECT`),
  never from committed files.
- For every build or test run record the exact command, result and relevant
  failure output.
- Never claim a build or test passed unless it actually ran and passed. If it
  could not run, say so explicitly and why.

## Definition of done (implementer)

- ticket acceptance criteria addressed one by one;
- smallest coherent change, scope respected;
- CRG updated and impact radius inspected after the change;
- build and automation tests run (or explicitly reported as unavailable);
- `Task/<ID>/report.md` updated with evidence, risks and unverified assumptions;
- branch pushed and PR opened.

The ticket is **not** done until the reviewer writes verdict `APPROVE`.
