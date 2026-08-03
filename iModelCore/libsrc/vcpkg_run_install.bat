@echo off
rem ---------------------------------------------------------------------------------------------
rem  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem  See LICENSE.md in the repository root for full copyright notice.
rem ---------------------------------------------------------------------------------------------
rem Wrapper script for vcpkg install, invoked from .mke build files.
rem Customize IMODEL_VCPKG_ROOT for developer or CI environments.
rem
rem Usage: vcpkg_run_install.bat <manifest_dir> <install_root> <triplet>
rem   manifest_dir: Directory containing vcpkg.json
rem   install_root: Where vcpkg_installed/<triplet> output goes (e.g., $OutRoot/vcpkg)
rem   triplet:      vcpkg triplet (e.g., x64-windows-static)
rem ---------------------------------------------------------------------------------------------

echo vcpkg_run_install.bat %*

setlocal EnableExtensions

set "MANIFEST_DIR=%~1"
set "INSTALL_ROOT=%~2"
set "TRIPLET=%~3"

if "%MANIFEST_DIR%"=="" goto :usage
if "%INSTALL_ROOT%"=="" goto :usage
if "%TRIPLET%"=="" goto :usage

rem Trim trailing slashes to avoid Windows quoted-argument parsing issues
rem when a path ends with "\" (can consume the closing quote).
:trim_manifest_dir
if "%MANIFEST_DIR:~-1%"=="\" (
    set "MANIFEST_DIR=%MANIFEST_DIR:~0,-1%"
    goto :trim_manifest_dir
)
if "%MANIFEST_DIR:~-1%"=="/" (
    set "MANIFEST_DIR=%MANIFEST_DIR:~0,-1%"
    goto :trim_manifest_dir
)

:trim_install_root
if "%INSTALL_ROOT:~-1%"=="\" (
    set "INSTALL_ROOT=%INSTALL_ROOT:~0,-1%"
    goto :trim_install_root
)
if "%INSTALL_ROOT:~-1%"=="/" (
    set "INSTALL_ROOT=%INSTALL_ROOT:~0,-1%"
    goto :trim_install_root
)

:trim_vcpkg_root
if not "%VCPKG_ROOT%"=="" (
    if "%VCPKG_ROOT:~-1%"=="\" (
        set "VCPKG_ROOT=%VCPKG_ROOT:~0,-1%"
        goto :trim_vcpkg_root
    )
    if "%VCPKG_ROOT:~-1%"=="/" (
        set "VCPKG_ROOT=%VCPKG_ROOT:~0,-1%"
        goto :trim_vcpkg_root
    )
)

if "%IMODEL_VCPKG_ROOT%"=="" (
    set "IMODEL_VCPKG_ROOT=%SrcRoot%vcpkg"
)

:trim_imodel_vcpkg_root
if not "%IMODEL_VCPKG_ROOT%"=="" (
    if "%IMODEL_VCPKG_ROOT:~-1%"=="\" (
        set "IMODEL_VCPKG_ROOT=%IMODEL_VCPKG_ROOT:~0,-1%"
        goto :trim_imodel_vcpkg_root
    )
    if "%IMODEL_VCPKG_ROOT:~-1%"=="/" (
        set "IMODEL_VCPKG_ROOT=%IMODEL_VCPKG_ROOT:~0,-1%"
        goto :trim_imodel_vcpkg_root
    )
)

set "VS_VCPKG_ROOT="
if not "%VCINSTALLDIR%"=="" (
    set "VS_VCPKG_ROOT=%VCINSTALLDIR%\vcpkg"
)

:trim_vs_vcpkg_root
if not "%VS_VCPKG_ROOT%"=="" (
    if "%VS_VCPKG_ROOT:~-1%"=="\" (
        set "VS_VCPKG_ROOT=%VS_VCPKG_ROOT:~0,-1%"
        goto :trim_vs_vcpkg_root
    )
    if "%VS_VCPKG_ROOT:~-1%"=="/" (
        set "VS_VCPKG_ROOT=%VS_VCPKG_ROOT:~0,-1%"
        goto :trim_vs_vcpkg_root
    )
)

rem Locate vcpkg. Prefer a standalone checkout over the Visual Studio bundled
rem copy, even if vcvarsall injected VCPKG_ROOT to the bundled location.
set "VCPKG_EXE="

if not "%IMODEL_VCPKG_ROOT%"=="" (
    if exist "%IMODEL_VCPKG_ROOT%\vcpkg.exe" (
        set "VCPKG_ROOT=%IMODEL_VCPKG_ROOT%"
        set "VCPKG_EXE=%IMODEL_VCPKG_ROOT%\vcpkg.exe"
    )
)

if not "%VCPKG_ROOT%"=="" (
    if "%VCPKG_EXE%"=="" (
        if /I not "%VCPKG_ROOT%"=="%VS_VCPKG_ROOT%" (
            if exist "%VCPKG_ROOT%\vcpkg.exe" set "VCPKG_EXE=%VCPKG_ROOT%\vcpkg.exe"
        )
    )
)

if "%VCPKG_EXE%"=="" (
    if exist "D:\src\vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=D:\src\vcpkg"
        set "VCPKG_EXE=D:\src\vcpkg\vcpkg.exe"
    )
)

if "%VCPKG_EXE%"=="" (
    if exist "%USERPROFILE%\src\vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=%USERPROFILE%\src\vcpkg"
        set "VCPKG_EXE=%USERPROFILE%\src\vcpkg\vcpkg.exe"
    )
)

if "%VCPKG_EXE%"=="" (
    if exist "%VCINSTALLDIR%\vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=%VCINSTALLDIR%\vcpkg"
        set "VCPKG_EXE=%VCINSTALLDIR%\vcpkg\vcpkg.exe"
    )
)

if not exist "%VCPKG_EXE%" (
    echo Error: vcpkg not found at "%VCPKG_EXE%"
    echo Set VCPKG_ROOT to the standalone vcpkg installation directory.
    exit /b 1
)

rem --------------------------------------------------------------------------------------
rem Persistent per-user vcpkg cache base.
rem
rem The downloads/tools tree, the registries git repo, and the binary "archives" cache all live
rem under this base. It is placed under the user profile (LOCALAPPDATA) rather than under
rem VCPKG_ROOT for two reasons:
rem   * Writability: VCPKG_ROOT may resolve to a protected location -- e.g. the Visual Studio
rem     bundled copy under %VCINSTALLDIR%\vcpkg (Program Files) or a shared, read-only
rem     IMODEL_VCPKG_ROOT -- which we must not require to be writable.
rem   * Persistence: LOCALAPPDATA is not under OutRoot, so a clean build does not wipe it. Tools
rem     (cmake, msys2, ...), source archives, and the shallow registry repo are downloaded and
rem     extracted once and reused across clean builds instead of being re-extracted every time
rem     (re-extraction is slow and repeatedly reopens the Windows Defender rename_or_delete
rem     "Access is denied" race on a freshly-extracted cmake.exe).
rem If the profile location is unavailable (LOCALAPPDATA unset or not creatable, e.g. a
rem locked-down agent), fall back to a directory under INSTALL_ROOT so the build still works.
rem --------------------------------------------------------------------------------------
set "VCPKG_CACHE_BASE=%LOCALAPPDATA%\Bentley\vcpkg"
if "%LOCALAPPDATA%"=="" set "VCPKG_CACHE_BASE=%INSTALL_ROOT%\vcpkg-cache"
if not exist "%VCPKG_CACHE_BASE%" mkdir "%VCPKG_CACHE_BASE%" 2>nul
if not exist "%VCPKG_CACHE_BASE%" (
    echo vcpkg: persistent cache base "%VCPKG_CACHE_BASE%" unavailable; falling back to INSTALL_ROOT
    set "VCPKG_CACHE_BASE=%INSTALL_ROOT%\vcpkg-cache"
    if not exist "%VCPKG_CACHE_BASE%" mkdir "%VCPKG_CACHE_BASE%"
)

rem Per-platform downloads/tools tree and registries git repo. The platform key is the first two
rem triplet tokens (e.g. x64-windows, arm64-android, x64-android), so parallel builds of
rem different arches (notably AndroidARM64 vs AndroidX64) get separate trees and cannot race on
rem tool extraction or registry git fetch/GC. Triplet variants of the same platform (static /
rem -md / -veracode / -clang) and configs (debug/release) intentionally share one persistent
rem cache; the cross-process lock around the install below serializes same-platform runs.
rem Honor the caller-provided VCPKG_DOWNLOADS / X_VCPKG_REGISTRIES_CACHE / VCPKG_DEFAULT_BINARY_CACHE
rem escape hatches when set; otherwise derive them from the cache base.
for /f "tokens=1,2 delims=-" %%a in ("%TRIPLET%") do set "VCPKG_PLATFORM_KEY=%%a-%%b"

if "%VCPKG_DOWNLOADS%"=="" set "VCPKG_DOWNLOADS=%VCPKG_CACHE_BASE%\downloads\%VCPKG_PLATFORM_KEY%"
if not exist "%VCPKG_DOWNLOADS%" mkdir "%VCPKG_DOWNLOADS%" 2>nul

if "%X_VCPKG_REGISTRIES_CACHE%"=="" set "X_VCPKG_REGISTRIES_CACHE=%VCPKG_DOWNLOADS%\registries"
if not exist "%X_VCPKG_REGISTRIES_CACHE%" mkdir "%X_VCPKG_REGISTRIES_CACHE%" 2>nul

rem Use a persistent local binary cache by default to avoid rebuilding heavy ports (for example,
rem crashpad) across builds. Shared across arches/configs (vcpkg keeps it concurrency-safe).
if "%VCPKG_DEFAULT_BINARY_CACHE%"=="" set "VCPKG_DEFAULT_BINARY_CACHE=%VCPKG_CACHE_BASE%\archives"
if not exist "%VCPKG_DEFAULT_BINARY_CACHE%" mkdir "%VCPKG_DEFAULT_BINARY_CACHE%" 2>nul
if "%VCPKG_BINARY_SOURCES%"=="" (
    set "VCPKG_BINARY_SOURCES=clear;files,%VCPKG_DEFAULT_BINARY_CACHE%,readwrite"
)

rem For Android cross-compilation, vcpkg needs ANDROID_NDK_HOME.
rem Our build system sets ANDROID_NDK_ROOT; map it if ANDROID_NDK_HOME is not set.
if "%ANDROID_NDK_HOME%"=="" (
    if not "%ANDROID_NDK_ROOT%"=="" (
        set "ANDROID_NDK_HOME=%ANDROID_NDK_ROOT%"
    )
)

set "OVERLAY_TRIPLETS=%MANIFEST_DIR%\triplets"
set "OVERLAY_TRIPLET_FILE=%OVERLAY_TRIPLETS%\%TRIPLET%.cmake"
rem Require a repo-provided overlay triplet for the requested triplet. Every supported build must
rem use one of our custom triplet files: they carry CACHE_BUST markers and build flags that feed
rem vcpkg's ABI hash, so falling back to vcpkg's built-in triplets would silently produce binaries
rem with a different ABI and defeat the cache-busting scheme. Error out instead of using a default.
if not exist "%OVERLAY_TRIPLET_FILE%" (
    echo Error: no custom overlay triplet "%TRIPLET%" found at "%OVERLAY_TRIPLET_FILE%"
    echo This build requires a repo-provided triplet; vcpkg's built-in triplets must not be used.
    exit /b 1
)
rem Quote the path value: a manifest/checkout path may contain spaces, and %OVERLAY_ARG%
rem is expanded unquoted on the vcpkg command line, so cmd would otherwise split it.
set OVERLAY_ARG=--overlay-triplets="%OVERLAY_TRIPLETS%"

rem Allow a manifest to ship overlay ports (e.g. a locally-patched crashpad that
rem builds with clang-cl) in a "ports" subdirectory alongside vcpkg.json.
set "OVERLAY_PORTS=%MANIFEST_DIR%\ports"
if exist "%OVERLAY_PORTS%" set OVERLAY_ARG=%OVERLAY_ARG% --overlay-ports="%OVERLAY_PORTS%"

echo vcpkg: installing packages from "%MANIFEST_DIR%" (triplet=%TRIPLET%, install-root=%INSTALL_ROOT%)
echo vcpkg: exe="%VCPKG_EXE%"
echo vcpkg: root="%VCPKG_ROOT%"
echo vcpkg: cache-base="%VCPKG_CACHE_BASE%"
echo vcpkg: downloads="%VCPKG_DOWNLOADS%"
echo vcpkg: registries-cache="%X_VCPKG_REGISTRIES_CACHE%"
echo vcpkg: binary-cache="%VCPKG_DEFAULT_BINARY_CACHE%"
echo vcpkg: binary-sources="%VCPKG_BINARY_SOURCES%"

rem --------------------------------------------------------------------------------------
rem Cross-process lock around the install. Same-platform builds share the per-platform
rem downloads/tools tree and registries git repo above; vcpkg.PartFile.xml only serializes
rem consumers within a single BentleyBuild graph, so two same-platform builds (static + dynamic,
rem or two pipelines) can still run concurrently and corrupt that shared state (see PR #1497:
rem concurrent registry fetch/GC). Serialize them with a platform-keyed lock file: cmd opens the
rem redirection target (handle 9) without write-sharing, so a second process's open fails while
rem the first holds it. Different arches use different lock files and still build in parallel.
rem The OS releases the handle when this process exits, so a crash cannot wedge the lock.
rem Handle 8 preserves the real stderr so vcpkg's own error output is not swallowed by the 2>nul
rem that suppresses the expected "file in use" noise from a failed lock acquisition.
rem --------------------------------------------------------------------------------------
set "VCPKG_LOCKFILE=%VCPKG_CACHE_BASE%\%VCPKG_PLATFORM_KEY%.install.lock"

:acquire_lock
set "GOT_LOCK="
(
    9>>"%VCPKG_LOCKFILE%" (
        set "GOT_LOCK=1"
        call :run_install
    )
) 8>&2 2>nul
if defined GOT_LOCK goto :lock_done
echo vcpkg: waiting for same-platform install lock "%VCPKG_LOCKFILE%"...
rem ~5s portable sleep (timeout /t breaks when stdin is redirected in CI).
ping -n 6 127.0.0.1 >nul
goto :acquire_lock

:lock_done
if not "%INSTALL_RC%"=="0" exit /b %INSTALL_RC%
exit /b 0

:run_install
"%VCPKG_EXE%" install --vcpkg-root "%VCPKG_ROOT%" --downloads-root "%VCPKG_DOWNLOADS%" --triplet "%TRIPLET%" --x-install-root "%INSTALL_ROOT%" --x-manifest-root "%MANIFEST_DIR%" --x-buildtrees-root "%INSTALL_ROOT%\buildtrees" --x-packages-root "%INSTALL_ROOT%\packages" %OVERLAY_ARG% 2>&8
set "INSTALL_RC=%errorlevel%"
goto :eof

:usage
echo Usage: %~nx0 ^<manifest_dir^> ^<install_root^> ^<triplet^>
exit /b 1
