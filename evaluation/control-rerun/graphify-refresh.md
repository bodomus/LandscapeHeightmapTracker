# Graphify Refresh

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

## Inputs

- Start HEAD: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- Previous Graphify graph freshness: built from commit `f17cf664`
- Previous graph stats:
  - Nodes: `376`
  - Edges: `538`
  - Communities: `18`
- Graphify version: `graphify 0.9.8`
- CLI help checked before refresh: `graphify --help`
- Relevant help entry used: `update <path>` - "re-extract code files and update the graph (no LLM needed)"

## Commands

```powershell
graphify --help
git rev-parse HEAD
graphify update .
graphify diagnose multigraph --graph graphify-out/graph.json --max-examples 10
```

## Refresh Result

- Refresh command: `graphify update .`
- Exit code: `0`
- Build/update time: `1.711` seconds
- Command output summary:
  - Backed up semantic and curated graph files to `graphify-out/2026-07-10/`
  - Rebuilt graph: `503` nodes, `645` edges, `38` communities
  - Updated `graphify-out/graph.json`, `graphify-out/graph.html`, and `graphify-out/GRAPH_REPORT.md`
  - Printed Gemini tip for semantic extraction, but code update completed without requiring an API key

## Resulting Freshness Metadata

- Current HEAD after refresh: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- `graphify-out/GRAPH_REPORT.md` freshness section reports: `Built from commit: 93b214fb`
- Freshness conclusion: Graphify now matches current HEAD by commit prefix.

## Resulting Graph Stats

- Directed: `False`
- Multigraph: `False`
- Nodes: `503`
- Edges: `645`
- Communities: `38`
- Source paths represented by graph nodes: `42`

## Diagnostics

`graphify diagnose multigraph --graph graphify-out/graph.json --max-examples 10` reported:

- Missing endpoint edges: `0`
- Dangling endpoint edges: `0`
- Self-loop edges: `4`
- Exact duplicate edges: `0`
- Directed same-endpoint collapsed edges: `0`
- Undirected same-endpoint collapsed edges: `0`
- Same-endpoint group count: `0`
- Relation/source/location/context variant groups: `0`
- Producer suppression sites: `60`

No parser failure, warning, or error text was found in `graphify-out/GRAPH_REPORT.md`.

## Limitation

The Graphify report records only the short commit prefix (`93b214fb`), not the full SHA. The prefix matches the current full HEAD `93b214fbe2ea3f78286e2240d6b117eb6812c36d`, so freshness is established for this rerun.
