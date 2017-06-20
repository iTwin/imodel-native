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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
extern "C" EXPORT_ATTRIBUTE iModelBridge* iModelBridge_getInstance()
    {
    return new RootModelConverterApp;
    }
