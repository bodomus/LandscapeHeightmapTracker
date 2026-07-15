# CRG Metrics

## Tool Coverage

| Capability required by benchmark | Tested with CRG | Result |
|---|---:|---|
| Symbol lookup | Yes | Good for exact names |
| Callers | Yes, via rename preview/impact/flows | Poor; missed qualified static call sites |
| Callees | Yes, via flows/impact edges | Partial; local mapper callees worked |
| Dependencies | Yes, via communities/hubs/bridges/impact | Useful for orientation, noisy for runtime |
| Impact radius | Yes | Overbroad; needed source filtering |
| Semantic search | Yes | Weak; no embeddings, brittle keyword behavior |
| Change-related investigation | Yes | Inconsistent risk/impact outputs |

## CRG Output Volume

| Tool/query type | Approx volume | Notes |
|---|---:|---|
| `list_graph_stats` | Tiny | High signal |
| Exact symbol search | Tiny to medium | High signal when query is a single exact symbol |
| Multi-term search | Tiny | Often zero results; low signal |
| `list_flows` | Medium | Useful but incomplete runtime modeling |
| `get_flow` | Small | Useful for same-file flows |
| `get_impact_radius` | Very large | High noise for mapper files |
| `detect_changes` | Very large | Overstated changed entities |
| `get_community(include_members=true)` | Very large | Useful index but too broad |
| Rename preview | Tiny | Useful negative test; poor caller coverage |

## Per-Question Scores

| Question | CRG usefulness | Main reason |
|---|---:|---|
| Q1 initialization/register editor functionality | 4/5 | Exact symbol lookup and hub data worked |
| Q2 main visualization widget/owner | 4/5 | Found widget and panel functions after exact query |
| Q3 cursor/landscape acquisition | 3/5 | Found trace symbol, missed full path |
| Q4 world/local to image/pixel mapping | 4/5 | Mapper functions and local flows were good |
| Q5 visual marker update path | 2/5 | No complete cross-file delegate/Slate flow |
| Q6 actual callers | 1/5 | Rename preview missed real call sites |
| Q7 impact radius | 3/5 | Found many related nodes but overbroad/noisy |
| Q8 transform sensitivity | 2/5 | Needed source search to distinguish real transform use |
| Q9 dimensional assumptions | 2/5 | Needed source search; CRG not semantic enough |
| Q10 marker widget to coordinate acquisition | 3/5 | Found pieces, not the runtime connection |

Overall CRG usefulness: **3/5**.

## False Positives / False Negatives

False positives:

- Header `class` pseudo-functions.
- Mapper impact including unrelated startup/style/menu nodes.
- `ComputeDesiredSize(float LayoutScaleMultiplier)` in transform/scale source grep, not a Landscape transform concern.

False negatives:

- Multi-term searches returned zero for relevant queries.
- Caller discovery missed:
  - `SLandscapeHeightmapTrackerPanel::OnViewportClick` calling `MapWorldPosition`.
  - Coordinate mapper tests calling `MapWorldPosition`.
  - Panel/tests calling `MapUVToLocalPosition`.
- CRG flows missed:
  - `InputKey -> TraceLandscapeClick -> Broadcast -> OnViewportClick`.
  - `OnSpawnPluginTab -> SLandscapeHeightmapTrackerPanel -> SHeightmapTrackerImageView`.
  - `SHeightmapTrackerImageView::OnMouseButtonDown -> OnHeightmapClicked -> SetReverseMarker`.
