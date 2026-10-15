<#
.SYNOPSIS
    Builds the host editor target with the LandscapeHeightmapTracker plugin and runs
    the plugin's automation tests. Used by agents locally and by CI.

.DESCRIPTION
    Machine-specific paths come from parameters or environment variables, never from
    committed files:
      LHT_UE_ROOT        Unreal Engine root, e.g. C:\Program Files\Epic Games\UE_5.7
      LHT_HOST_UPROJECT  Host project .uproject that contains (or will receive) the plugin

    Exit code 0 = build succeeded and no automation test failed.
    Logs and reports: <PluginRoot>\Saved\Validation\<timestamp>\

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Invoke-Validation.ps1
.EXAMPLE
    ... -File Scripts/Invoke-Validation.ps1 -TestFilter "LandscapeHeightmapTracker.UI." -Rhi Offscreen
.EXAMPLE
    # CI: copy the checked-out plugin into a dedicated host project first
    ... -File Scripts/Invoke-Validation.ps1 -SyncPluginToHost
#>
[CmdletBinding()]
param(
    [string] $UERoot = $env:LHT_UE_ROOT,
    [string] $HostUProject = $env:LHT_HOST_UPROJECT,
    [string] $EditorTarget,
    [string] $Configuration = 'Development',
    [string] $TestFilter = 'LandscapeHeightmapTracker.',
    [ValidateSet('NullRHI', 'Offscreen')]
    [string] $Rhi = 'NullRHI',
    [switch] $SkipBuild,
    [switch] $SkipTests,
    [switch] $SyncPluginToHost,
    [int] $TestTimeoutMinutes = 30
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Write-Step([string] $Text) { Write-Host "`n=== $Text ===" -ForegroundColor Cyan }
function Fail([string] $Text) { Write-Host "RESULT: FAIL - $Text" -ForegroundColor Red; exit 1 }

# ---------- resolve inputs ----------
$pluginRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$pluginName = 'LandscapeHeightmapTracker'

if ([string]::IsNullOrWhiteSpace($UERoot)) { Fail 'UE root not set (param -UERoot or env LHT_UE_ROOT).' }
if ([string]::IsNullOrWhiteSpace($HostUProject)) { Fail 'Host project not set (param -HostUProject or env LHT_HOST_UPROJECT).' }
if (-not (Test-Path -LiteralPath $UERoot)) { Fail "UE root not found: $UERoot" }
if (-not (Test-Path -LiteralPath $HostUProject)) { Fail "Host .uproject not found: $HostUProject" }

$HostUProject = (Resolve-Path -LiteralPath $HostUProject).Path
$hostDir = Split-Path -Parent $HostUProject
if ([string]::IsNullOrWhiteSpace($EditorTarget)) {
    $EditorTarget = [System.IO.Path]::GetFileNameWithoutExtension($HostUProject) + 'Editor'
}

$buildBat = Join-Path $UERoot 'Engine\Build\BatchFiles\Build.bat'
$editorCmd = Join-Path $UERoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
foreach ($p in @($buildBat, $editorCmd)) { if (-not (Test-Path -LiteralPath $p)) { Fail "Missing engine file: $p" } }

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$outDir = Join-Path $pluginRoot "Saved\Validation\$stamp"
New-Item -ItemType Directory -Path $outDir -Force | Out-Null

Write-Host "Plugin root : $pluginRoot"
Write-Host "Host project: $HostUProject"
Write-Host "Target      : $EditorTarget Win64 $Configuration"
Write-Host "Output      : $outDir"

# ---------- optional sync (CI) ----------
$hostPluginDir = Join-Path $hostDir "Plugins\$pluginName"
if ($SyncPluginToHost) {
    Write-Step 'Sync plugin into host project'
    $resolvedTarget = [System.IO.Path]::GetFullPath($hostPluginDir).TrimEnd('\')
    if ($resolvedTarget -ieq $pluginRoot.TrimEnd('\')) { Fail 'Refusing to sync: host plugin dir is the repository itself.' }
    New-Item -ItemType Directory -Path $hostPluginDir -Force | Out-Null
    $exclude = @('.git', '.github', '.claude', 'Binaries', 'Intermediate', 'Saved', 'Build', 'build',
                 '.code-review-graph', 'graphify-out', 'evaluation', 'Tickets', 'Task', 'Reviews')
    $rcArgs = @($pluginRoot, $hostPluginDir, '/MIR', '/NFL', '/NDL', '/NJH', '/NP', '/XD') + $exclude
    Write-Host "robocopy $($rcArgs -join ' ')"
    & robocopy @rcArgs | Out-Null
    if ($LASTEXITCODE -ge 8) { Fail "robocopy failed with code $LASTEXITCODE" }
    $global:LASTEXITCODE = 0
} elseif (-not (Test-Path -LiteralPath (Join-Path $hostPluginDir "$pluginName.uplugin"))) {
    Write-Warning "Plugin not found under host Plugins dir ($hostPluginDir). Use -SyncPluginToHost or install it there."
}

$summary = [ordered]@{ Build = 'skipped'; Tests = 'skipped'; Passed = 0; Warnings = 0; Failed = 0; NotRun = 0 }

# ---------- build ----------
if (-not $SkipBuild) {
    Write-Step 'Build'
    $buildLog = Join-Path $outDir 'build.log'
    $buildArgs = @($EditorTarget, 'Win64', $Configuration, "-Project=$HostUProject", '-WaitMutex', '-NoHotReloadFromIDE')
    Write-Host "& '$buildBat' $($buildArgs -join ' ')"
    & $buildBat @buildArgs 2>&1 | Tee-Object -FilePath $buildLog
    if ($LASTEXITCODE -ne 0) {
        $summary.Build = "FAILED (exit $LASTEXITCODE)"
        Write-Host ($summary | Out-String)
        Fail "build failed, see $buildLog"
    }
    $summary.Build = 'passed'
}

# ---------- tests ----------
if (-not $SkipTests) {
    Write-Step "Automation tests: $TestFilter ($Rhi)"
    $reportDir = Join-Path $outDir 'report'
    $testLog = Join-Path $outDir 'tests.log'
    $rhiArg = if ($Rhi -eq 'NullRHI') { '-NullRHI' } else { '-RenderOffscreen' }
    $testArgs = @(
        $HostUProject, '-Unattended', '-NoP4', '-NoSplash', '-NoSound', $rhiArg,
        "-ExecCmds=Automation RunTests $TestFilter", '-TestExit=Automation Test Queue Empty',
        "-ReportExportPath=$reportDir", "-AbsLog=$testLog", '-stdout', '-FullStdOutLogOutput'
    )
    Write-Host "& '$editorCmd' $($testArgs -join ' ')"
    $proc = Start-Process -FilePath $editorCmd -ArgumentList ($testArgs | ForEach-Object { if ($_ -match '\s') { "`"$_`"" } else { $_ } }) `
        -NoNewWindow -PassThru -RedirectStandardOutput (Join-Path $outDir 'tests-stdout.log') -RedirectStandardError (Join-Path $outDir 'tests-stderr.log')
    if (-not $proc.WaitForExit($TestTimeoutMinutes * 60 * 1000)) {
        $proc.Kill()
        Fail "tests timed out after $TestTimeoutMinutes min"
    }

    $index = Join-Path $reportDir 'index.json'
    if (-not (Test-Path -LiteralPath $index)) { Fail "no automation report produced ($index); see $testLog" }
    # index.json may contain a BOM and non-strict trailing commas in some engine versions
    $json = (Get-Content -LiteralPath $index -Raw -Encoding UTF8) -replace ',(\s*[\]}])', '$1' | ConvertFrom-Json

    function Get-Count($obj, [string] $name) { if ($obj.PSObject.Properties.Name -contains $name) { [int]$obj.$name } else { 0 } }
    $summary.Passed = Get-Count $json 'succeeded'
    $summary.Warnings = Get-Count $json 'succeededWithWarnings'
    $summary.Failed = Get-Count $json 'failed'
    $summary.NotRun = Get-Count $json 'notRun'

    if ($json.PSObject.Properties.Name -contains 'tests') {
        $failedTests = @($json.tests | Where-Object { $_.state -eq 'Fail' })
        foreach ($t in $failedTests) {
            Write-Host "FAILED: $($t.fullTestPath)" -ForegroundColor Red
            foreach ($e in @($t.entries)) {
                if ($e.event.type -eq 'Error') { Write-Host "    $($e.event.message)" }
            }
        }
    }
    $total = $summary.Passed + $summary.Warnings + $summary.Failed
    $summary.Tests = if ($total -eq 0) { 'NO TESTS MATCHED' } elseif ($summary.Failed -gt 0) { 'FAILED' } else { 'passed' }
}

# ---------- summary ----------
Write-Step 'Summary'
$summary | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $outDir 'summary.json') -Encoding UTF8
Write-Host ($summary | Out-String)
if ($summary.Tests -in @('FAILED', 'NO TESTS MATCHED')) { Fail "tests: $($summary.Tests)" }
Write-Host 'RESULT: PASS' -ForegroundColor Green
exit 0
