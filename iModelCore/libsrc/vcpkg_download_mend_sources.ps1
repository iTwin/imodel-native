#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Materialize vcpkg sources for Mend using each consumer's vcpkg-mend.json triplet list.
# -ValidateOnly checks that configuration and downloads nothing, so the normal build can fail on a
# missing or malformed vcpkg-mend.json instead of leaving it to the Mend pipeline to discover.
#---------------------------------------------------------------------------------------------
[CmdletBinding(DefaultParameterSetName = 'Download')]
param(
    [Parameter(Mandatory = $true, Position = 0, ParameterSetName = 'Download')] [string] $ScanRoot,
    [Parameter(Mandatory = $true, ParameterSetName = 'Validate')] [switch] $ValidateOnly
)

$ErrorActionPreference = 'Stop'

# Resolves relative input against PowerShell's current location; [System.IO.Path]::GetFullPath
# would silently use the process working directory instead, which ScanRoot is then deleted from.
# GetFullPath is safe on the already-absolute result, and is what collapses mixed separators and
# '.'/'..' segments so that the prefix comparisons below cannot be defeated by spelling.
function Resolve-InputPath([string] $path) {
    $absolute = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path)
    return [System.IO.Path]::GetFullPath($absolute).TrimEnd('\', '/')
}

# The script ships in libsrc, so its own directory is always the tree to scan.
$LibsrcDir = Resolve-InputPath $PSScriptRoot
$isWindowsHost = [System.IO.Path]::DirectorySeparatorChar -eq '\'
$wrapperName = if ($isWindowsHost) { 'vcpkg_run_install.ps1' } else { 'vcpkg_run_install.sh' }
$wrapper = [System.IO.Path]::Combine($LibsrcDir, $wrapperName)
# Run the wrapper under whichever PowerShell is running this, rather than whatever 'powershell' or
# 'pwsh' happens to be on PATH, so the two cannot end up on different engines.
$psHostExe = if ($isWindowsHost) { (Get-Process -Id $PID).Path } else { $null }

if (-not [System.IO.File]::Exists($wrapper)) {
    throw "vcpkg install wrapper not found at '$wrapper'"
}

if (-not $ValidateOnly) {
    $ScanRoot = Resolve-InputPath $ScanRoot

    # ScanRoot is deleted recursively below, so refuse anything that would take the source tree with
    # it.  Compare case-insensitively on every host: whether the filesystem folds case varies by
    # platform and by volume, and for this guard over-matching only costs a loud failure while
    # under-matching deletes.
    if (-not $ScanRoot -or $ScanRoot -eq [System.IO.Path]::GetPathRoot($ScanRoot).TrimEnd('\', '/')) {
        throw "scan root '$ScanRoot' must not be a filesystem root"
    }
    if ($LibsrcDir.Equals($ScanRoot, [System.StringComparison]::OrdinalIgnoreCase) -or
        $LibsrcDir.StartsWith($ScanRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "scan root '$ScanRoot' must not contain the libsrc directory '$LibsrcDir'"
    }
    # Manifest discovery below runs before the scan root is emptied, so a nested one would be discovered.
    if ($ScanRoot.StartsWith($LibsrcDir + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "scan root '$ScanRoot' must not be inside the libsrc directory '$LibsrcDir'"
    }
}

$manifests = @()
foreach ($manifestFile in @(Get-ChildItem -Path $LibsrcDir -Filter vcpkg.json -File -Recurse | Sort-Object FullName)) {
    $manifestDir = $manifestFile.Directory.FullName
    if ([System.IO.File]::Exists([System.IO.Path]::Combine($manifestDir, 'vcpkg-configuration.json'))) {
        $manifests += $manifestFile
        continue
    }

    # Port manifests nested under a consumer (ports/, overlay-ports/) are not consumers themselves.
    $nestedInConsumer = $false
    for ($ancestor = $manifestFile.Directory.Parent;
         $ancestor -and $ancestor.FullName.Length -gt $LibsrcDir.Length;
         $ancestor = $ancestor.Parent) {
        if ([System.IO.File]::Exists([System.IO.Path]::Combine($ancestor.FullName, 'vcpkg-configuration.json'))) {
            $nestedInConsumer = $true
            break
        }
    }
    if (-not $nestedInConsumer) {
        # Skipping this silently would drop the library out of the Mend scan without any signal.
        throw "'$($manifestFile.FullName)' has no sibling 'vcpkg-configuration.json', so it is not recognized as a vcpkg consumer"
    }
}
if ($manifests.Count -eq 0) {
    throw "no vcpkg consumer manifests found under '$LibsrcDir'"
}

$consumerDirs = New-Object 'System.Collections.Generic.HashSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($manifestFile in $manifests) {
    [void]$consumerDirs.Add($manifestFile.Directory.FullName)
}
foreach ($strayConfig in @(Get-ChildItem -Path $LibsrcDir -Filter vcpkg-mend.json -File -Recurse)) {
    if (-not $consumerDirs.Contains($strayConfig.Directory.FullName)) {
        throw "'$($strayConfig.FullName)' is not beside a recognized vcpkg consumer manifest"
    }
}

# Everything each consumer needs is read and checked up front: a typo in the last consumer must not
# cost the existing scan output plus a full download of every consumer ahead of it.
$plan = @()
foreach ($manifestFile in $manifests) {
    $manifestDir = $manifestFile.Directory.FullName
    $consumer = $manifestDir.Substring($LibsrcDir.Length).TrimStart('\', '/')
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
    }

    $manifest = Get-Content -LiteralPath $manifestFile.FullName -Raw | ConvertFrom-Json

    # 'dependencies' names only the direct ports; 'overrides' pins the whole transitively resolved
    # set, so including it is what catches a transitive port that silently stops being pulled in.
    $ports = New-Object 'System.Collections.Generic.SortedSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($entry in (@($manifest.dependencies) + @($manifest.overrides))) {
        $name = if ($entry -is [string]) { $entry } else { $entry.name }
        if ($name) {
            [void]$ports.Add($name)
        }
    }

    $plan += [PSCustomObject]@{
        Consumer    = $consumer
        ManifestDir = $manifestDir
        Triplets    = $triplets
        Ports       = $ports
    }
}

if ($ValidateOnly) {
    Write-Output "Validated Mend scan configuration for $($plan.Count) vcpkg consumers under '$LibsrcDir'."
    return
}

if ([System.IO.Directory]::Exists($ScanRoot)) {
    if ($isWindowsHost) {
        # Extracted vcpkg buildtrees paths routinely exceed MAX_PATH, which Remove-Item cannot delete.
        & cmd.exe /c rmdir /s /q "$ScanRoot"
        if ([System.IO.Directory]::Exists($ScanRoot)) {
            throw "could not remove existing scan root '$ScanRoot'"
        }
    }
    else {
        Remove-Item -LiteralPath $ScanRoot -Force -Recurse
    }
}
[void][System.IO.Directory]::CreateDirectory($ScanRoot)

foreach ($consumerPlan in $plan) {
    $consumer = $consumerPlan.Consumer
    $manifestDir = $consumerPlan.ManifestDir
    foreach ($triplet in $consumerPlan.Triplets) {
        $installRoot = [System.IO.Path]::Combine($ScanRoot, $consumer, $triplet)
        Write-Output "Materializing vcpkg sources for $consumer ($triplet)"
        # A child process, not dot-sourcing: the wrapper calls Add-Type and sets VCPKG_* variables,
        # neither of which survives a second invocation in one session.
        if ($isWindowsHost) {
            & $psHostExe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File $wrapper $manifestDir $installRoot $triplet -MendScan
        }
        else {
            & /bin/bash $wrapper $manifestDir $installRoot $triplet --mend-scan
        }
        if ($LASTEXITCODE -ne 0) {
            throw "vcpkg source materialization failed for $consumer ($triplet) with exit code $LASTEXITCODE"
        }
    }
}

$missing = @()
foreach ($consumerPlan in $plan) {
    $consumer = $consumerPlan.Consumer
    foreach ($dependency in $consumerPlan.Ports) {
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

Write-Output "Materialized and validated vcpkg sources for $($plan.Count) consumers under '$ScanRoot'."
