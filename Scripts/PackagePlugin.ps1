[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $PluginDir,

    [Parameter(Mandatory = $true)]
    [string] $TargetPlatform,

    [Parameter(Mandatory = $true)]
    [string] $TargetConfiguration
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$pluginRoot = (Resolve-Path -LiteralPath $PluginDir).Path
$descriptor = Get-ChildItem -LiteralPath $pluginRoot -Filter '*.uplugin' -File | Select-Object -First 1
if ($null -eq $descriptor) {
    throw "No .uplugin descriptor found in '$pluginRoot'."
}

$pluginName = [System.IO.Path]::GetFileNameWithoutExtension($descriptor.Name)
$outputDir = Join-Path $pluginRoot 'build'
$archiveName = '{0}-{1}-{2}.zip' -f $pluginName, $TargetPlatform, $TargetConfiguration
$archivePath = Join-Path $outputDir $archiveName
$stagingRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("{0}-package-{1}" -f $pluginName, [Guid]::NewGuid().ToString('N'))
$stagedPlugin = Join-Path $stagingRoot $pluginName

$packageEntries = @(
    'Binaries',
    'Config',
    'Content',
    'Resources',
    'Scripts',
    'Shaders',
    'Source'
)

try {
    New-Item -ItemType Directory -Path $stagedPlugin -Force | Out-Null

    Copy-Item -LiteralPath $descriptor.FullName -Destination $stagedPlugin
    foreach ($entry in $packageEntries) {
        $sourcePath = Join-Path $pluginRoot $entry
        if (Test-Path -LiteralPath $sourcePath) {
            Copy-Item -LiteralPath $sourcePath -Destination $stagedPlugin -Recurse
        }
    }

    Get-ChildItem -LiteralPath $stagedPlugin -Directory -Recurse -Force |
        Where-Object { $_.Name -in @('.idea', '.vs', '.vscode', 'Intermediate', 'Saved') } |
        Sort-Object { $_.FullName.Length } -Descending |
        Remove-Item -Recurse -Force

    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    if (Test-Path -LiteralPath $archivePath) {
        Remove-Item -LiteralPath $archivePath -Force
    }

    Compress-Archive -LiteralPath $stagedPlugin -DestinationPath $archivePath -CompressionLevel Optimal
    Write-Host "LandscapeHeightmapTracker package created: $archivePath"
}
finally {
    if (Test-Path -LiteralPath $stagingRoot) {
        Remove-Item -LiteralPath $stagingRoot -Recurse -Force
    }
}
