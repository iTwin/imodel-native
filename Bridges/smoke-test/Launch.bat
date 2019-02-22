@echo off



call Deletebuild.bat

python DownloadBridge.py -s \\isbprdfs02\ITG\Builds\AzureBuilds\iModelBridgeService-MicroStation -d C:\bridge\build -b 2.0 -f iModelBridgeMstnx64.msi


TIMEOUT 20

call Delete.bat

call install.bat


python CheckServerBuild.py C:\bridge\build\ 2. C:\bridge\

python login.py

call bridgenew.bat

python ErrorFind.py

python BridgeEmail.py 




