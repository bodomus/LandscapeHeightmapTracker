# UE5-14 Investigation

## Workflow

- Level: 2 (Slate layout + новый editor tab и lifecycle регистрации).
- Initial branch: `main`.
- Initial commit: `3a033d8066ca6795d8a3bc64b482b2967bd99fe1`.
- Initial working tree: пользовательский неотслеживаемый каталог `images/`.
- Graphify: существующий граф от 31 июля был устаревшим для Paint Layers из
  UE5-13; выполнен `graphify update .` — 1186 nodes, 1533 edges, 88 communities.
- CRG: существующий индекс содержит 232 nodes / 1782 edges / 44 files; успешные
  scoped queries подтвердили наличие панели, bulk-remove widget и module tab
  boundary. SHA относится к родительской feature-ветке UE5-13, но исходники,
  затрагиваемые UE5-14, присутствуют и проверены напрямую.

## Current behavior

`SLandscapeHeightmapTrackerPanel::Construct` строит один длинный `SVerticalBox`:

1. Landscape controls/info;
2. встроенный `SLandscapePaintLayerBulkRemoveWidget`;
3. Heightmap controls/info;
4. Height Zone controls и карта;
5. Tracking;
6. Coordinate Diagnostics.

Из-за последовательного расположения секций окно чрезмерно высокое.

## Architecture and ownership

- `FLandscapeHeightmapTrackerModule::StartupModule` регистрирует singleton Nomad
  tab `LandscapeHeightmapTracker`.
- `OnSpawnPluginTab` создаёт `SLandscapeHeightmapTrackerPanel`.
- Панель владеет mapping/height-zone/marker состоянием и подписками viewport.
- `SLandscapePaintLayerBulkRemoveWidget` уже является самостоятельным виджетом.
  Он хранит только собственное transient UI selection state и обращается к
  `FLandscapePaintLayerService`; callbacks панели ему не нужны.
- `FGlobalTabmanager::TryInvokeTab` подходит для отдельной singleton editor
  поверхности: повторный вызов активирует существующий tab, а spawner lifecycle
  симметричен StartupModule/ShutdownModule.

## Graph findings validated in source

- Graphify назвал панель, module tab spawner, image view, paint widget и service
  основной рабочей областью.
- CRG нашёл `Construct` панели (текущий большой layout), `OnSpawnPluginTab` и
  `SLandscapePaintLayerBulkRemoveWidget::Construct`.
- Исходники подтверждают фактическую цепочку владения и все Slate callbacks.
- Графы не использовались как доказательство delegate/runtime flow.

## Smallest correct change

1. Зарегистрировать второй скрытый Nomad tab для Paint Layers и симметрично снять
   регистрацию при shutdown.
2. В отдельном spawner создавать существующий
   `SLandscapePaintLayerBulkRemoveWidget` без дублирования его логики.
3. Добавить в панель кнопку, вызывающую `TryInvokeTab`.
4. Пересобрать только Slate layout `Construct`, сохранив те же attributes,
   callbacks и state fields.

## Direct and adjacent impact

- Direct: module header/cpp и panel header/cpp.
- Adjacent: существующий Paint Layers widget и service (переиспользуются без
  изменения), module startup/shutdown и tab manager lifecycle.
- Test-only: `LandscapeHeightmapTracker.PaintLayers.*` проверяет service, но в
  проекте нет automation-тестов визуальной структуры Slate.
- Graph-proximity noise: mapper, editor mode и height-zone algorithms не требуют
  изменения; их callbacks нужно только сохранить в layout.

## Risks

- Потеря callback/attribute при переносе Slate блоков.
- Несимметричная регистрация нового tab spawner.
- Недостаточная ширина правой колонки на узком окне.
- Визуальный layout и singleton activation требуют ручной editor-проверки;
  компиляция и service tests не доказывают геометрию Slate.

