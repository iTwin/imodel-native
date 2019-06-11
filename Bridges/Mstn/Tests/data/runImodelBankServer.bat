node %MSTN_BRIDGE_TESTS_IMODEL_BANK_SERVER_JS% --handleDumpEndpoints --verbose  %1   %~dp0\server.config.json    %~dp0\logging.config.json
IF '%ERRORLEVEL%'=='0' GOTO OK
    ECHO ************** ERROR %ERRORLEVEL% ****************
:OK