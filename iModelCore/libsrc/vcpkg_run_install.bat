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

rem Persistent per-platform downloads directory nested under the vcpkg checkout's downloads/
rem folder (which vcpkg's own .gitignore ignores, so these caches never show up as untracked
rem in the vcpkg repo). The binary cache "archives" lives alongside under VCPKG_ROOT. Two goals:
rem   1. Persistence: it lives under SrcRoot (VCPKG_ROOT), NOT OutRoot, so a full clean build
rem      does not wipe it. Tools (cmake, msys2, ...) and source archives are downloaded and
rem      extracted once and reused across clean builds, instead of being re-extracted every
rem      time. Re-extraction is what repeatedly reopens the Windows Defender rename_or_delete
rem      "Access is denied" race on a freshly-extracted cmake.exe.
rem   2. Cross-arch isolation: the platform key is the first two triplet tokens
rem      (e.g. x64-windows, arm64-android, x64-android), so parallel builds of different arches
rem      (notably AndroidARM64 vs AndroidX64) get separate downloads\<platform>\tools trees and
rem      cannot race on tool extraction. Triplet variants of the same platform (static / -md /
rem      -veracode / -clang) and different configs (debug/release) intentionally share one
rem      persistent cache; the sequential install chain (vcpkg.PartFile.xml) serializes
rem      libraries within an arch, and once a tool is extracted no further extraction (hence no
rem      race) occurs for that platform.
rem The binary cache (VCPKG_DEFAULT_BINARY_CACHE) remains shared so compiled packages are not
rem rebuilt redundantly.
for /f "tokens=1,2 delims=-" %%a in ("%TRIPLET%") do set "VCPKG_PLATFORM_KEY=%%a-%%b"
set "VCPKG_DOWNLOADS=%VCPKG_ROOT%\downloads\%VCPKG_PLATFORM_KEY%"
if not exist "%VCPKG_DOWNLOADS%" mkdir "%VCPKG_DOWNLOADS%"
if "%VCPKG_DEFAULT_BINARY_CACHE%"=="" (
    set "VCPKG_DEFAULT_BINARY_CACHE=%VCPKG_ROOT%\archives"
)
if not exist "%VCPKG_DEFAULT_BINARY_CACHE%" mkdir "%VCPKG_DEFAULT_BINARY_CACHE%"

rem Persistent per-platform git registries cache, nested under the same per-platform downloads
rem directory (so it lives under SrcRoot and survives clean builds, and under vcpkg's gitignored
rem downloads/ so it never shows as untracked). Two goals, mirroring the downloads cache:
rem   1. Persistence: because it is not under OutRoot, a clean build does not wipe it, so vcpkg
rem      reuses the existing shallow registry repo and does a small incremental fetch (or none)
rem      instead of a full cold fetch from github.com/microsoft/vcpkg every clean build. That
rem      cold fetch is what intermittently fails with "RPC failed; curl 56 / early EOF".
rem   2. Cross-arch isolation: the <platform> key keeps parallel arch builds (e.g. AndroidARM64
rem      vs AndroidX64) on separate registry repos so their concurrent git fetch/GC operations
rem      cannot collide (the default global cache at %LOCALAPPDATA%\vcpkg\registries is shared
rem      across arches and would race, causing transient "port does not exist" failures).
set "X_VCPKG_REGISTRIES_CACHE=%VCPKG_DOWNLOADS%\registries"
if not exist "%X_VCPKG_REGISTRIES_CACHE%" mkdir "%X_VCPKG_REGISTRIES_CACHE%"

rem Use a persistent local binary cache by default to avoid rebuilding heavy ports
rem (for example, crashpad) across builds. Allow callers to override.
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
echo vcpkg: downloads="%VCPKG_DOWNLOADS%"
echo vcpkg: registries-cache="%X_VCPKG_REGISTRIES_CACHE%"
echo vcpkg: binary-cache="%VCPKG_DEFAULT_BINARY_CACHE%"
echo vcpkg: binary-sources="%VCPKG_BINARY_SOURCES%"

if "%OVERLAY_ARG%"=="" (
    "%VCPKG_EXE%" install --vcpkg-root "%VCPKG_ROOT%" --downloads-root "%VCPKG_DOWNLOADS%" --triplet "%TRIPLET%" --x-install-root "%INSTALL_ROOT%" --x-manifest-root "%MANIFEST_DIR%" --x-buildtrees-root "%INSTALL_ROOT%\buildtrees" --x-packages-root "%INSTALL_ROOT%\packages"
) else (
    "%VCPKG_EXE%" install --vcpkg-root "%VCPKG_ROOT%" --downloads-root "%VCPKG_DOWNLOADS%" --triplet "%TRIPLET%" --x-install-root "%INSTALL_ROOT%" --x-manifest-root "%MANIFEST_DIR%" --x-buildtrees-root "%INSTALL_ROOT%\buildtrees" --x-packages-root "%INSTALL_ROOT%\packages" %OVERLAY_ARG%
)
if errorlevel 1 exit /b %errorlevel%

exit /b 0

:usage
echo Usage: %~nx0 ^<manifest_dir^> ^<install_root^> ^<triplet^>
exit /b 1
