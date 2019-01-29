rem generate COD-format import lists for all .mclass files under the current directory.
del temp.out
del temp1.out

rem javaload generates complete data, including inner classes
for /r %%i in (*.mclass) do javaload -imports -unmangle %%i >> temp.out

rem strip out inner classes, and collapse system references.
gema -f %GEMADIR%\g\topclasses.g temp.out temp1.out

