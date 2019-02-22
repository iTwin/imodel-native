echo deleting file

del /q "C:\bridge\BridgeInstallationDirectory\*"
FOR /D %%p IN ("C:\bridge\BridgeInstallationDirectory\*.*") DO rmdir "%%p" /s /q


echo Done!