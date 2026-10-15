# Development workflow

Replaces the former `.codex/PRE_TICKET_WORKFLOW.md`. Applies to every agent.

```text
 Owner: requirement in plain language
   │
   ▼
 /ticket ──► grill (clarifying questions) ──► Tickets/<ID>.md + YouTrack issue (State: Open)
   │
   ▼
 /implement <ID>   (local Claude Code on Windows, Sonnet)
   │  branch feature/<ID>-…, State: In Progress
   │  preflight (CRG) → Task/<ID>/plan.md → code + tests
   │  Invoke-Validation.ps1 → CRG update/impact → Task/<ID>/report.md
   │  push + PR
   ▼
 CI (self-hosted Windows runner: build + automation tests)
   │
   ▼
 Reviewer (independent agent, Opus, clean context)
   │  writes Reviews/review-<ID>.md, posts PR comment, YouTrack comment
   ├── REQUEST CHANGES ─► /implement <ID> (fix iteration) ─► CI ─► Reviewer …
   └── APPROVE ─► YouTrack State: To Verify ─► owner verifies manually ─► owner merges ─► Done
```

## 1. Roles

| Role | Who | Model | Writes |
|---|---|---|---|
| Ticket author | `/ticket` skill | Opus | `Tickets/<ID>.md`, YouTrack issue |
| Implementer | main Claude Code session, `/implement` | Sonnet (escalate to Opus, see §6) | code, tests, `Task/<ID>/plan.md`, `Task/<ID>/report.md` |
| Reviewer | `reviewer` agent / `claude-review.yml` | Opus | `Reviews/review-<ID>.md`, PR comment, YouTrack comment |
| Owner | human | — | approves tickets, manual verification, merge |

**Independence rule.** The implementer never writes to `Reviews/` and never marks
a reviewer finding as resolved. The implementer answers findings in
`Task/<ID>/report.md` (section *Review iteration N*). Only the reviewer, after
re-checking, closes a finding and sets the verdict.

## 2. Ticket (`/ticket`)

1. Grill the requirement (see `grill-me` skill) until goal, scope, non-goals,
   acceptance criteria, tests and manual checks are unambiguous. Stop and ask on
   any contested point.
2. Write `Tickets/<ID>.md` using the template in the `ticket` skill.
3. Create the YouTrack issue in project **UE5** with the same text as the
   description (the MCP connector cannot attach files, so the full markdown goes
   into the description). Rename the file to the real ID.
4. Commit the ticket file on `main`-based branch only together with the
   implementation; do not push tickets directly to `main`.

## 3. Workflow levels

| Level | Examples | Required |
|---|---|---|
| 0 trivial | typo, comment, formatting, metadata | working-tree check; relevant validation only |
| 1 local | isolated bugfix, one component | CRG scoped analysis, source validation, targeted tests, build |
| 2 structural | feature, multi-file fix, lifecycle/delegate/Slate/mapper change, new subsystem | full CRG preflight, `plan.md` before coding, full test run, CRG impact after |

When unsure between 1 and 2, choose 2.

## 4. Implementation (`/implement <ID>`)

1. **Baseline.** `git status`, branch, HEAD. Never destroy user changes.
   Create `feature/<ID>-<name>` from up-to-date `main`. YouTrack: `In Progress`.
2. **Preflight (CRG).** `code-review-graph update`, then via MCP:
   minimal/review context for the ticket vocabulary, callers/dependants of the
   symbols likely to change, related tests, expected impact radius.
   Verify every finding that drives a decision in source.
3. **Plan** (Level 2, or Level 1 when non-obvious): `Task/<ID>/plan.md` —
   current vs expected behavior, root cause / gap, smallest correct change,
   affected symbols, tests to add, validation commands, risks
   (lifecycle, delegate cleanup, Slate ownership, transactions, mapper).
4. **Implement** the smallest coherent change + automation tests.
5. **Validate.** `Scripts/Invoke-Validation.ps1` (build + tests). Narrow filter
   first, then the full `LandscapeHeightmapTracker.` namespace.
6. **Impact.** `code-review-graph update` + `detect-changes --base main --brief`.
   Unexpected dependants → stop and investigate before expanding the change.
7. **Report.** `Task/<ID>/report.md` (template in the `implement` skill).
8. **Publish.** Commit, push, open PR `<ID>: <title>` linking the ticket.
   Do not merge.

## 5. Review

Triggered automatically after a green CI run on the PR (see `Docs/AUTOMATION.md`)
or locally with `/review <ID>` in a **fresh** session. The reviewer receives only
the ticket ID / PR number — never the implementer's reasoning.

Verdicts: `APPROVE`, `REQUEST CHANGES`, `BLOCKED` (cannot review: missing build
evidence, CI red, unclear ticket).

On `REQUEST CHANGES` the owner (or automation) starts `/implement <ID>` again;
the implementer fixes and appends *Review iteration N* to `report.md`.
The reviewer re-reviews the whole diff, not only the fixes.

## 6. Model policy

- Implementation runs on **Sonnet** (`.claude/settings.json`).
- Ticket writing, planning of Level 2 tasks and review run on **Opus**.
- Escalate implementation to Opus (`/model opus`) when:
  - the ticket changes coordinate mapping/transforms, module/editor-mode
    lifecycle, delegate wiring or transactions/Undo; or
  - the reviewer returned `REQUEST CHANGES` with a blocking (P0/P1) finding
    twice for the same ticket.

## 7. YouTrack states (project UE5)

`Open` → `In Progress` (implementation started) → `To Verify` (reviewer APPROVE,
awaiting owner's manual check) → `Done` (owner merged). The agent never sets
`Done`.

## 8. Failure handling

- CRG unavailable → report it, continue with `rg` + source, mark impact analysis
  as degraded. Never fabricate graph results.
- Build/tests cannot run → say so explicitly in `report.md`; the reviewer must
  return `BLOCKED` unless CI evidence exists.
- Contested requirement or design decision → stop and ask the owner.
