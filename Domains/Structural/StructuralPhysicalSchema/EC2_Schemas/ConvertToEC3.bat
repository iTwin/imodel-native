@echo off

rem *-------- Schema file name constants ----------*
set SCHEMA_NAME=StructuralPhysical

set EC2_SCHEMA_NAME=%SCHEMA_NAME%.01.00.ecschema.xml

set EC3_SCHEMA_NAME_AUTO=%SCHEMA_NAME%.01.00.00.ecschema.xml
set EC3_SCHEMA_NAME_FINAL=%SCHEMA_NAME%.ecschema.xml

rem *-------- Machine-specific locations ----------*
set BSIOUT=D:\sourcetrees\bim0200dev\out

rem *-------- Relative locations ----------*
set DOMAIN_SRC=..

set EC2_SCHEMA_DIR=%DOMAIN_SRC%\EC2_Schemas
set EC3_SCHEMA_DIR=%DOMAIN_SRC%\EC3_Schemas

rem *-------- Schema converter path ----------*
rem To build the EC3 schema converter app:
rem 1) set BUILDSTRATEGY=DgnClientSDK
rem 2) bb -r ecobjects -f ecobjects -p ecobjectstools pull
rem 3) bb -r ecobjects -f ecobjects -p ecobjectstools b
set SCHEMA_CONVERTER=%BSIOUT%\Winx64\Product\ecobjectstools\SchemaConverter.exe

rem -------------------------------------------------------------------------
rem Convert EC2 schema to EC3 schema
rem -------------------------------------------------------------------------
rem %SCHEMA_CONVERTER% -i "%EC2_SCHEMA_DIR%\%EC2_SCHEMA_NAME" -o "%EC3_SCHEMA_DIR%"
%SCHEMA_CONVERTER% -i "%EC2_SCHEMA_DIR%\%EC2_SCHEMA_NAME" -o "%EC3_SCHEMA_DIR%" -x 3 -r .\ -v 01.00

rem -------------------------------------------------------------------------
rem Rename EC3 schema
rem -------------------------------------------------------------------------
cd %EC3_SCHEMA_DIR%

del %EC3_SCHEMA_NAME_FINAL%
rename %EC3_SCHEMA_NAME_AUTO% %EC3_SCHEMA_NAME_FINAL%
