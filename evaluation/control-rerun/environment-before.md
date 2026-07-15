# Environment Before Controlled Rerun

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Recorded: 2026-07-10

## Git

- Current HEAD: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- HEAD commit date: `2026-07-09 19:28:22 +0300`
- HEAD subject: `Merge pull request #1 from bodomus/codex/UE5-10-bidirectional-heightmap-tracking`
- Working tree before rerun: existing unrelated/uncommitted benchmark artifacts and `.gitignore` changes were present (`.gitignore`, `.codex/`, `evaluation/`).

## Graphify

- Executable: `C:\Users\bodom\.local\bin\graphify.exe`
- Version command used: `graphify --version`
- Version: `graphify 0.9.8`
- Help command used: `graphify --help`
- Existing graph path: `graphify-out/graph.json`
- Existing report path: `graphify-out/GRAPH_REPORT.md`
- Existing graph reported freshness: built from commit `f17cf664`
- Current HEAD for comparison: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- Freshness finding: stale relative to current HEAD.
- Existing graph stats:
  - Directed: `False`
  - Multigraph: `False`
  - Nodes: `376`
  - Edges/links: `538`
  - Communities: `18`
  - Source paths represented by graph nodes: `31`

## Code Review Graph

- Executable: `C:\Python312\Scripts\code-review-graph.exe`
- Version command used: `code-review-graph --version`
- Version: `code-review-graph 2.3.6`
- Help commands used:
  - `code-review-graph --help`
  - `code-review-graph status --help`
  - `code-review-graph build --help`
  - `code-review-graph update --help`
  - `code-review-graph postprocess --help`
  - `code-review-graph embed --help`
- Existing database path: `.code-review-graph/graph.db`
- Status command used: `code-review-graph status`
- CRG reported status:
  - Nodes: `125`
  - Edges: `916`
  - Files: `25`
  - Languages: `csharp, cpp, c`
  - Last updated: `2026-07-09T20:17:25`
  - Built on branch: `main`
  - Built at commit: `93b214fbe2ea`
- Database metadata:
  - `git_branch`: `main`
  - `git_head_sha`: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
  - `last_build_type`: `full`
  - `last_postprocessed_at`: `2026-07-09T20:17:25`
  - `last_updated`: `2026-07-09T20:17:25`
  - `postprocess_level`: `full`
  - `schema_version`: `9`
- Freshness finding: CRG database matches current HEAD.
- Embedding status:
  - Embeddings table rows: `0`
  - FTS rows: `125`
  - Embedding providers present: none
- Available embedding configuration from CLI help:
  - Command: `code-review-graph embed`
  - Providers: `local`, `openai`, `google`, `minimax`
  - Default provider: `local`
  - Local provider requires `code-review-graph[embeddings]`
  - Default local model: `all-MiniLM-L6-v2`
  - Environment variables relevant to external providers (`OPENAI`, `GOOGLE`, `GEMINI`, `MINIMAX`, `EMBED`, `CRG`, `CODE_REVIEW`) were not set in this shell.
