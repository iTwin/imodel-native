ECHO Off
REM 
REM This batch file will attempt to convert all the OpenPlant Function BIS schema in this directory to EC3 format and write the output to the "Plant_BIS_01_00" directory.
REM If a file with the same name exists in the "Plant_BIS_01_00" directory it will be overwritten without warning!
ECHO On

set OPBIMClientSrc=D:\Builds\SourceTrees\opdgndb01-0\out\

%OPBIMClientSrc%\Winx64\Product\OPBimClientTest\Tests\OPSchemaConverter.exe -i StructuralMaterials.01.00.ecschema.xml -o ..\\EC3_Schemas -r e: -x 3 -r .\ -v 01.00

cd ..\EC3_Schemas

del StructuralMaterials.ecschema.xml
rename   StructuralMaterials.01.00.00.ecschema.xml  StructuralMaterials.ecschema.xml



