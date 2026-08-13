#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Wrapper for vcpkg install, invoked from .mke build files.
# Usage: vcpkg_run_install.ps1 <manifest_dir> <install_root> <triplet> [-MendScan]
#   -MendScan: materialize sources for a Mend scan instead of building (download only, no binary
#              cache, no compiler tracking).
#---------------------------------------------------------------------------------------------
param(
    [Parameter(Position = 0)] [string] $ManifestDir,
    [Parameter(Position = 1)] [string] $InstallRoot,
    [Parameter(Position = 2)] [string] $Triplet,
    [switch] $MendScan
)

$ErrorActionPreference = 'Stop'

function Normalize-Path([string] $path) {
    $full = [System.IO.Path]::GetFullPath($path)
    $root = [System.IO.Path]::GetPathRoot($full)
    while ($full.Length -gt $root.Length -and ($full.EndsWith('\') -or $full.EndsWith('/'))) {
        $full = $full.Substring(0, $full.Length - 1)
    }
    return $full
}

function Ensure-Directory([string] $path) {
    try {
        [void][System.IO.Directory]::CreateDirectory($path)
        return [System.IO.Directory]::Exists($path)
    }
    catch {
        return $false
    }
}

function Resolve-Final([string] $path) {
    $path = [System.IO.Path]::GetFullPath($path)
    $FILE_SHARE_ALL = 7
    $OPEN_EXISTING = 3
    $FILE_FLAG_BACKUP_SEMANTICS = 0x02000000
    $handle = [VcpkgLock.Native]::CreateFileW($path, 0, $FILE_SHARE_ALL, [System.IntPtr]::Zero, $OPEN_EXISTING, $FILE_FLAG_BACKUP_SEMANTICS, [System.IntPtr]::Zero)
    if ($handle.ToInt64() -eq -1) {
        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "CreateFileW('$path') failed with Win32 error $errorCode"
    }

    try {
        $buffer = New-Object System.Text.StringBuilder 32768
        $length = [VcpkgLock.Native]::GetFinalPathNameByHandleW($handle, $buffer, $buffer.Capacity, 0)
        if ($length -le 0 -or $length -ge $buffer.Capacity) {
            $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            throw "GetFinalPathNameByHandle('$path') failed with Win32 error $errorCode (returned $length)"
        }

        $resolved = $buffer.ToString()
        if ($resolved.StartsWith('\\?\UNC\')) { return '\\' + $resolved.Substring(8) }
        if ($resolved.StartsWith('\\?\')) { return $resolved.Substring(4) }
        return $resolved
    }
    finally {
        [void][VcpkgLock.Native]::CloseHandle($handle)
    }
}

function Quote-WindowsCommandLineArg([string] $value) {
    if ($null -eq $value) {
        return '""'
    }
    if ($value.Length -eq 0) {
        return '""'
    }
    if ($value -notmatch '[\s"]') {
        return $value
    }

    $builder = New-Object System.Text.StringBuilder
    [void]$builder.Append('"')
    $backslashCount = 0
    foreach ($c in $value.ToCharArray()) {
        if ($c -eq '\\') {
            $backslashCount++
            continue
        }

        if ($c -eq '"') {
            if ($backslashCount -gt 0) {
                [void]$builder.Append((New-Object string('\\', $backslashCount * 2)))
            }
            [void]$builder.Append('\\"')
            $backslashCount = 0
            continue
        }

        if ($backslashCount -gt 0) {
            [void]$builder.Append((New-Object string('\\', $backslashCount)))
            $backslashCount = 0
        }
        [void]$builder.Append($c)
    }

    if ($backslashCount -gt 0) {
        [void]$builder.Append((New-Object string('\\', $backslashCount * 2)))
    }
    [void]$builder.Append('"')
    return $builder.ToString()
}

function Invoke-NativeProcessInKillOnCloseJob([string] $exePath, [string[]] $arguments) {
    $job = [VcpkgLock.Native]::CreateJobObjectW([System.IntPtr]::Zero, $null)
    if ($job -eq [System.IntPtr]::Zero) {
        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "CreateJobObjectW failed with Win32 error $errorCode"
    }

    $process = $null
    try {
        $limits = New-Object VcpkgLock.JOBOBJECT_EXTENDED_LIMIT_INFORMATION
        $limits.BasicLimitInformation.LimitFlags = [VcpkgLock.Native]::JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE

        $limitsSize = [System.Runtime.InteropServices.Marshal]::SizeOf([type] [VcpkgLock.JOBOBJECT_EXTENDED_LIMIT_INFORMATION])
        $limitsPtr = [System.Runtime.InteropServices.Marshal]::AllocHGlobal($limitsSize)
        try {
            [System.Runtime.InteropServices.Marshal]::StructureToPtr($limits, $limitsPtr, $false)
            if (-not [VcpkgLock.Native]::SetInformationJobObject($job, [VcpkgLock.Native]::JobObjectExtendedLimitInformation, $limitsPtr, [uint32] $limitsSize)) {
                $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                throw "SetInformationJobObject failed with Win32 error $errorCode"
            }
        }
        finally {
            [System.Runtime.InteropServices.Marshal]::FreeHGlobal($limitsPtr)
        }

        $startInfo = New-Object System.Diagnostics.ProcessStartInfo
        $startInfo.FileName = $exePath
        $startInfo.UseShellExecute = $false
        $quotedArgs = @($arguments | ForEach-Object { Quote-WindowsCommandLineArg $_ })
        $startInfo.Arguments = [string]::Join(' ', $quotedArgs)
        $process = [System.Diagnostics.Process]::Start($startInfo)
        if ($null -eq $process) {
            throw "failed to start process '$exePath'"
        }

        if (-not [VcpkgLock.Native]::AssignProcessToJobObject($job, $process.Handle)) {
            $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            try {
                if (-not $process.HasExited) {
                    $process.Kill()
                }
            }
            catch {
                # best effort cleanup before surfacing assignment failure
            }
            throw "AssignProcessToJobObject failed with Win32 error $errorCode"
        }

        $process.WaitForExit()
        return $process.ExitCode
    }
    finally {
        if ($process) {
            $process.Dispose()
        }
        [void][VcpkgLock.Native]::CloseHandle($job)
    }
}

try {
    if (-not $ManifestDir -or -not $InstallRoot -or -not $Triplet) {
        throw 'Usage: vcpkg_run_install.ps1 <manifest_dir> <install_root> <triplet> [-MendScan]'
    }

    $ManifestDir = Normalize-Path $ManifestDir
    $InstallRoot = Normalize-Path $InstallRoot

    $vsVcpkgRoot = $null
    if ($env:VCINSTALLDIR) {
        $vsVcpkgRoot = Normalize-Path ([System.IO.Path]::Combine($env:VCINSTALLDIR, 'vcpkg'))
    }

    $candidateRoots = New-Object 'System.Collections.Generic.List[string]'
    if ($env:IMODEL_VCPKG_ROOT) {
        $candidateRoots.Add((Normalize-Path $env:IMODEL_VCPKG_ROOT))
    }
    elseif ($env:SrcRoot) {
        $candidateRoots.Add((Normalize-Path "$($env:SrcRoot)vcpkg"))
    }
    if ($env:VCPKG_ROOT) {
        $candidate = Normalize-Path $env:VCPKG_ROOT
        if (-not $vsVcpkgRoot -or -not [string]::Equals($candidate, $vsVcpkgRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
            $candidateRoots.Add($candidate)
        }
    }
    $candidateRoots.Add('D:\src\vcpkg')
    if ($env:USERPROFILE) {
        $candidateRoots.Add((Normalize-Path ([System.IO.Path]::Combine($env:USERPROFILE, 'src\vcpkg'))))
    }
    if ($vsVcpkgRoot) {
        $candidateRoots.Add($vsVcpkgRoot)
    }

    $vcpkgExe = $null
    foreach ($candidateRoot in $candidateRoots) {
        $candidateExe = [System.IO.Path]::Combine($candidateRoot, 'vcpkg.exe')
        if ([System.IO.File]::Exists($candidateExe)) {
            $env:VCPKG_ROOT = $candidateRoot
            $vcpkgExe = $candidateExe
            break
        }
    }
    if (-not $vcpkgExe) {
        throw 'vcpkg was not found. Set IMODEL_VCPKG_ROOT to the standalone vcpkg installation directory.'
    }

    $cacheBase = if ($env:LOCALAPPDATA) {
        [System.IO.Path]::Combine($env:LOCALAPPDATA, 'Bentley\vcpkg')
    } else {
        [System.IO.Path]::Combine($InstallRoot, 'vcpkg-cache')
    }
    if (-not (Ensure-Directory $cacheBase)) {
        $cacheBase = [System.IO.Path]::Combine($InstallRoot, 'vcpkg-cache')
        Write-Output "vcpkg: persistent cache base unavailable; falling back to '$cacheBase'"
    }
    if (-not (Ensure-Directory $cacheBase)) {
        throw "vcpkg cache base '$cacheBase' could not be created or is not writable"
    }

    $tripletParts = $Triplet.Split('-')
    if ($tripletParts.Length -lt 2) {
        throw "invalid vcpkg triplet '$Triplet'"
    }
    $platformKey = "$($tripletParts[0])-$($tripletParts[1])"

    if (-not $env:VCPKG_DOWNLOADS) {
        $env:VCPKG_DOWNLOADS = [System.IO.Path]::Combine($cacheBase, "downloads\$platformKey")
    }
    if (-not (Ensure-Directory $env:VCPKG_DOWNLOADS)) {
        throw "vcpkg downloads directory '$($env:VCPKG_DOWNLOADS)' could not be created"
    }

    if (-not $env:X_VCPKG_REGISTRIES_CACHE) {
        $env:X_VCPKG_REGISTRIES_CACHE = [System.IO.Path]::Combine($env:VCPKG_DOWNLOADS, 'registries')
    }
    if (-not (Ensure-Directory $env:X_VCPKG_REGISTRIES_CACHE)) {
        throw "vcpkg registries cache '$($env:X_VCPKG_REGISTRIES_CACHE)' could not be created"
    }

    if (-not $env:VCPKG_DEFAULT_BINARY_CACHE) {
        $env:VCPKG_DEFAULT_BINARY_CACHE = [System.IO.Path]::Combine($cacheBase, 'archives')
    }
    if (-not (Ensure-Directory $env:VCPKG_DEFAULT_BINARY_CACHE)) {
        throw "vcpkg binary cache '$($env:VCPKG_DEFAULT_BINARY_CACHE)' could not be created"
    }

    if (-not $env:ANDROID_NDK_HOME -and $env:ANDROID_NDK_ROOT) {
        $env:ANDROID_NDK_HOME = $env:ANDROID_NDK_ROOT
    }

    $overlayTriplets = [System.IO.Path]::Combine($ManifestDir, 'triplets')
    $overlayTripletFile = [System.IO.Path]::Combine($overlayTriplets, "$Triplet.cmake")
    if (-not [System.IO.File]::Exists($overlayTripletFile)) {
        throw "no custom overlay triplet '$Triplet' found at '$overlayTripletFile'; vcpkg's built-in triplets must not be used"
    }
    $overlayPorts = [System.IO.Path]::Combine($ManifestDir, 'ports')

    # vcpkg probes the target triplet's compiler only to hash it into the package ABI, which fails
    # when the host cannot compile for that triplet (e.g. scanning x64-linux sources on Windows).
    # Shadow the repo triplet with a generated one that opts out; the repo triplet still supplies
    # every build setting, so nothing else about the invocation changes.
    if ($MendScan) {
        $generatedTriplets = [System.IO.Path]::Combine($InstallRoot, 'generated-triplets')
        if (-not (Ensure-Directory $generatedTriplets)) {
            throw "generated triplet directory '$generatedTriplets' could not be created"
        }
        $includePath = $overlayTripletFile.Replace('\', '/')
        [System.IO.File]::WriteAllText(
            [System.IO.Path]::Combine($generatedTriplets, "$Triplet.cmake"),
            "include(`"$includePath`")`r`nset(VCPKG_DISABLE_COMPILER_TRACKING ON)`r`n")
        $overlayTriplets = $generatedTriplets
        Write-Output "vcpkg: compiler tracking disabled; using generated triplet under '$generatedTriplets'"
    }

    Write-Output "vcpkg: installing packages from '$ManifestDir' (triplet=$Triplet, install-root=$InstallRoot)"
    Write-Output "vcpkg: exe='$vcpkgExe'"
    Write-Output "vcpkg: root='$($env:VCPKG_ROOT)'"
    Write-Output "vcpkg: cache-base='$cacheBase'"
    Write-Output "vcpkg: downloads='$($env:VCPKG_DOWNLOADS)'"
    Write-Output "vcpkg: registries-cache='$($env:X_VCPKG_REGISTRIES_CACHE)'"
    Write-Output "vcpkg: binary-cache='$($env:VCPKG_DEFAULT_BINARY_CACHE)'"
    if ($env:VCPKG_BINARY_SOURCES) {
        Write-Output "vcpkg: binary-sources='$($env:VCPKG_BINARY_SOURCES)'"
    }
    else {
        Write-Output "vcpkg: binary-sources='<unset>' (using vcpkg default provider)"
    }

    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace VcpkgLock {
    [StructLayout(LayoutKind.Sequential)]
    public struct JOBOBJECT_BASIC_LIMIT_INFORMATION {
        public long PerProcessUserTimeLimit;
        public long PerJobUserTimeLimit;
        public uint LimitFlags;
        public UIntPtr MinimumWorkingSetSize;
        public UIntPtr MaximumWorkingSetSize;
        public uint ActiveProcessLimit;
        public IntPtr Affinity;
        public uint PriorityClass;
        public uint SchedulingClass;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct IO_COUNTERS {
        public ulong ReadOperationCount;
        public ulong WriteOperationCount;
        public ulong OtherOperationCount;
        public ulong ReadTransferCount;
        public ulong WriteTransferCount;
        public ulong OtherTransferCount;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
        public JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
        public IO_COUNTERS IoInfo;
        public UIntPtr ProcessMemoryLimit;
        public UIntPtr JobMemoryLimit;
        public UIntPtr PeakProcessMemoryUsed;
        public UIntPtr PeakJobMemoryUsed;
    }

    public static class Native {
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern IntPtr CreateFileW(string name, uint access, uint share, IntPtr sa, uint disposition, uint flags, IntPtr template);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern int GetFinalPathNameByHandleW(IntPtr handle, StringBuilder path, int cch, int flags);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr handle);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern IntPtr CreateJobObjectW(IntPtr jobAttributes, string name);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool SetInformationJobObject(IntPtr job, int infoClass, IntPtr jobObjectInfo, uint jobObjectInfoLength);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);

        public const int JobObjectExtendedLimitInformation = 9;
        public const uint JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000;
    }
}
'@

    $lockDirs = New-Object 'System.Collections.Generic.SortedSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($path in @($env:VCPKG_DOWNLOADS, $env:X_VCPKG_REGISTRIES_CACHE)) {
        [void]$lockDirs.Add((Resolve-Final $path))
    }
    if ($lockDirs.Count -lt 1 -or $lockDirs.Count -gt 2) {
        throw "vcpkg lock resolver produced $($lockDirs.Count) lock directories (expected 1 or 2)"
    }

    $lockFiles = @($lockDirs | ForEach-Object { [System.IO.Path]::Combine($_, '.vcpkg-install.lock') })
    $maxAttempts = 720
    for ($attempt = 1; $attempt -le $maxAttempts; $attempt++) {
        $handles = New-Object 'System.Collections.Generic.List[System.IDisposable]'
        $contended = $false
        try {
            foreach ($lockFile in $lockFiles) {
                try {
                    $handle = [System.IO.File]::Open($lockFile, [System.IO.FileMode]::Append, [System.IO.FileAccess]::Write, [System.IO.FileShare]::Read)
                    $handles.Add($handle)
                }
                catch [System.IO.IOException] {
                    $errorCode = $_.Exception.HResult -band 0xFFFF
                    if ($errorCode -eq 32 -or $errorCode -eq 33) {
                        $contended = $true
                        break
                    }
                    throw "vcpkg lock file '$lockFile' is not appendable: $($_.Exception.Message)"
                }
            }

            if (-not $contended) {
                $arguments = @(
                    'install',
                    '--vcpkg-root', $env:VCPKG_ROOT,
                    '--downloads-root', $env:VCPKG_DOWNLOADS,
                    '--triplet', $Triplet,
                    '--x-install-root', $InstallRoot,
                    '--x-manifest-root', $ManifestDir,
                    '--x-buildtrees-root', [System.IO.Path]::Combine($InstallRoot, 'buildtrees'),
                    '--x-packages-root', [System.IO.Path]::Combine($InstallRoot, 'packages'),
                    "--overlay-triplets=$overlayTriplets"
                )
                if ([System.IO.Directory]::Exists($overlayPorts)) {
                    $arguments += "--overlay-ports=$overlayPorts"
                }
                if ($MendScan) {
                    $arguments += '--only-downloads'
                    $arguments += '--binarysource=clear'
                    Write-Output "vcpkg: source download mode enabled; extracted sources will remain under '$InstallRoot\buildtrees'"
                    Write-Output 'vcpkg: binary cache disabled for this invocation'
                }

                exit (Invoke-NativeProcessInKillOnCloseJob $vcpkgExe $arguments)
            }
        }
        finally {
            foreach ($handle in $handles) { $handle.Dispose() }
        }

        if ($attempt -eq $maxAttempts) {
            throw "could not acquire vcpkg install lock after $attempt attempts: $($lockFiles -join ', ')"
        }
        Write-Output "vcpkg: waiting for vcpkg install lock(s)... (attempt $attempt of $maxAttempts)"
        Start-Sleep -Seconds 5
    }
}
catch {
    [Console]::Error.WriteLine("Error: $($_.Exception.Message)")
    exit 1
}
