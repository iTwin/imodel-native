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
    set "IMODEL_VCPKG_ROOT=%SrcRoot%\vcpkg"
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

set "VCPKG_DOWNLOADS=%VCPKG_ROOT%\downloads"

rem For Android cross-compilation, vcpkg needs ANDROID_NDK_HOME.
rem Our build system sets ANDROID_NDK_ROOT; map it if ANDROID_NDK_HOME is not set.
if "%ANDROID_NDK_HOME%"=="" (
    if not "%ANDROID_NDK_ROOT%"=="" (
        set "ANDROID_NDK_HOME=%ANDROID_NDK_ROOT%"
    )
)

set "OVERLAY_TRIPLETS=%MANIFEST_DIR%\triplets"
set "OVERLAY_ARG="
if exist "%OVERLAY_TRIPLETS%" set "OVERLAY_ARG=--overlay-triplets=%OVERLAY_TRIPLETS%"

echo vcpkg: installing packages from "%MANIFEST_DIR%" (triplet=%TRIPLET%, install-root=%INSTALL_ROOT%)
echo vcpkg: exe="%VCPKG_EXE%"
echo vcpkg: root="%VCPKG_ROOT%"
echo vcpkg: downloads="%VCPKG_DOWNLOADS%"

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
