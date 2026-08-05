#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Resolve the two directories vcpkg_run_install.bat locks -- the downloads/tools tree
# (VCPKG_DOWNLOADS) and the registries git repo (X_VCPKG_REGISTRIES_CACHE) -- to their FINAL
# on-disk identities, de-duplicate them case-insensitively, and sort them into one deterministic
# order. Reads the two paths from those environment variables and writes the resulting lock
# directories to stdout, one per line (zero, one, or two lines), followed by a completion sentinel.
# Before emitting, each canonical lock file is preflighted for append access so an unwritable cache
# dir is reported here rather than silently burning the caller's lock-acquisition retry budget.
# Output is written as UTF-8 so paths with characters outside the console's OEM code page survive
# the pipe to the batch consumer (which switches its console to code page 65001 to match).
#
# GetFinalPathNameByHandle collapses junction/symlink aliases, 8.3 short names and case
# differences that a plain absolute-path normalization (e.g. cmd's %~fI) leaves intact, so two
# spellings of one directory de-duplicate to a single lock. An OrdinalIgnoreCase sort then gives
# every process the same global acquire order regardless of user culture, preventing an AB/BA
# livelock between runs whose override paths list the same pair in opposite order.
#---------------------------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'

# Emit stdout as UTF-8 (no BOM) so canonical paths with characters outside the console's OEM code
# page survive the pipe to the batch consumer, which switches its console to code page 65001 to
# match. Without this a non-OEM path would arrive as '?' replacement characters while the ASCII
# completion sentinel still parsed, silently corrupting a lock directory. Guard the assignment: a
# rare host without a valid console handle would throw here, and losing UTF-8 fidelity is better
# than failing outright.
try { [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding $false } catch { }

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

function Test-LockAppendable([string] $dir) {
    # Preflight the ACTUAL lock file (not a throwaway probe): open '<dir>\.vcpkg-install.lock' for
    # append with shared read/write. Creating a random probe file only proves the directory accepts
    # new files; it does not prove this specific, possibly pre-existing, lock file can be opened for
    # append -- in a shared cache it may be read-only or carry another user's ACL. Classify the
    # failure: a sharing/lock violation means another run legitimately holds the lock right now, so
    # the path and ACL are fine (retry briefly, then treat as appendable); anything else (access
    # denied, read-only, path error) is permanent and must fail immediately rather than be
    # misclassified as contention and burn the caller's ~1h retry budget.
    $lock = [System.IO.Path]::Combine($dir, '.vcpkg-install.lock')
    $ERROR_SHARING_VIOLATION = 32
    $ERROR_LOCK_VIOLATION = 33
    for ($attempt = 0; $attempt -lt 10; $attempt++) {
        try {
            $fs = [System.IO.File]::Open($lock, [System.IO.FileMode]::Append, [System.IO.FileAccess]::Write, [System.IO.FileShare]::ReadWrite)
            $fs.Dispose()
            return
        }
        catch [System.IO.IOException] {
            # Subclasses (FileNotFound/DirectoryNotFound) also land here; only genuine sharing/lock
            # violations are transient, everything else is permanent and rethrown to the outer catch.
            $code = $_.Exception.HResult -band 0xFFFF
            if ($code -eq $ERROR_SHARING_VIOLATION -or $code -eq $ERROR_LOCK_VIOLATION) {
                Start-Sleep -Milliseconds 200
                continue
            }
            throw "vcpkg lock file '$lock' is not appendable: $($_.Exception.Message)"
        }
        # UnauthorizedAccessException (access denied / read-only) is intentionally NOT caught here so
        # it propagates to the outer catch and fails the resolver immediately.
    }
    # Retries exhausted on sharing/lock violation only: another run is holding the lock, which proves
    # the path is writable, so treat it as appendable.
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
    # Preflight append access to every canonical lock file BEFORE emitting anything, so a permanent
    # permission/path failure aborts with no stdout and the caller never locks a half-validated set.
    foreach ($d in $locks) { Test-LockAppendable $d }
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
