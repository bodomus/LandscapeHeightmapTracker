---
name: graphify-repository-analysis
description: Build, refresh, inspect, and query a repository Graphify knowledge graph before architecture investigation, planning, implementation, refactoring, debugging, or review. Use for subsystem orientation, semantic relationships, cross-file discovery, and architecture context.
---

# Graphify Repository Analysis

## Repository workflow precedence

When the repository `AGENTS.md` or `.codex/PRE_TICKET_WORKFLOW.md` requires this
skill, that repository workflow takes precedence over the general guidance below.

For Level 2 tasks in this repository, execute this skill as part of mandatory
preflight.

For Level 1 tasks, reuse or query Graphify when architecture context is relevant.
Do not blindly rebuild it.

For Level 0 tasks, this skill is normally unnecessary.

## Purpose

Use Graphify as a repository-navigation and architectural-context tool.

Graphify is supporting evidence, not an authoritative source. Confirm important
conclusions by reading source code, configuration, tests, and build files.

## Primary uses

Use this skill for:

- unfamiliar repository areas;
- architecture or subsystem analysis;
- feature implementation spanning multiple files or subsystems;
- structural refactoring;
- debugging with unclear entry points;
- impact-oriented code review;
- locating modules, classes, interfaces, services, factories, widgets, commands,
  handlers, adapters, and tests.

## Repository assumptions

Run from the repository root resolved by:

```powershell
git rev-parse --show-toplevel
```

Expected working artifacts are normally stored under:

```text
graphify-out/
```

Respect repository-specific exclusions.

Exclude generated code, build output, caches, databases, models, binaries, and
vendored dependencies unless explicitly in scope.

Typical exclusions:

- `.git/`
- `.idea/`
- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `.code-review-graph/`
- `graphify-out/`
- `node_modules/`

Inspect `Build/` before excluding it because Unreal plugin packaging metadata may
be stored there.

## Workflow

### 1. Read repository instructions

1. Read applicable `AGENTS.md`.
2. Read `.codex/PRE_TICKET_WORKFLOW.md`.
3. Read the ticket and acceptance criteria.
4. Inspect existing investigation or implementation-plan artifacts.
5. Identify likely subsystems, symbols, directories, and tests.

### 2. Verify tool availability

Confirm the installed Graphify integration or CLI.

A common check is:

```powershell
graphify --help
```

Do not assume any command, slash command, backend, update option, or wrapper exists
until confirmed by installed help or repository documentation.

If unavailable:

- report the missing prerequisite;
- continue with CRG and direct source analysis;
- do not fabricate Graphify results.

### 3. Assess graph state

Determine whether a usable graph exists.

Treat it as stale or unusable when:

- required artifacts are missing;
- graph files are unreadable;
- scoped queries fail;
- relevant source files or symbols are absent;
- architecture changed materially;
- repository or Graphify configuration changed;
- results are demonstrably inconsistent with source.

Do not rebuild solely because timestamps differ.

### 4. Build or refresh

Use the exact repository-documented command or installed skill.

For the previously established local Ollama environment, configuration may include:

```powershell
$env:OLLAMA_BASE_URL = "http://localhost:11434/v1"
$env:OLLAMA_API_KEY = "ollama"
$env:OLLAMA_MODEL = "qwen25coder14b:latest"
```

A previously successful command was:

```powershell
graphify label "." --backend ollama --batch-size 40
```

Before executing it, confirm that:

- the installed Graphify version still supports the syntax;
- the repository root is correct;
- output and exclusion behavior match this repository;
- required environment variables are already configured or explicitly permitted.

Do not silently change model, backend, context size, batch size, output directory,
or exclusions.

Avoid rebuilding when a current graph already covers the task.

### 5. Query the graph

Start from concrete ticket vocabulary:

- class and interface names;
- modules and namespaces;
- editor modes;
- delegates and callbacks;
- widgets;
- commands;
- mappers and transforms;
- configuration keys;
- error messages;
- subsystem terminology.

Use multiple focused queries.

Common confirmed command families may include:

```powershell
graphify query "<question or symbol set>"
graphify explain "<symbol or subsystem>"
graphify path "<source symbol>" "<target symbol>"
```

Confirm exact syntax before execution.

### 6. Validate against source

For every conclusion affecting implementation:

1. open referenced source files;
2. confirm names and signatures;
3. verify actual control flow;
4. verify delegate registration and cleanup;
5. verify Slate callbacks and ownership;
6. inspect tests and configuration;
7. distinguish inferred graph evidence from verified source evidence.

Important limitation:

`path` output reflects graph connectivity or a shortest graph path. It may traverse
relationships such as file → namespace → file and must not be interpreted as
runtime execution flow without source proof.

### 7. Produce a compact working set

Before editing, identify:

- relevant files;
- key symbols;
- entry points;
- subsystem boundaries;
- dependencies;
- tests;
- configuration;
- likely change surface;
- uncertainties requiring direct inspection.

Do not dump the full graph into context.

### 8. Report evidence

When producing investigation or review artifacts, record:

- graph status: reused, created, refreshed, or unavailable;
- exact confirmed commands;
- relevant queries;
- important findings;
- source files used for verification;
- graph/source discrepancies;
- limitations or incomplete coverage.

## Failure handling

If Graphify fails:

1. record the exact confirmed command;
2. capture a concise error;
3. check backend, model, environment variables, output path, context limits,
   exclusions, and corrupted artifacts;
4. do not delete existing graph data without explicit authorization;
5. continue with CRG, `rg`, source inspection, build, and tests;
6. mark Graphify validation partial or unavailable.

## Safety and repository hygiene

- Do not commit credentials or machine-specific model paths.
- Do not commit large generated graph artifacts unless repository policy requires
  them.
- Do not index generated, build, cache, database, or vendor noise.
- Do not modify production code merely to improve graph extraction.
- Do not treat communities, semantic labels, inferred flows, or graph paths as
  proof.
- Source and executable validation remain authoritative.

## Definition of done

This skill is complete when:

- Graphify availability and graph freshness were assessed;
- the graph was reused, created, or refreshed only when justified;
- focused task-specific queries were executed when available;
- important findings were verified in source;
- implementation or review used the verified working set;
- failures and limitations were reported accurately.
