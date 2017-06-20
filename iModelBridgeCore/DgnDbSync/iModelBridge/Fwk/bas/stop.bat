set OFService="Bentley Orchestration Shepherd"

net stop %OFService%

taskkill /f /im pas*
taskkill /f /im ustationwrapper.exe

taskkill /f /im orchadmin.exe
taskkill /f /im ecoconfig.exe

taskkill /f /im python.exe
