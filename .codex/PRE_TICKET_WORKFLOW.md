# PRE_TICKET_WORKFLOW.md

> Mandatory repository-intelligence workflow for Codex before every non-trivial
> ticket, bugfix, refactor, feature, investigation, implementation-planning task,
> or code review.

## 0. Purpose

This workflow defines how the repository must be investigated, implemented, and
validated.

The repository uses two complementary code-intelligence systems:

1. **Graphify**
   - architecture and subsystem exploration;
   - semantic and cross-file relationships;
   - architecture-oriented `query`, `explain`, and `path` navigation.

2. **code-review-graph (CRG)**
   - structural code graph;
   - callers, callees, inheritance, imports, and dependency analysis;
   - impact radius and review context;
   - change-aware and incremental analysis.

Neither graph is authoritative.

**Source code, tests, build output, runtime behavior, and maintained project
documentation remain the sources of truth.**

Graph-derived conclusions that affect implementation decisions must be validated
against source code.

---

## 1. Workflow levels

Classify the task before running expensive analysis.

### Level 0 — trivial

Examples:

- spelling;
- formatting;
- comment-only changes;
- metadata-only edits;
- documentation edits that cannot affect code behavior.

Required:

- read applicable repository instructions;
- inspect working-tree state;
- perform only validation relevant to the edit.

Graph preflight is not required.

### Level 1 — local change

Examples:

- isolated bugfix;
- local test correction;
- change limited to one well-understood component;
- small behavior change with narrow dependencies.

Required:

- repository status;
- CRG availability, freshness, and scoped dependency analysis;
- direct source validation;
- targeted tests;
- Graphify reuse or scoped query when architectural context is relevant;
- no blind full Graphify rebuild.

### Level 2 — structural change

Examples:

- feature;
- multi-file bugfix;
- refactor;
- lifecycle or delegate changes;
- Slate state-flow changes;
- mapper or coordinate-transform changes;
- new subsystem, module, editor mode, service, or important entry point;
- broad review or architecture investigation.

Required:

- full Graphify preflight;
- full CRG preflight;
- investigation;
- implementation plan;
- source validation;
- post-change CRG update and impact analysis;
- targeted and broader validation;
- Graphify refresh after implementation when architecture changed.

When uncertain between Level 1 and Level 2, choose Level 2.

---

## 2. Mandatory execution order

For every non-trivial task, execute the applicable phases in this order:

1. Read repository instructions.
2. Record repository and working-tree state.
3. Classify the workflow level.
4. Verify Graphify availability and graph health when required.
5. Create or refresh Graphify only when required.
6. Verify CRG availability and graph health.
7. Create or update CRG when required.
8. Collect ticket-specific context from applicable graph tools.
9. Validate important findings against source code.
10. Inspect relevant tests and build configuration.
11. Produce investigation and implementation-plan artifacts for Level 2 tasks.
12. Implement the ticket.
13. Update CRG after implementation.
14. Inspect blast radius and review context.
15. Run targeted tests, then broader validation when justified.
16. Refresh Graphify only when architecture or subsystem structure changed.
17. Produce a concise implementation report.

Do not skip directly to implementation.

---

## 3. Phase A — repository instructions and baseline

Before repository-wide search or implementation:

1. Resolve the repository root:

   ```powershell
   git rev-parse --show-toplevel
   ```

2. Read the root `AGENTS.md`.
3. Read any nested `AGENTS.md` files applicable to target files.
4. Read ticket-specific user instructions.
5. Identify:
   - repository root;
   - current branch;
   - current commit;
   - dirty working-tree state;
   - likely solution/project/module boundaries;
   - documented build and test commands;
   - host Unreal project, when required.

Record the initial state.

Suggested baseline commands:

```powershell
git rev-parse --show-toplevel
git branch --show-current
git rev-parse HEAD
git status --short
```

Do not overwrite, revert, clean, stash, reset, or otherwise destroy pre-existing
user changes.

---

## 4. Phase B — Graphify preflight

Follow `$graphify-repository-analysis`.

### 4.1 Verify availability

Confirm the installed Graphify integration or CLI.

A common check is:

```powershell
graphify --help
```

Treat command examples in this document as illustrative until confirmed against
the installed tool or a repository wrapper.

If Graphify is unavailable:

1. do not silently install or upgrade global software;
2. report the missing prerequisite;
3. continue with CRG and direct source inspection;
4. mark Graphify analysis unavailable in the final report;
5. do not fabricate graph findings.

### 4.2 Verify graph existence and usability

Expected working artifacts are normally under:

```text
graphify-out/
```

They may include:

```text
graphify-out/
├── graph.json
├── GRAPH_REPORT.md
└── graph.html
```

At minimum, verify that the persisted graph exists, is readable, and can answer a
scoped query.

If the graph is absent or unusable:

1. use the exact repository-documented Graphify command or installed skill;
2. target the repository root;
3. apply repository exclusions;
4. exclude generated, vendor, build, cache, database, and model directories;
5. verify successful query behavior after creation.

Do not execute undocumented slash commands or guessed CLI options.

### 4.3 Assess freshness

Refresh Graphify when one or more conditions apply:

- no graph exists;
- graph files are unreadable;
- Graphify queries fail;
- the graph predates major architectural changes relevant to the task;
- a new subsystem, module, plugin, application, or important entry point was added;
- important ticket-relevant files or symbols are absent;
- a prior refactor materially changed subsystem boundaries;
- repository instructions explicitly require refresh.

Do not rebuild solely because timestamps differ.

Prefer incremental update only when the installed tool or repository wrapper
explicitly supports it.

Do not invent backend credentials or switch model/provider without permission.

### 4.4 Collect ticket context

For Level 2 tasks, collect at least:

1. owning subsystem;
2. central concepts and symbols;
3. important cross-file or cross-module relationships;
4. likely entry points and boundaries;
5. architecture explanation for the main concept named in the ticket.

Use focused queries rather than dumping the entire graph.

Use `path` only as navigation assistance.

**Never treat a shortest graph path as proof of runtime execution flow.**

---

## 5. Phase C — CRG preflight

Follow `$code-review-graph-analysis`.

### 5.1 Discover the exact project command

Search the repository for:

- CRG configuration;
- wrapper scripts;
- documented CLI commands;
- `.code-review-graph/`;
- prior evaluation reports;
- setup instructions.

Do not invent CLI syntax.

The command names below are examples only and must be confirmed before execution:

```powershell
code-review-graph --help
code-review-graph build
code-review-graph update --brief
code-review-graph detect-changes --brief
```

If CRG is unavailable:

1. do not silently install or upgrade global software;
2. report the missing prerequisite;
3. continue with Graphify and direct source inspection;
4. mark CRG analysis unavailable;
5. do not fabricate callers, dependants, or blast radius.

### 5.2 Verify or create the CRG graph

Preferred behavior:

1. identify the exact repository-supported CRG command;
2. try to query or update the existing graph;
3. if missing, uninitialized, corrupted, or incompatible, build it;
4. verify a scoped analysis command succeeds.

Do not assume the graph is fresh merely because `.code-review-graph/` exists.

A successful update or query is stronger evidence than file existence.

### 5.3 Collect scoped context

Collect, as applicable:

- architecture overview;
- exact symbols;
- callers and callees;
- imports;
- inheritance;
- dependants;
- related tests;
- minimal review context;
- impact radius;
- execution-flow candidates when supported.

For a bugfix or feature, answer:

1. Which symbols are likely to change?
2. Who calls them?
3. What depends on them?
4. Which tests cover the area?
5. What files form the minimal review context?
6. What is the expected blast radius?

Do not dump the entire graph into context.

---

## 6. Phase D — merge graph evidence and source evidence

Create a working model with four layers.

### Layer 1 — ticket intent

- What behavior is requested?
- What is the acceptance condition?
- What must not change?

### Layer 2 — Graphify architecture view

- owning subsystem;
- architectural concepts;
- cross-file or cross-module relationships;
- likely entry points;
- data and control boundaries.

### Layer 3 — CRG structural view

- concrete symbols;
- callers and callees;
- dependants;
- inheritance;
- imports;
- tests;
- likely impact radius.

### Layer 4 — source validation

Open the minimum necessary source files and verify:

- graph conclusions;
- actual control flow;
- actual data flow;
- delegate wiring;
- lifecycle registration and cleanup;
- Slate callbacks and attributes;
- state ownership;
- configuration;
- test behavior.

When graph output and source disagree, source wins.

Record relevant discrepancies.

---

## 7. Mandatory investigation questions

For non-trivial tasks, answer:

1. What is the current behavior?
2. What is the expected behavior?
3. What is the root cause, missing capability, or architectural gap?
4. What is the smallest correct change?
5. Which symbols are directly affected?
6. Which callers or dependants may be indirectly affected?
7. Which tests already cover the area?
8. Which tests are missing?
9. Which build/test commands validate the change?
10. Is there graph/source disagreement requiring caution?
11. Does Unreal lifecycle, delegate cleanup, Slate ownership, or viewport routing
    introduce additional risk?
12. For mapper changes, are forward mapping, reverse mapping, transforms, bounds,
    marker state, and tests covered?

For Level 2 tasks, produce:

```text
investigation.md
implementation-plan.md
```

unless the ticket defines different artifact names.

---

## 8. Implementation rules

During implementation:

1. follow all applicable `AGENTS.md` instructions;
2. preserve user changes;
3. keep scope aligned with the ticket;
4. prefer the smallest coherent change;
5. avoid unrelated refactors;
6. preserve public behavior unless change is explicitly required;
7. add or update tests for changed behavior;
8. re-query CRG when unexpected dependencies appear;
9. re-query Graphify when an unexpected subsystem boundary appears;
10. validate risky assumptions directly in source;
11. verify Unreal delegate cleanup and lifecycle symmetry;
12. verify Slate ownership and callback lifetimes;
13. verify coordinate transforms and bounds for mapper-related changes.

---

## 9. Post-implementation CRG validation

After code changes:

1. run the exact repository-supported CRG update command;
2. inspect changed symbols and review context;
3. inspect blast radius;
4. identify related tests and suspicious untested paths.

Required questions:

1. What changed?
2. What is the blast radius?
3. Which callers or dependants are affected?
4. Which tests should run?
5. Are there suspicious untested paths?
6. Did the change escape the intended subsystem?
7. Did new code become disconnected from the normal execution path?
8. Are obsolete paths still reachable?

If blast radius is larger than expected:

- stop expanding implementation blindly;
- inspect additional affected files;
- determine whether the change should be narrowed;
- add missing validation where necessary.

---

## 10. Testing order

Run validation from narrowest to broadest:

1. directly affected unit or automation tests;
2. subsystem/module tests;
3. relevant integration or editor tests;
4. plugin or host-project build;
5. broader solution/project validation when justified;
6. manual Unreal Editor validation when required.

Do not claim a test passed unless it was actually executed successfully.

If a test or build cannot run, state:

- what was not run;
- why;
- what alternative evidence exists;
- what risk remains.

---

## 11. Post-implementation Graphify refresh policy

Do not blindly rebuild Graphify after every ticket.

Refresh or update it when changes affect:

- architecture;
- subsystem boundaries;
- module/plugin/application structure;
- major cross-module relationships;
- important entry points;
- public workflows;
- large refactors.

For a small local bugfix with no architectural effect, Graphify refresh is normally
unnecessary.

CRG should be updated more aggressively because it supports change impact and
review context.

---

## 12. Failure handling

### Graphify failure

1. record the exact confirmed command;
2. capture the concise failure;
3. do not fabricate results;
4. continue with CRG and source inspection;
5. report degraded analysis.

### CRG failure

1. record the exact confirmed command;
2. capture the concise failure;
3. do not fabricate dependencies or impact radius;
4. continue with Graphify and source inspection;
5. report degraded impact analysis.

### Both tools fail

1. continue with repository instructions and direct source analysis;
2. state that graph-assisted analysis was unavailable;
3. avoid graph-derived confidence claims;
4. do not block a safe task solely because optional tooling failed, unless the user
   explicitly requires successful graph execution before work.

---

## 13. Required implementation report

At the end of a non-trivial ticket, produce:

```markdown
# Implementation Report

## Ticket
<ticket id and summary>

## Workflow
- Level: 1 / 2
- Graphify skill: used / unavailable / not required
- CRG skill: used / unavailable
- Working tree before changes: clean / dirty

## Preflight
- Graphify graph: existing / created / updated / unchanged / unavailable
- CRG graph: existing / created / updated / unavailable
- Repository root:
- Branch:
- Initial commit:

## Investigation
- Current behavior:
- Expected behavior:
- Root cause or implementation gap:
- Main symbols:
- Owning subsystem:
- Expected blast radius:

## Changes
- ...

## Graph validation
- Graphify findings used:
- CRG findings used:
- Source validations performed:
- Graph/source discrepancies:

## Post-change impact
- CRG updated: yes / no
- Blast radius:
- Unexpected dependants:
- Related tests:

## Validation
- Build:
- Targeted tests:
- Broader tests:
- Manual validation:

## Remaining risks
- ...
```

---

## 14. Non-negotiable rules

1. Do not start non-trivial implementation before applicable preflight is complete.
2. Do not assume graph databases exist or are fresh.
3. Do not invent commands, backend settings, credentials, or graph findings.
4. If a required graph is missing, create it using confirmed project commands.
5. If a graph is stale for the task, update it using confirmed project commands.
6. Validate important graph conclusions against source code.
7. Do not use Graphify shortest paths as proof of runtime flow.
8. Use Graphify primarily for architecture and candidate relationships.
9. Use CRG primarily for structural dependencies, review context, and impact.
10. Update CRG after implementation before final review.
11. Do not destroy unrelated user changes.
12. Do not declare completion without validation evidence.
13. When graph tools disagree with source, source wins.
14. Explicit user instructions override this workflow.
15. Applicable `AGENTS.md` instructions apply after explicit user instructions.

---

## 15. Ticket execution handoff

After completing all applicable preflight phases, continue with the ticket supplied
in the current task.

The ticket defines **what to implement**.

This workflow defines **how the repository must be investigated, implemented, and
validated**.
