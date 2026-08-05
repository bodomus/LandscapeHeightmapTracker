# UE5-13 investigation

## Repository intelligence

Graphify определил `SLandscapeHeightmapTrackerPanel` как существующую точку интеграции UI, создаваемую `FLandscapeHeightmapTrackerModule`. CRG подтвердил, что панель уже владеет состоянием Landscape и является локальной границей изменения; отдельного paint-layer subsystem в проекте нет.

Выводы графов проверены по `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`, публичному header панели и module startup/tab creation.

## Штатный путь UE 5.7

Кнопка X Target Layer находится в:

`Engine/Source/Editor/LandscapeEditor/Private/LandscapeEditorDetailCustomization_TargetLayers.cpp`

`FLandscapeEditorCustomNodeBuilder_TargetLayers::OnTargetLayerDeleteClicked`:

1. показывает Yes/No confirmation;
2. создаёт `FScopedTransaction("Delete Layer")`;
3. создаёт `FScopedSetLandscapeEditingLayer` для текущего edit-layer;
4. вызывает `Target->LandscapeInfo->DeleteLayer(Target->LayerInfoObj.Get(), Target->LayerName)`;
5. вызывает `FEdModeLandscape::UpdateTargetList()`.

Публичная реализация `ULandscapeInfo::DeleteLayer` находится в `Runtime/Landscape/Private/LandscapeEdit.cpp`. Она вызывает `LandscapeActor->Modify()`, удаляет `FLandscapeInfoLayerSettings`, вызывает `ALandscape::RemoveTargetLayer`, очищает weightmap-данные всех компонентов через `FLandscapeEditDataInterface::DeleteLayer`, затем `UpdateLayerInfoMap()`. Компоненты и proxy получают `Modify()`, а Landscape получает `RequestLayersContentUpdate(Update_All)`.

## Публичные данные для списка

- `ULandscapeInfo::Layers` содержит `FLandscapeInfoLayerSettings` с `LayerName` и `LayerInfoObj`.
- `ULandscapeLayerInfoObject::GetBlendMethod()` возвращает blend mode.
- `ALandscapeProxy::RetrieveTargetLayerNamesFromMaterials()` возвращает имена из Landscape, override, hole и per-LOD materials.
- orphaned надёжно определяется как слой с LayerInfo, отсутствующий во всех назначенных Landscape materials.

## Ограничения

`LandscapeEditor` private API использовать нельзя. Сервис применяет публичные Runtime/Landscape API, текущий `ALandscape::GetEditingLayer()` и после операции обновляет viewport. Штатная Landscape Mode UI обновит target list при следующем refresh; собственный список обновляется немедленно.
