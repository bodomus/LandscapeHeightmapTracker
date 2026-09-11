# UE5-19 — Verify Material Instance texture assignment by read-back

## Summary

Исправить ложные warnings в ScanVault Importer v1 при назначении texture parameters в `UMaterialInstanceConstant`.

Сейчас importer считает `false` от:

```cpp
UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(...)
```

ошибкой и добавляет:

```text
material.mapping.assign_failed
```

Но в реальном UE 5.7 параметр при этом фактически может быть назначен корректно.

Ручная проверка показала, что при warnings текстуры реально стоят в MI:

```text
BaseColorTexture
NormalTexture
RoughnessTexture
```

## Required change

В `ScanVaultImporter.cpp`, в `AssignTextureParameters(...)`, не использовать return value setter-а как единственный критерий ошибки.

После setter выполнить read-back verification.

Ожидаемая логика:

```cpp
UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
    Instance,
    Mapping.ParameterName,
    TextureValue);

UMaterialEditingLibrary::UpdateMaterialInstance(Instance);

UTexture* AssignedTexture =
    UMaterialEditingLibrary::GetMaterialInstanceTextureParameterValue(
        Instance,
        Mapping.ParameterName);

if (AssignedTexture != TextureValue)
{
    AddIssue(
        Report.Issues,
        EImportIssueSeverity::Warning,
        TEXT("material.mapping.assign_failed"),
        FString::Printf(
            TEXT("Failed to verify texture parameter '%s'."),
            *Mapping.ParameterName.ToString()));
}
```

Если API signature в UE 5.7 отличается, использовать эквивалентный supported read-back path.

## Preserve existing checks

Не удалять:

```text
ParentHasTextureParameter(...)
```

Итоговая семантика должна быть:

```text
parameter missing
    -> material.mapping.parameter_missing

texture for role missing
    -> material.mapping.texture_missing

setter returns false, read-back matches expected texture
    -> no warning

read-back null/different
    -> material.mapping.assign_failed
```

## Scope

Исправить только ложную диагностику assignment.

Не менять:

- manifest schema;
- ScanVault mappings;
- material profile semantics;
- naming;
- texture import;
- MI creation;
- Nanite;
- LOD;
- conflict policy.

## Tests

Добавить regression coverage:

```text
setter false + read-back matches
    -> no assign_failed

read-back mismatch/null
    -> assign_failed preserved
```

Если прямой UE API вызов неудобно тестировать, вынести минимальную verification policy в helper.

## Manual validation

Повторно импортировать:

```text
Wooden_Sticks_And_Twigs
```

Проверить, что:

```text
BaseColorTexture assigned
NormalTexture assigned
RoughnessTexture assigned
```

и warnings:

```text
material.mapping.assign_failed
```

для них больше не появляются.

Реальные warnings:

```text
material.mapping.texture_missing
material.mapping.parameter_missing
```

должны сохраниться.

## Validation

1. build UE57Editor;
2. run `LandscapeHeightmapTracker.ScanVault.*`;
3. run `git diff --check`;
4. manual import real MLV-14 manifest;
5. verify MI parameter values;
6. update `Task/SCANVAULT-IMPORTER-V1/implementation-report.md`;
7. update `Reviews/review-UE5-19.md`.

Expected:

```text
Build: passed
ScanVault tests: passed
git diff --check: passed
Texture parameter read-back: passed
False assign_failed warnings: removed
Real warnings: preserved
```
