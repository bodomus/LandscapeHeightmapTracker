---
name: implement
description: Implement a ticket end-to-end on a feature branch - preflight with code-review-graph, plan, code and tests, build and automation tests, impact check, report, push and PR. Also handles fix iterations after a REQUEST CHANGES review. Use when the owner says "/implement UE5-XX", "сделай тикет", "исправь по ревью".
argument-hint: <ticket ID, e.g. UE5-21>
disable-model-invocation: true
---

# Implement a ticket

Ticket: `$ARGUMENTS`. Follow `Docs/WORKFLOW.md` §4. Use the `unreal-editor-plugin`
and `code-review-graph` skills. Track the steps below in the task list.

## 0. Detect mode
- `Reviews/review-<ID>.md` exists with verdict `REQUEST CHANGES` → **fix iteration**:
  read the review, address every finding, append *Review iteration N* to
  `Task/<ID>/report.md`. Never edit `Reviews/`.
- Otherwise → **new implementation**.

## 1. Baseline
- `git status --short`, current branch, `git rev-parse HEAD`. If the tree is dirty with
  changes you did not make, stop and ask.
- Read `Tickets/<ID>.md`; if missing, fetch the issue from YouTrack (`get_issue`) and save it.
- Check model policy (`Docs/WORKFLOW.md` §6). If Opus is required and you are on Sonnet,
  tell the owner and suggest `/model opus` before continuing.
- New implementation: `git fetch origin main`, `git switch -c feature/<ID>-<kebab> origin/main`.
  Fix iteration: switch to the existing ticket branch and pull.
- YouTrack: State `In Progress`.

## 2. Preflight (CRG)
- `code-review-graph update` (or `uvx code-review-graph update`).
- MCP: `get_minimal_context_tool` / `semantic_search_nodes_tool` for ticket vocabulary,
  `query_graph_tool` for callers/callees/dependants of symbols likely to change,
  `get_impact_radius_tool` for the expected blast radius. Find related tests.
- Verify each decision-driving finding in source with `rg` and reading the code.
- If CRG fails: record the command and error, continue with `rg`, mark analysis degraded.

## 3. Plan
Level 2 (or non-obvious Level 1): write `Task/<ID>/plan.md`:
current vs expected behavior; root cause or gap; smallest correct change;
files/symbols to change; tests to add; validation commands; risks
(lifecycle, delegate cleanup, Slate ownership, transactions/Undo, mapper, performance).
Contested design choice → stop and ask the owner.

## 4. Implement
Smallest coherent change. Add/adjust automation tests for every acceptance criterion
that can be automated. Update `Docs/` when behavior or architecture changes.
Commit in logical steps: `feat: <ID> ...`, `test: <ID> ...`.

## 5. Validate
```
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Invoke-Validation.ps1 -TestFilter "LandscapeHeightmapTracker.<Area>"
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Invoke-Validation.ps1
```
Fix failures before moving on. UI screenshot tests need `-Rhi Offscreen`.
If the build cannot run (no engine, cloud session), say so — never claim success.

## 6. Impact
`code-review-graph update`, then `code-review-graph detect-changes --base main --brief`
(or `detect_changes_tool`). Unexpected dependants or a change escaping the intended
subsystem → stop and investigate before expanding scope.

## 7. Report — `Task/<ID>/report.md`
```markdown
# <ID> — implementation report

## Summary
## Acceptance criteria
| Criterion | Status | Evidence (test / file:line / manual) |
## Changes
## CRG
- Preflight: commands, key findings, source validations
- Post-change: blast radius, unexpected dependants
## Validation
| Command | Result | Notes |
## Not verified / risks
## Review iteration N   (fix iterations only: finding → change → evidence)
```

## 8. Publish
- `git push -u origin <branch>`.
- New implementation: `gh pr create --base main --title "<ID>: <title>"` with body:
  ticket link, summary, acceptance-criteria table, validation results, risks,
  "Visual verification required: yes/no". Fix iteration: comment on the PR with
  what changed.
- Do **not** merge, do **not** set YouTrack `To Verify` — that follows the reviewer's APPROVE.
- Final line to the owner: `🟢 PASS` / `🔴 FAIL` + PR link, and the reminder that
  review runs after CI (or `/review <ID>` in a fresh session).
