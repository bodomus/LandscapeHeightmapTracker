---
name: ticket
description: Turn a plain-language requirement from the owner into a precise ticket - Tickets/<ID>.md plus a YouTrack issue in project UE5. Use when the owner asks to create, write or formulate a ticket/task ("создай тикет", "оформи задачу").
argument-hint: <requirement in plain language>
---

# Create a ticket

Input: `$ARGUMENTS` — the owner's requirement (may be Russian, informal).

1. **Understand.** Run the `grill-me` procedure. Do not write the ticket until the
   owner confirms the decision summary. For a tiny, unambiguous change one short
   round of questions is enough.
2. **Check duplicates.** YouTrack `search_issues` with `project: UE5` and key words;
   `rg` in `Tickets/`. If a related issue exists, propose linking or extending it.
3. **Draft** using the template below. Prose in Russian, identifiers in English.
   Every acceptance criterion must be checkable; list concrete automation tests.
4. **Show the draft** to the owner and apply corrections.
5. **Create the YouTrack issue** (`create_issue`, project `UE5`, Type `Task` or `Bug`,
   Priority from the discussion, State `Open`). Description = the full markdown
   (the connector cannot attach files).
6. **Save** `Tickets/<ID>.md` with the real ID (e.g. `Tickets/UE5-21.md`) and the
   issue URL in the header. Do not commit to `main`; the file is committed on the
   ticket's feature branch by `/implement`.
7. Report: ID, URL, workflow level (0/1/2), recommended model for implementation
   (Sonnet unless `Docs/WORKFLOW.md` §6 says Opus).

## Template

```markdown
# <ID> — <short title>

- URL: <youtrack url>
- Type: Task | Bug
- Priority: Normal
- Level: 1 | 2
- Visual verification by owner before merge: yes | no

## Context and goal
<why, current behavior, desired outcome>

## Scope
<what to build: UI, behavior, parameters with defaults and units>

## Non-goals
<explicitly out of scope>

## Technical constraints
<editor-only, Undo/Redo, dirtying, threading, lifetime, performance limits,
existing code/approach to reuse>

## Acceptance criteria
- [ ] ...

## Automation tests
1. ...

## Manual verification
1. ...

## Deliverables
Code + tests, `Task/<ID>/report.md`, updated docs (`Docs/...`) if behavior or
architecture changes, screenshots when UI changes.
```
