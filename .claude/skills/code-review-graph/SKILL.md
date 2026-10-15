---
name: code-review-graph
description: How to use code-review-graph (CRG) in this repo for symbol lookup, callers/callees, dependants, review context and change impact before and after editing C++ code. Use during implementation preflight, post-change impact checks and code review.
---

# code-review-graph (CRG)

CRG keeps a structural graph of the repo in `.code-review-graph/graph.db`
(gitignored; indexes only git-tracked files, so `Build/` copies are not indexed).
It is the cheapest way to get context: ask the graph first, then open only the
files it points to.

## Commands (CLI)
```
code-review-graph update --skip-flows        # incremental refresh (hook runs it after edits)
code-review-graph update                     # refresh incl. flows
code-review-graph build                      # full rebuild — only if update fails or DB is corrupt
code-review-graph detect-changes --base main --brief   # changed symbols + risk vs main
```
If `code-review-graph` is not on PATH use `uvx code-review-graph ...`.
Do not delete `.code-review-graph/` as a first troubleshooting step.

## MCP tools (server `code-review-graph`)
| Need | Tool |
|---|---|
| Compact context for a task or symbol set | `get_minimal_context_tool` |
| Find symbols by meaning | `semantic_search_nodes_tool` |
| Callers / callees / imports / inheritance | `query_graph_tool`, `traverse_graph_tool` |
| Blast radius of a change | `get_impact_radius_tool`, `get_affected_flows_tool` |
| Review of current diff | `detect_changes_tool`, `get_review_context_tool` |
| Orientation | `get_hub_nodes_tool`, `list_communities_tool`, `list_graph_stats_tool` |

## Known limits on Unreal C++ (from `evaluation/`)
- Macros (`UCLASS`, `DECLARE_DELEGATE_*`, `SLATE_ARGUMENT`), lambdas bound to
  delegates and Slate attributes are weakly represented. Delegate subscription,
  `BindSP/BindRaw/AddUObject`, `SNew(...).OnClicked(...)` wiring must be verified with `rg`.
- Graph proximity ≠ runtime flow. Absence claims require exhaustive `rg`.

## Usage pattern
- **Preflight:** update → minimal context for ticket vocabulary → callers/dependants
  of symbols to change → impact radius → related tests → verify in source.
- **After change:** update → `detect-changes --base main --brief` → compare with the
  expected radius; investigate surprises.
- **Review:** `detect_changes_tool` + `get_review_context_tool` for the diff; look for
  callers/implementations that should have changed and did not.

Record the commands used and the key findings in the report/review.
