#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Dot-source this into a script to get shared vcpkg-mend.json validation, so the check cannot drift
# between the consumers. The equivalent check on non-Windows lives in vcpkg_run_install.sh.
#---------------------------------------------------------------------------------------------

# Checks the parsed shape rather than wrapping in @(), which would silently accept a scalar
# "triplets": "x64-windows" as a one-element array and disagree with the shell wrapper.
function Read-MendTriplets([string] $mendConfigPath) {
    try {
        $mendConfig = Get-Content -LiteralPath $mendConfigPath -Raw | ConvertFrom-Json -ErrorAction Stop
    }
    catch {
        throw "'$mendConfigPath' is not valid JSON: $($_.Exception.Message)"
    }
    if ($mendConfig -isnot [System.Management.Automation.PSCustomObject]) {
        throw "'$mendConfigPath' must contain a JSON object"
    }
    $triplets = $mendConfig.triplets
    if ($triplets -isnot [System.Object[]] -or $triplets.Count -eq 0 -or
        $triplets.Where({ $_ -isnot [string] -or [string]::IsNullOrWhiteSpace($_) }).Count -ne 0) {
        throw "'$mendConfigPath' must contain a 'triplets' array of non-empty strings"
    }
    return $triplets
}

# vcpkg builds these solely to build other ports (cmake helpers, gn, python packaging, the 7-Zip
# extractor, etc.); none of it is linked into or shipped with our product, so Mend should never see
# it. vcpkg itself names its own helper ports with a "vcpkg-" prefix; the rest of this list is
# non-"vcpkg-" ports that are still build-only, found by auditing what Mend flagged.
$script:MendBuiltinBuildOnlyPorts = New-Object 'System.Collections.Generic.HashSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($name in @('sevenzip', 'gn', 'detect_compiler')) {
    [void]$script:MendBuiltinBuildOnlyPorts.Add($name)
}

# A consumer's vcpkg-mend.json may list additional build-only port names under "excludePorts" (e.g.
# a tool pulled in only by that consumer's own graph) beyond the shared list above.
function Read-MendExcludePorts([string] $mendConfigPath) {
    $mendConfig = Get-Content -LiteralPath $mendConfigPath -Raw | ConvertFrom-Json -ErrorAction Stop
    $excludePorts = $mendConfig.excludePorts
    if ($null -eq $excludePorts) {
        return @()
    }
    if ($excludePorts -isnot [System.Object[]] -or
        $excludePorts.Where({ $_ -isnot [string] -or [string]::IsNullOrWhiteSpace($_) }).Count -ne 0) {
        throw "'$mendConfigPath' 'excludePorts', if present, must be an array of non-empty strings"
    }
    return $excludePorts
}

# True if $portName exists only to build other ports and must be stripped from a Mend scan tree.
function Test-MendBuildOnlyPort([string] $portName, [string[]] $consumerExcludePorts) {
    if ($portName.StartsWith('vcpkg-', [System.StringComparison]::OrdinalIgnoreCase)) {
        return $true
    }
    if ($script:MendBuiltinBuildOnlyPorts.Contains($portName)) {
        return $true
    }
    return $consumerExcludePorts -and ($consumerExcludePorts -contains $portName)
}
