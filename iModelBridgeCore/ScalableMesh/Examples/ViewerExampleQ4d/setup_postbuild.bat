
SET folder=%~dp0

mkdir "%folder%\out\x64\data"
copy "%folder%\images\*.png" "%folder%\out\x64\data\"
copy "%folder%\config\*.txt" "%folder%\out\x64\"
copy "%folder%\dependencies\bin\x64\*.dll" "%folder%\out\x64\"
copy "%folder%\3smGL\bin\*.dll" "%folder%\out\x64\"

cd %folder%

xcopy "..\Dlls\*.*" "%folder%\out\x64\" /S /Y

