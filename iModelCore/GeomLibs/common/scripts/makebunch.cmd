
@echo off
rem create (caller, callee) list in a file, using all objects in the current dir as data.
del fileToFunc.list
del funcToFile.g
for %%i in (*.obj) do call symbolsfromobjectfile %%i symboldefs >> funcToFile.g
for %%i in (*.obj) do call symbolsfromobjectfile %%i symbolrefs >> fileToFunc.list
gema -f funcToFile.g fileToFunc.list |sort | uniq > fileToFile.list
