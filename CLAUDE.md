# CLAUDE.md

@AGENTS.md
@Docs/WORKFLOW.md
@Docs/PROJECT_RULES.md

## Claude Code specifics

### Environment
- Implementation, builds and tests run in Claude Code **on the owner's Windows
  machine** (desktop app or CLI) with UE 5.7 installed. A cloud/Linux session
  can write tickets and review diffs but cannot build — never claim it did.
- Shell is Git Bash on Windows; PowerShell scripts are run as
  `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/<script>.ps1 ...`.
- Required env vars (set once with `setx`): `LHT_UE_ROOT`
  (e.g. the `UE_5.7` folder), `LHT_HOST_UPROJECT` (host `.uproject`).

### Commands (skills)
- `/ticket <requirement>` — grill the requirement, write `Tickets/<ID>.md`, create the YouTrack issue.
- `/implement <ID>` — full implementation cycle per `Docs/WORKFLOW.md` §4.
- `/review <ID>` — independent review via the `reviewer` agent. Run it in a fresh session.
- `/grill-me` — stress-test a plan or requirement with questions.

### MCP servers
- `code-review-graph` (`.mcp.json`) — use its tools *before* opening many files:
  `get_minimal_context_tool`, `get_review_context_tool`, `query_graph_tool`,
  `get_impact_radius_tool`, `detect_changes_tool`, `semantic_search_nodes_tool`.
  A PostToolUse hook runs `code-review-graph update` after edits.
- `context7` — up-to-date library docs. Coverage of Unreal Engine is limited:
  for engine APIs prefer reading engine headers under `$LHT_UE_ROOT/Engine/Source`.
- YouTrack — the claude.ai connector (project key **UE5**). Tools: `create_issue`,
  `update_issue`, `add_issue_comment`, `get_issue`, `search_issues`.

### Models
Default model for this repo is Sonnet (implementation). The `reviewer` agent is
pinned to Opus. Follow the escalation rules in `Docs/WORKFLOW.md` §6 and
*suggest* `/model opus` to the owner when they apply.

### Context economy
- Do not read `Build/`, `Binaries/`, `Intermediate/`, `Saved/`, `evaluation/`,
  `graphify-out/` or old `Reviews/`/`Task/` unless the task is about them.
- Prefer CRG queries and targeted `rg` over reading whole directories.
- `Docs/ARCHITECTURE.md` is the quickest orientation; keep it updated when
  architecture changes.

### Communication with the owner
- The owner writes in Russian; answer in Russian. Code, commits, comments,
  tickets' technical identifiers stay in English; ticket prose may be Russian.
- On any contested requirement or design decision: stop and ask.
- Quality over speed.
- Final status lines: `🟢 PASS` when everything succeeded, otherwise
  `🔴 FAIL` / `🔴 BLOCKED` / `🔴 PARTIAL` with the reason.
- Be a teammate: challenge the ticket or the owner's idea when there is a better
  option, with arguments.
