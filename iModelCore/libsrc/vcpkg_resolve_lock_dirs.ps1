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
# spellings of one directory de-duplicate to a single lock. Sort-Object -Unique then gives every
# process the same global acquire order, preventing an AB/BA livelock between runs whose override
# paths list the same pair in opposite order.
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
    try { $path = [System.IO.Path]::GetFullPath($path) } catch { }

    $FILE_SHARE_ALL = 7
    $OPEN_EXISTING = 3
    $FILE_FLAG_BACKUP_SEMANTICS = 0x02000000   # required to open a directory handle
    $handle = [VcpkgLock.Native]::CreateFileW($path, 0, $FILE_SHARE_ALL, [System.IntPtr]::Zero, $OPEN_EXISTING, $FILE_FLAG_BACKUP_SEMANTICS, [System.IntPtr]::Zero)
    if ($handle.ToInt64() -eq -1) { return $path }   # e.g. missing dir; the .bat re-checks existence

    try {
        $sb = New-Object System.Text.StringBuilder 32768
        $len = [VcpkgLock.Native]::GetFinalPathNameByHandleW($handle, $sb, $sb.Capacity, 0)
        if ($len -le 0) { return $path }
        $resolved = $sb.ToString()
        if ($resolved.StartsWith('\\?\UNC\')) { return '\\' + $resolved.Substring(8) }
        if ($resolved.StartsWith('\\?\'))     { return $resolved.Substring(4) }
        return $resolved
    }
    finally {
        [void][VcpkgLock.Native]::CloseHandle($handle)
    }
}

@($env:VCPKG_DOWNLOADS, $env:X_VCPKG_REGISTRIES_CACHE) |
    ForEach-Object { Resolve-Final $_ } |
    Where-Object { $_ } |
    Sort-Object -Unique
