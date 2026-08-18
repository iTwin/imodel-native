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
