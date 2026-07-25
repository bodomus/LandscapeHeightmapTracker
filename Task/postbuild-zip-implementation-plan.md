# Post-build ZIP implementation plan

1. Add a Win64 plugin `PostBuildSteps` command to the `.uplugin` descriptor.
2. Add a strict PowerShell packaging script that stages only distributable plugin
   paths and creates a deterministic platform/configuration-named ZIP.
3. Document the generated package and installation workflow.
4. Validate JSON, script behavior, archive layout, CRG impact, and a real UBT
   host-project build.
