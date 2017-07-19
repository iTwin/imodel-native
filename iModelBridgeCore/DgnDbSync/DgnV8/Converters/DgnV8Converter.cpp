/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Converters/DgnV8Converter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/ConverterApp.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    RootModelConverterApp app;
    return app.Run(argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey)
    {
    // Note that bridgeRegSubKey may not be "DgnV8Bridge". The ABD bridge, for example, reuses the v8 bridge code and customizes it at run time. The ABD bridge uses its own
    //  registry subkey. We have no way currently to check if brigeRegSubKey is reasonable. We'll just have to trust that the fwk knows what it's doing.
    return new RootModelConverterApp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge_getAffinity(iModelBridge::BridgeAffinity& bridgeAffinity, BentleyApi::BeFileName const& thisLibraryPath, BentleyApi::BeFileName const& sourceFileName)
    {
    if (BSISUCCESS == Converter::CheckCanOpenFile(sourceFileName, thisLibraryPath))
        {
        bridgeAffinity.m_affinity = BentleyApi::Dgn::iModelBridge::Affinity::Low;
        bridgeAffinity.m_bridgeRegSubKey = L"DgnV8Bridge";
        }

#ifdef COMMENT_OUT_EXAMPLE_CODE
    // *** The following is just an example of how iModelBridge_getAffinity could support more than one bridge.
    if (sourceFileName.EndsWith(L".csv"))
        {
        bridgeAffinity.m_affinity = BentleyApi::Dgn::iModelBridge::Affinity::Low;
        bridgeAffinity.m_bridgeRegSubKey = L"MythicalCsvBridge";
        return;
        }
#endif
    }
