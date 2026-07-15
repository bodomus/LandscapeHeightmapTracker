# CRG Embedding Diagnosis

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

## Baseline

- CRG version: `code-review-graph 2.3.6`
- Baseline database: `.code-review-graph/graph.db`
- Baseline graph freshness: matched current HEAD `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- Baseline status:
  - Nodes: `125`
  - Edges: `916`
  - Files: `25`
  - Last updated: `2026-07-09T20:17:25`
  - Built at commit: `93b214fbe2ea`
  - Embeddings: `0`
  - FTS rows: `125`

## CLI Authority Checked

Commands:

```powershell
code-review-graph --help
code-review-graph status --help
code-review-graph build --help
code-review-graph update --help
code-review-graph postprocess --help
code-review-graph embed --help
```

Embedding help states:

- Command: `code-review-graph embed`
- Providers: `local`, `openai`, `google`, `minimax`
- Default provider: `local`
- Default local model: `all-MiniLM-L6-v2`
- Local provider requires `code-review-graph[embeddings]`

No relevant external provider environment variables (`OPENAI`, `GOOGLE`, `GEMINI`, `MINIMAX`, `EMBED`, `CRG`, `CODE_REVIEW`) were set in this shell.

## Diagnosis

Initial local provider check:

- `sentence_transformers`: missing
- `torch`: present (`2.5.1+cu121`)
- `transformers`: present (`4.57.1`)
- `numpy`: present (`2.3.3`)

Initial command:

```powershell
code-review-graph embed --repo . --provider local
```

Result:

- Exit code: `1`
- Time: `0.325` seconds
- Error: local embedding provider needs `sentence-transformers`; install with `pip install code-review-graph[embeddings]` or switch provider.

Conclusion: CRG reported zero embedded nodes because the local embedding extra was not installed and no external embedding provider was configured. Embeddings are optional but expected for semantic search; FTS still existed for keyword lookup.

## Enablement

Supported CRG extra installed:

```powershell
python -m pip install "code-review-graph[embeddings]"
```

Result:

- Exit code: `0`
- Time: `5.562` seconds
- Installed `sentence-transformers 3.4.1`
- Production plugin source was not modified.

Embedding command:

```powershell
code-review-graph embed --repo . --provider local
```

Result:

- Exit code: `0`
- Time: `55.134` seconds
- Model/provider: `local:all-MiniLM-L6-v2`
- Runtime device: `cuda`
- Output: `Embedded 100 new node(s). Total embeddings: 100. Semantic search is now active.`

## Rebuild/Verification

Full CRG rebuild command:

```powershell
code-review-graph build --repo .
```

Result:

- Exit code: `0`
- Time: `0.969` seconds
- Output summary: `Full build: 25 files, 130 nodes, 934 edges (postprocess=full)`
- Postprocess output also reported `FTS index rebuilt: 125 rows indexed` and `Loaded 100 unique nodes, 916 edges`.

Follow-up embedding command:

```powershell
code-review-graph embed --repo . --provider local
```

Result:

- Exit code: `0`
- Time: `4.176` seconds
- Output: `Embedded 0 new node(s). Total embeddings: 100. Semantic search is now active.`

Final `code-review-graph status`:

- Nodes: `125`
- Edges: `916`
- Files: `25`
- Languages: `csharp, cpp, c`
- Last updated: `2026-07-10T11:48:49`
- Built at commit: `93b214fbe2ea`

Final database metadata:

- `git_head_sha`: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- `last_build_type`: `full`
- `last_postprocessed_at`: `2026-07-10T11:48:49`
- `last_updated`: `2026-07-10T11:48:49`
- `postprocess_level`: `full`
- `schema_version`: `9`

Final embedding status:

- Embeddings: `100`
- FTS rows: `125`
- Provider rows:
  - `local:all-MiniLM-L6-v2`: `100`

Coverage by language/kind:

- Embedded:
  - `cpp` `Function`: `77`
  - `c` `Class`: `10`
  - `c` `Function`: `10`
  - `cpp` `Class`: `1`
  - `csharp` `Class`: `1`
  - `csharp` `Function`: `1`
- Not embedded:
  - `cpp` `File`: `14`
  - `c` `File`: `10`
  - `csharp` `File`: `1`

## Conclusion

Embeddings could be enabled safely without modifying production source. Current CRG has semantic search active for all Function/Class nodes, including C++ symbols. File nodes remain unembedded by observed CRG behavior.
