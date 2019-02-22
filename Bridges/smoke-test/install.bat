set /p version=<C:\bridge\Version.log
msiexec /a C:\bridge\build\%version%\iModelBridgeMstnx64.msi /qb TARGETDIR=C:\bridge\BridgeInstallationDirectory
