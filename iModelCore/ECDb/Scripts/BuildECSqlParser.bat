@ECHO off
REM Copyright (c) Bentley Systems, Incorporated. All rights reserved.
REM See LICENSE.md in the repository root for full copyright notice.
@ECHO on
REM Attempt to download win_flex_bison tools and extract them if they do not already exist.
REM Once the tool are avaliable the script build besqlite
set win_flex_ver=2.5.24
set bin_root=%appdata%\winflexbison
set zip_file=%temp%\win_flex_bison.zip
set flex_tool=%bin_root%\win_flex.exe
set bison_tool=%bin_root%\win_bison.exe
set self_path=%~dp0
set parse_dir=%self_path%\..\ECDb\ECSql\parser
set zip_download_url=https://github.com/lexxmark/winflexbison/releases/download/v%win_flex_ver%/win_flex_bison-%win_flex_ver%.zip
if not exist %flex_tool% goto download_and_unzip

goto build

:download_and_unzip
md %bin_root%
curl -L "%zip_download_url%" --output %zip_file%
set vbs="%temp%\_.vbs"
if exist %vbs% del /f /q %vbs%
>>%vbs% echo Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% echo If NOT fso.FolderExists("%bin_root%") Then
>>%vbs% echo fso.CreateFolder("%bin_root%")
>>%vbs% echo End If
>>%vbs% echo set objShell = CreateObject("Shell.Application")
>>%vbs% echo set FilesInZip=objShell.NameSpace("%zip_file%").items
>>%vbs% echo objShell.NameSpace("%bin_root%").CopyHere(FilesInZip)
>>%vbs% echo Set fso = Nothing
>>%vbs% echo Set objShell = Nothing
cscript //nologo %vbs%


:build
ECHO Compiling Lexical Analyzer ...
%flex_tool% -i -8 -PSQLyy -L -o%parse_dir%\SqlFlex.cpp %parse_dir%\SQLflex.l

ECHO Compiling Parser ...
%bison_tool% -k -l -pSQLyy -bSql -o %parse_dir%\SqlBison.cpp --defines=%parse_dir%\SqlBison.h %parse_dir%\SQlbison.y

