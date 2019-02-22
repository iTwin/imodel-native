echo deleting file

del /q "C:\bridge\build\*"
FOR /D %%p IN ("C:\bridge\build\*.*") DO rmdir "%%p" /s /q

echo Done!