# UE5-14 Implementation Plan

1. Добавить `PaintLayersTabName`, регистрацию/снятие регистрации spawner и
   `OnSpawnPaintLayersTab`, создающий существующий bulk-remove widget.
2. Добавить обработчик кнопки `OpenPaintLayers` в основной панели.
3. Перестроить верхнюю область `Construct` в две bordered columns, не меняя
   существующие attributes и callbacks.
4. Заменить встроенный Paint Layers widget компактной кнопкой.
5. Перестроить Height Zone: controls + карта слева, Tracking и Diagnostics справа,
   сохранив все текущие bindings.
6. Проверить diff на отсутствие логических изменений.
7. Обновить CRG, проверить impact radius/review context и Graphify только если
   изменятся важные entry points (новый tab является таким entry point).
8. Собрать UE57Editor, запустить целевые Paint Layers tests и доступные broader
   plugin tests.
9. Выполнить доступную ручную проверку, обновить UE5-14, создать
   `review-UE5-14.md` и implementation report.

