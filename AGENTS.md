# AGENTS.md

## Mandatory pre-ticket workflow

Before starting any non-trivial ticket, feature, bugfix, refactor, investigation,
implementation-planning, or review task:

1. Resolve the repository root with:

   ```powershell
   git rev-parse --show-toplevel
   ```

2. Read the repository-root file `.codex/PRE_TICKET_WORKFLOW.md`.
3. Use `$graphify-repository-analysis`.
4. Use `$code-review-graph-analysis`.
5. Execute all applicable preflight phases.
6. Do not begin implementation until repository-intelligence preflight is complete.
7. After implementation, update CRG, inspect impact radius, run validation, and
   refresh Graphify when required by the workflow.

For trivial spelling, formatting, comment-only, or metadata-only changes, the full
graph preflight may be skipped when graph context cannot affect correctness.

## Repository intelligence

This project uses two complementary systems:

1. **Graphify**
   - architectural exploration;
   - subsystem discovery;
   - semantic and cross-file relationships;
   - candidate entry-point discovery.

2. **code-review-graph (CRG)**
   - structural code analysis;
   - exact symbol relationships;
   - callers, callees, imports, inheritance, and dependants;
   - review scoping and impact analysis.

Before implementing non-trivial changes:

1. verify graph availability and freshness;
2. query Graphify for architectural context when required by the workflow level;
3. query CRG for concrete symbol dependencies;
4. validate important graph findings against source code;
5. inspect tests covering the affected subsystem.

After implementation:

1. update CRG;
2. inspect blast radius and review context;
3. run affected tests and required builds;
4. update Graphify only when architecture, subsystem boundaries, important entry
   points, or cross-module relationships changed.

## Project scope

This repository contains an Unreal Engine C++ editor plugin.

Analysis and implementation must account for:

- Unreal module startup and shutdown;
- editor modes and editor viewport behavior;
- delegates, delegate handles, subscription, broadcast, and cleanup;
- Slate widget construction, attributes, callbacks, and ownership;
- Unreal macros and reflection-style APIs;
- object lifetime, weak references, garbage collection, and editor-only state;
- coordinate transforms, bounds, landscape components, heightmap mapping, and
  marker state;
- host-project and Unreal Engine version constraints.

## Repository layout

- `Source/` — production plugin source and tests colocated with source, if any.
- `Resources/` — plugin resources and editor assets.
- `Config/` — plugin configuration.
- `Docs/` — maintained project documentation.
- `Tickets/` — ticket specifications and acceptance criteria.
- `Task/` — active task artifacts.
- `Reviews/` — code-review reports and verdicts.
- `evaluation/` — historical Graphify/CRG benchmark and comparison artifacts.
- `graphify-out/` — current generated Graphify working graph.
- `.code-review-graph/` — current generated CRG state.
- `.codex/` — Codex workflows and repository automation.
- `.agents/skills/` — repository-local Codex skills.

## Generated and non-source directories

Do not treat these as production source:

- `.git/`
- `.idea/`
- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `.code-review-graph/`
- `graphify-out/`

Inspect `Build/` before excluding it. Unreal plugin packaging metadata or resources
may be stored there.

Do not manually edit generated Unreal build artifacts.

Do not include generated, cache, database, model, or vendor directories in
Graphify or CRG indexing unless the ticket explicitly concerns them.

## Code-intelligence routing

- Use Graphify for unfamiliar subsystem orientation, broad architecture discovery,
  semantic relationships, and cross-file candidate discovery.
- Use CRG for exact symbol lookup, callers, callees, imports, inheritance,
  dependants, compact local graph facts, review context, and impact analysis.
- Treat graph tools as candidate generators.
- Do not treat graph paths, communities, inferred flows, or impact radius as
  verified execution behavior.
- Use direct `rg` searches and source inspection as the authority for exact
  callers, callees, registrations, and runtime flow.
- Verify Unreal delegates, Slate attributes, lambdas, macros, module lifecycle,
  editor-mode lifecycle, and event wiring in source before making claims.
- Use targeted exhaustive search for absence claims.
- Check graph/index freshness before trusting graph results.
- When Graphify, CRG, and source disagree, source evidence wins.

## Unreal runtime-flow verification

Prove runtime paths with source evidence such as:

- direct calls;
- delegate subscription and removal;
- delegate broadcast;
- Slate callbacks and attributes;
- widget construction and ownership;
- Unreal lifecycle registration;
- explicit state writes and reads;
- command registration;
- editor-mode activation;
- viewport event routing.

Graph connectivity alone is insufficient.

## Change safety

- Do not modify production code during investigation-only tasks.
- Do not overwrite, reset, clean, stash, revert, or otherwise destroy pre-existing
  user changes.
- Keep implementation scope aligned with the ticket.
- Avoid unrelated refactoring.
- Preserve public behavior unless the ticket explicitly changes it.
- For impact analysis, distinguish direct impact, adjacent impact, test-only
  impact, and graph-proximity noise.
- Mapper changes require inspection of forward mapping, reverse mapping, UI marker
  state, transforms, bounds handling, and tests.

## Search authority

For exact questions:

1. use `rg`;
2. inspect source;
3. verify line-level evidence;
4. inspect registration, configuration, and tests.

Graphify and CRG may suggest where to look, but source code remains authoritative.

## Build and validation

The plugin normally requires a host Unreal project and an installed Unreal Engine
version for compilation and runtime validation.

Before building, identify:

- repository root;
- host `.uproject`;
- Unreal Engine installation and version;
- target editor configuration;
- plugin installation mode;
- documented build/test wrappers.

Do not invent machine-specific Unreal paths.

Prefer repository-provided scripts when present.

For every build or test run, record:

- exact command;
- working directory;
- result;
- relevant failure output.

Do not claim a build or test passed unless it was actually executed successfully.

## Definition of done

A non-trivial ticket is not complete until:

- the applicable pre-ticket workflow was executed;
- important graph findings were validated against source;
- the smallest coherent implementation was completed;
- CRG was updated after changes;
- post-change impact was inspected;
- required tests and builds were run or explicitly reported as unavailable;
- remaining risks and unverified assumptions were documented;
- an implementation report was produced.
