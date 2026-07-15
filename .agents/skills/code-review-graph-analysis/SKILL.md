---
name: code-review-graph-analysis
description: Build or refresh code-review-graph and use it for exact symbol relationships, callers, callees, imports, inheritance, dependants, review context, and change-impact analysis before implementation or code review.
---

# Code Review Graph Analysis

## Repository workflow precedence

When the repository `AGENTS.md` or `.codex/PRE_TICKET_WORKFLOW.md` requires this
skill, that repository workflow takes precedence over the general guidance below.

For Level 1 and Level 2 tasks in this repository, execute this skill as part of
mandatory preflight and post-change validation.

For Level 0 tasks, this skill is normally unnecessary.

## Purpose

Use code-review-graph (CRG) as an independent structural-analysis and review aid.

CRG complements direct source inspection, Graphify, compilation, tests, static
analysis, and runtime validation. It replaces none of them.

## Primary uses

Use this skill for:

- pre-implementation impact analysis;
- changes spanning multiple symbols, files, or modules;
- local bugfix dependency checks;
- code review;
- regression-risk analysis;
- locating callers, callees, dependants, implementations, and adjacent tests;
- validating an implementation plan;
- checking whether a diff reached all structurally related code;
- comparing structural evidence with Graphify.

## Repository assumptions

Run from the repository root resolved by:

```powershell
git rev-parse --show-toplevel
```

Expected local state is normally stored under:

```text
.code-review-graph/
```

Do not assume it is valid merely because files exist.

Exclude generated code, build output, caches, databases, models, third-party
dependencies, and vendored assets unless explicitly in scope.

Typical exclusions:

- `.git/`
- `.idea/`
- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `graphify-out/`
- `.code-review-graph/`
- `node_modules/`

Inspect `Build/` before excluding it because Unreal plugin packaging metadata may
be relevant.

## Workflow

### 1. Read instructions and establish scope

1. Read applicable `AGENTS.md`.
2. Read `.codex/PRE_TICKET_WORKFLOW.md`.
3. Read the ticket, acceptance criteria, investigation, and current plan.
4. For review work, inspect the diff and changed files first.
5. Identify key symbols, likely callers, dependencies, tests, and Unreal lifecycle
   concerns.

### 2. Discover the exact project command

Search for:

- CRG configuration;
- repository wrappers;
- setup documentation;
- `.code-review-graph/`;
- evaluation reports;
- executable names such as `code-review-graph` or `crg`.

Do not invent CLI syntax.

Possible command families may include:

```powershell
code-review-graph --help
code-review-graph build
code-review-graph update --brief
code-review-graph detect-changes --brief
```

These are examples only. Confirm exact syntax against the installed tool or
repository documentation before execution.

If no supported invocation can be established:

- state that CRG invocation is not configured;
- continue with direct source analysis;
- report missing setup as a separate infrastructure issue.

### 3. Verify tool and backend

Confirm:

- executable availability;
- repository root;
- graph/index compatibility;
- embedding backend;
- database readability;
- a successful scoped query or update.

The established environment may use:

```text
all-MiniLM-L6-v2
```

CUDA may be available, but correctness must not depend on it.

Do not commit credentials, model files, private paths, or machine-specific absolute
paths.

### 4. Assess database freshness

Refresh or rebuild when:

- database is absent;
- repository identity or root changed;
- relevant files or symbols are missing;
- source changes materially exceed indexed state;
- embedding/index configuration changed;
- update/query commands fail;
- data is demonstrably stale or corrupted.

Reuse the database when it is current enough for the task.

Do not delete an existing database as the first troubleshooting step.

### 5. Build or refresh

Use the exact repository-supported command.

The build or update should:

- target the repository root;
- apply repository exclusions;
- include source, relevant tests, build configuration, and maintained docs;
- exclude generated/vendor/cache/database noise;
- provide enough diagnostics to confirm coverage.

Capture when available:

- exact command;
- repository root;
- exclusions;
- indexed file, symbol, or node counts;
- embedding model;
- warnings and failures.

### 6. Analyze the task or diff

Inspect:

- changed or proposed symbols;
- direct and indirect callers;
- callees;
- interfaces and implementations;
- factories and registration points;
- delegates and lifecycle symmetry;
- Slate callbacks and ownership;
- configuration bindings;
- serialization or persistence contracts;
- threading, ownership, async, and editor boundaries;
- public APIs;
- tests;
- error handling and logging.

For implementation or review, answer:

1. Which components depend on the changed code?
2. Which expected callers or implementations were not modified?
3. Which tests cover the behavior?
4. What regression paths are structurally adjacent?
5. Does the change violate dependency direction?
6. Are new files disconnected from normal execution?
7. Are obsolete paths still reachable?
8. Is the expected blast radius consistent with the ticket?

### 7. Validate every important result

Open actual source files and confirm:

- the relationship exists;
- directionality is correct;
- signatures and types match;
- the path is active rather than dead, conditional, generated, or test-only;
- registration and configuration make it reachable;
- Unreal lifecycle and delegate cleanup are correct;
- relevant tests assert behavior rather than only compile.

Graph proximity is not proof of runtime execution.

### 8. Compare with Graphify

When both tools are available, classify findings as:

- confirmed by both tools and source;
- found by Graphify and confirmed in source;
- found by CRG and confirmed in source;
- tool disagreement resolved by source;
- unresolved due to incomplete indexing.

Do not force agreement.

### 9. Apply results

For implementation:

- refine the change surface;
- add missing files or tests to the plan;
- identify compatibility, lifecycle, and migration concerns;
- avoid modifying graph-neighbor files without source justification.

For review:

- prioritize correctness, regressions, lifecycle, ownership, concurrency, data
  loss, API compatibility, and missing tests;
- cite exact files and lines;
- distinguish defects from optional improvements;
- do not manufacture findings.

### 10. Post-change update

After implementation:

1. run the confirmed repository-supported CRG update;
2. inspect changed symbols;
3. inspect blast radius;
4. inspect related tests and review context;
5. investigate unexpected dependants;
6. verify the change did not escape the intended subsystem.

## Required review output

When used for formal review:

```markdown
## CRG analysis

- Database status: reused | refreshed | created | unavailable
- Scope:
- Confirmed commands:
- Indexed coverage:
- Key dependency findings:
- Source validation:
- Tests examined:
- Blast radius:
- Risks:
- Limitations:
```

Each finding should include severity, evidence, consequence, and recommended
correction.

## Failure handling

If CRG fails:

1. record the exact confirmed command;
2. capture a concise error;
3. check configuration, executable availability, repository root, exclusions,
   model availability, locks, corruption, and resources;
4. preserve existing data unless deletion is explicitly authorized;
5. continue with Graphify, `rg`, source review, build, and tests;
6. state that CRG validation was unavailable or partial.

## Safety and repository hygiene

- Never expose credentials.
- Avoid machine-specific absolute paths in committed configuration.
- Do not commit large indexes or model files unless project policy requires it.
- Do not modify code merely to create cleaner graph relationships.
- Do not use CRG output to bypass tests or build validation.
- Do not claim full repository coverage without evidence.

## Definition of done

This skill is complete when:

- CRG availability and freshness were assessed;
- the exact project-supported command was used or its absence was reported;
- task- or diff-specific dependency analysis was performed;
- important relationships were verified in source;
- affected tests and structural risks were examined;
- CRG was updated after implementation when code changed;
- results and limitations were documented without overstating confidence.
