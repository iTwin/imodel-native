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

# Resolves relative input against PowerShell's current location; [System.IO.Path]::GetFullPath
# would silently use the process working directory instead, which ScanRoot is then deleted from.
function Resolve-InputPath([string] $path) {
    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path).TrimEnd('\', '/')
}

$LibsrcDir = Resolve-InputPath $LibsrcDir
$ScanRoot = Resolve-InputPath $ScanRoot
$isWindowsHost = [System.IO.Path]::DirectorySeparatorChar -eq '\'
$wrapperName = if ($isWindowsHost) { 'vcpkg_run_install.ps1' } else { 'vcpkg_run_install.sh' }
$wrapper = [System.IO.Path]::Combine($LibsrcDir, $wrapperName)

if (-not [System.IO.File]::Exists($wrapper)) {
    throw "vcpkg install wrapper not found at '$wrapper'"
}

# ScanRoot is deleted recursively below, so refuse anything that would take the source tree with it.
# Compare case-insensitively on every host: whether the filesystem folds case varies by platform and
# by volume, and for this guard over-matching only costs a loud failure while under-matching deletes.
if (-not $ScanRoot -or $ScanRoot -eq [System.IO.Path]::GetPathRoot($ScanRoot).TrimEnd('\', '/')) {
    throw "scan root '$ScanRoot' must not be a filesystem root"
}
if ($LibsrcDir.Equals($ScanRoot, [System.StringComparison]::OrdinalIgnoreCase) -or
    $LibsrcDir.StartsWith($ScanRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "scan root '$ScanRoot' must not contain the libsrc directory '$LibsrcDir'"
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
    $consumer = $manifestDir.Substring($LibsrcDir.Length).TrimStart('\', '/')
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

    foreach ($dependency in $ports) {
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