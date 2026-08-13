#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Materialize vcpkg sources for Mend using each consumer's vcpkg-mend.json triplet list.
#---------------------------------------------------------------------------------------------
param(
    [Parameter(Mandatory = $true)] [string] $LibsrcDir,
    [Parameter(Mandatory = $true)] [string] $ScanRoot
)

$ErrorActionPreference = 'Stop'
$LibsrcDir = [System.IO.Path]::GetFullPath($LibsrcDir)
$ScanRoot = [System.IO.Path]::GetFullPath($ScanRoot)
$isWindowsHost = [System.IO.Path]::DirectorySeparatorChar -eq '\'
$wrapperName = if ($isWindowsHost) { 'vcpkg_run_install.ps1' } else { 'vcpkg_run_install.sh' }
$wrapper = [System.IO.Path]::Combine($LibsrcDir, $wrapperName)

if (-not [System.IO.File]::Exists($wrapper)) {
    throw "vcpkg install wrapper not found at '$wrapper'"
}

if ([System.IO.Directory]::Exists($ScanRoot)) {
    Remove-Item -LiteralPath $ScanRoot -Force -Recurse
}
[void][System.IO.Directory]::CreateDirectory($ScanRoot)

$manifests = @(Get-ChildItem -Path $LibsrcDir -Filter vcpkg.json -File -Recurse |
    Where-Object { [System.IO.File]::Exists([System.IO.Path]::Combine($_.Directory.FullName, 'vcpkg-configuration.json')) } |
    Sort-Object FullName)
if ($manifests.Count -eq 0) {
    throw "no vcpkg consumer manifests found under '$LibsrcDir'"
}

foreach ($manifestFile in $manifests) {
    $manifestDir = $manifestFile.Directory.FullName
    $consumer = $manifestDir.Substring($LibsrcDir.TrimEnd('\', '/').Length).TrimStart('\', '/')
    $mendConfigPath = [System.IO.Path]::Combine($manifestDir, 'vcpkg-mend.json')
    if (-not [System.IO.File]::Exists($mendConfigPath)) {
        throw "vcpkg consumer '$consumer' is missing required Mend configuration '$mendConfigPath'"
    }

    $mendConfig = Get-Content -LiteralPath $mendConfigPath -Raw | ConvertFrom-Json
    $triplets = @($mendConfig.triplets)
    if ($triplets.Count -eq 0 -or $triplets.Where({ -not $_ }).Count -ne 0) {
        throw "'$mendConfigPath' must contain a non-empty 'triplets' array"
    }

    foreach ($triplet in $triplets) {
        $tripletFile = [System.IO.Path]::Combine($manifestDir, 'triplets', "$triplet.cmake")
        if (-not [System.IO.File]::Exists($tripletFile)) {
            throw "Mend triplet '$triplet' for '$consumer' does not exist at '$tripletFile'"
        }

        $installRoot = [System.IO.Path]::Combine($ScanRoot, $consumer, $triplet)
        Write-Output "Materializing vcpkg sources for $consumer ($triplet)"
        if ($isWindowsHost) {
            & powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File $wrapper $manifestDir $installRoot $triplet -OnlyDownloads -DisableBinaryCache
        }
        else {
            & /bin/bash $wrapper $manifestDir $installRoot $triplet --only-downloads --disable-binary-cache
        }
        if ($LASTEXITCODE -ne 0) {
            throw "vcpkg source materialization failed for $consumer ($triplet) with exit code $LASTEXITCODE"
        }
    }
}

$missing = @()
foreach ($manifestFile in $manifests) {
    $manifestDir = $manifestFile.Directory.FullName
    $consumer = $manifestDir.Substring($LibsrcDir.TrimEnd('\', '/').Length).TrimStart('\', '/')
    $manifest = Get-Content -LiteralPath $manifestFile.FullName -Raw | ConvertFrom-Json
    foreach ($dependencyEntry in $manifest.dependencies) {
        $dependency = if ($dependencyEntry -is [string]) { $dependencyEntry } else { $dependencyEntry.name }
        $sourceRoots = @(Get-Item -Path ([System.IO.Path]::Combine($ScanRoot, $consumer, '*', 'buildtrees', $dependency, 'src')) -ErrorAction SilentlyContinue |
            Where-Object { $_.PSIsContainer })
        if ($sourceRoots.Count -eq 0 -or
            -not ($sourceRoots | Where-Object { @(Get-ChildItem -LiteralPath $_.FullName -Directory -ErrorAction SilentlyContinue).Count -ne 0 })) {
            $missing += "$consumer -> $dependency"
        }
    }
}

if ($missing.Count -ne 0) {
    throw "vcpkg did not materialize source for:`n$($missing -join "`n")"
}

Write-Output "Materialized and validated vcpkg sources for $($manifests.Count) consumers under '$ScanRoot'."