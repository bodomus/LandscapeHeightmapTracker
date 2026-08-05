# Landscape Paint Layers — bulk removal

## Editor UI

1. Select exactly one Landscape or LandscapeStreamingProxy actor.
2. Open the UE57Editor Landscape panel.
3. In Landscape Paint Layers, click Refresh.
4. Review layer name, LayerInfo asset, blend mode, material presence and orphaned marker.
5. Select layers with checkboxes or use Select All, None, Invert.
6. Click Remove Selected or Remove All and review the confirmation dialog.

Removal deletes paint/weightmap data through Unreal's standard
ULandscapeInfo::DeleteLayer path. It does not delete LayerInfo assets and does
not modify the Landscape material. Material-defined targets can therefore remain
visible after removal as unassigned targets, matching the stock Landscape X
button behavior.

The whole bulk operation is one Editor transaction and can be undone/redone once.

## Unreal Python

The library requires exactly one selected Landscape and an explicit confirmation
flag. It never opens a dialog from Python.

    import unreal

    layers = unreal.LandscapePaintLayerLibrary.get_selected_landscape_paint_target_layers()
    names = [layer.layer_name for layer in layers if layer.is_orphaned]
    result = unreal.LandscapePaintLayerLibrary.remove_selected_landscape_paint_target_layers(
        names,
        b_confirm=True,
    )
    unreal.log(result.message)

Passing b_confirm=False, selecting zero/multiple Landscapes, requesting an empty
set, an unknown name, or a duplicate name returns a failed structured result
without changing the Landscape.

## Manual verification checklist

- With zero and multiple Landscapes selected, Refresh reports the selection error
  and removal buttons stay disabled.
- A single selected Landscape lists every ULandscapeInfo::Layers entry.
- Material presence matches the selected proxy's Landscape/override/hole/LOD
  materials.
- Remove Selected leaves unselected paint data intact.
- Remove All uses the stronger warning and removes every listed layer in one
  transaction.
- Cancel changes nothing.
- Undo restores the operation in one step; Redo reapplies it in one step.
- LayerInfo assets remain in the Content Browser and material assignments remain
  unchanged.
- Viewports and the widget refresh after success.
