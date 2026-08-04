#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Resolve the two directories vcpkg_run_install.bat locks -- the downloads/tools tree
# (VCPKG_DOWNLOADS) and the registries git repo (X_VCPKG_REGISTRIES_CACHE) -- to their FINAL
# on-disk identities, de-duplicate them case-insensitively, and sort them into one deterministic
# order. Reads the two paths from those environment variables and writes the resulting lock
# directories to stdout, one per line (zero, one, or two lines).
#
# GetFinalPathNameByHandle collapses junction/symlink aliases, 8.3 short names and case
# differences that a plain absolute-path normalization (e.g. cmd's %~fI) leaves intact, so two
# spellings of one directory de-duplicate to a single lock. An OrdinalIgnoreCase sort then gives
# every process the same global acquire order regardless of user culture, preventing an AB/BA
# livelock between runs whose override paths list the same pair in opposite order.
#---------------------------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'

Add-Type -Namespace VcpkgLock -Name Native -MemberDefinition @'
[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
public static extern System.IntPtr CreateFileW(string name, uint access, uint share, System.IntPtr sa, uint disposition, uint flags, System.IntPtr template);

[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
public static extern int GetFinalPathNameByHandleW(System.IntPtr handle, System.Text.StringBuilder path, int cch, int flags);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern bool CloseHandle(System.IntPtr handle);
'@

function Resolve-Final([string] $path) {
    if ([string]::IsNullOrEmpty($path)) { return }

    # Fail closed: if any step below cannot produce the canonical final path, throw (nonzero exit)
    # instead of returning the raw spelling. Returning an unresolved path would let two aliases of
    # one directory survive as different strings and self-contend or acquire in different orders --
    # exactly the failure this helper exists to prevent (e.g. a restricted SMB share that is
    # openable/traversable but denies final-name normalization).
    $path = [System.IO.Path]::GetFullPath($path)

    $FILE_SHARE_ALL = 7
    $OPEN_EXISTING = 3
    $FILE_FLAG_BACKUP_SEMANTICS = 0x02000000   # required to open a directory handle
    $handle = [VcpkgLock.Native]::CreateFileW($path, 0, $FILE_SHARE_ALL, [System.IntPtr]::Zero, $OPEN_EXISTING, $FILE_FLAG_BACKUP_SEMANTICS, [System.IntPtr]::Zero)
    if ($handle.ToInt64() -eq -1) {
        $err = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "CreateFileW('$path') failed with Win32 error $err"
    }

    try {
        $sb = New-Object System.Text.StringBuilder 32768
        $len = [VcpkgLock.Native]::GetFinalPathNameByHandleW($handle, $sb, $sb.Capacity, 0)
        if ($len -le 0 -or $len -ge $sb.Capacity) {
            $err = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            throw "GetFinalPathNameByHandle('$path') failed with Win32 error $err (returned $len)"
        }
        $resolved = $sb.ToString()
        if ($resolved.StartsWith('\\?\UNC\')) { return '\\' + $resolved.Substring(8) }
        if ($resolved.StartsWith('\\?\'))     { return $resolved.Substring(4) }
        return $resolved
    }
    finally {
        [void][VcpkgLock.Native]::CloseHandle($handle)
    }
}

try {
    # Buffer the full result before emitting anything: if any path fails to canonicalize we must
    # abort with no stdout so the caller never locks a half-resolved set. Windows PowerShell's
    # -File mode does NOT turn an uncaught throw into a nonzero exit code, so translate it here.
    #
    # Order and de-duplicate with an explicit OrdinalIgnoreCase comparer, not Sort-Object -Unique
    # (which uses the current user's culture): these override dirs are shared across users, so a
    # culture-sensitive sort could order the same Unicode path pair differently per user and
    # reintroduce the reversed-acquisition livelock. SortedSet gives one locale-independent order
    # and case-insensitive de-dup in a single step.
    $locks = New-Object 'System.Collections.Generic.SortedSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($p in @($env:VCPKG_DOWNLOADS, $env:X_VCPKG_REGISTRIES_CACHE)) {
        $resolved = Resolve-Final $p
        if ($resolved) { [void]$locks.Add($resolved) }
    }
    $locks | ForEach-Object { $_ }
    # Completion sentinel, printed only after every lock dir resolved. The caller treats its
    # absence as failure, so a resolver that dies partway can never be mistaken for a short but
    # "valid" one-line result. A canonical path is always absolute, so it cannot collide with
    # this bare token.
    Write-Output '__VCPKG_LOCK_OK__'
}
catch {
    [Console]::Error.WriteLine($_.Exception.Message)
    exit 1
}
