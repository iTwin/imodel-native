set OFService="Bentley Orchestration Shepherd"

set asExtDir="D:\BASWorkDir\ext\"

call stop.bat

copy %OutRoot%\Winx86\Product\DgnV8MirrorICSPlugin\DgnV8MirrorICSPlugin.dll %asExtDir%

net start %OFService%

"C:\Program Files (x86)\Bentley\OrchestrationFramework\orchadmin.exe"
