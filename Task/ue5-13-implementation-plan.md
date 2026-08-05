# UE5-13 implementation plan

1. Добавить отражаемые структуры описания Target Layer и результата удаления.
2. Реализовать независимый `FLandscapePaintLayerService`: точный Editor selection, enumeration, полная предварительная валидация, одна transaction и штатный `ULandscapeInfo::DeleteLayer`.
3. Добавить `ULandscapePaintLayerLibrary` для Unreal Python с обязательным `bConfirm`.
4. Добавить отдельный Slate widget массового управления и встроить его в существующую панель.
5. Покрыть чистую валидацию тестами, выполнить UE 5.7 build и automation tests.
6. Обновить CRG, проверить impact radius, при необходимости обновить Graphify и выпустить implementation/review reports.
