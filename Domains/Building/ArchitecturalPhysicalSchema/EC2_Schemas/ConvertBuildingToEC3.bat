ECHO Off
REM 
REM This batch file will attempt to convert all the OpenPlant Function BIS schema in this directory to EC3 format and write the output to the "Plant_BIS_01_00" directory.
REM If a file with the same name exists in the "Plant_BIS_01_00" directory it will be overwritten without warning!
ECHO On

set OPBIMClientSrc=D:\Builds\OPBimClientTest\

%OPBIMClientSrc%Tests\OPSchemaConverter.exe -i ArchitecturalPhysical.01.00.ecschema.xml -o ..\\EC3_Schemas -r e: -x 3 -r .\ -v 01.00
REM %OPBIMClientSrc%\Winx64\Product\OPBimClientTest\Tests\OPSchemaConverter.exe -i BuildingCommon.01.00.ecschema.xml -o ..\\EC3_Schemas -r e: -x 3 -r .\ -v 01.00
REM %OPBIMClientSrc%\Winx64\Product\OPBimClientTest\Tests\OPSchemaConverter.exe -i IfcRevitDomain2x3.01.01.ecschema.xml -o ..\\EC3_Schemas -r e: -x 3 -r .\ -v 01.00

cd ..\EC3_Schemas

del ArchitecturalPhysical.ecschema.xml
rename   ArchitecturalPhysical.01.00.00.ecschema.xml  ArchitecturalPhysical.ecschema.xml



