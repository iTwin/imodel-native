/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RenderEnvironment.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>



/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Environment Environment::FromSmartIBL (BeFileNameCR fileName)
    {
    BeFile      iblFile;
    ByteStream  iblBytes;
    Environment environment;

    if (BeFileStatus::Success != iblFile.Open(fileName.c_str(), BeFileAccess::Read) ||
        BeFileStatus::Success != iblFile.ReadEntireFile(iblBytes))
        return environment;

    return environment;
    }
