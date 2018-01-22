/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RenderEnvironment.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#include <zlib/zip/unzip.h>



//=======================================================================================
// @bsiclass                                            Ray.Bentley         01/2018
//=======================================================================================
struct SmartIblFile
{
    unzFile     m_zipFile = nullptr;

    ~SmartIblFile() { if (nullptr != m_zipFile) unzClose (m_zipFile); }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Open(BeFileNameCR fileName)
    {
    ByteStream      iblStream;

    if (nullptr == (m_zipFile = unzOpen64 (Utf8String(fileName.c_str()).c_str())) ||
        SUCCESS != FindIBL () ||
        SUCCESS != ReadCurrentFile(iblStream))
        return ERROR;
    
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ReadCurrentFile(ByteStream& byteStream)
    {
    if (UNZ_OK != unzOpenCurrentFile(m_zipFile))
        return ERROR;

    uint8_t     buffer[4096];
    int         nRead;

    while (0 != (nRead = unzReadCurrentFile(m_zipFile, buffer, sizeof(buffer))))
        byteStream.Append(buffer, nRead);

    unzCloseCurrentFile(m_zipFile);
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FindIBL ()
    {
    for (int status = unzGoToFirstFile(m_zipFile); UNZ_OK == status; status = unzGoToNextFile(m_zipFile))
        {
        Utf8Char    zipName[MAXFILELENGTH];

        if (UNZ_OK != unzGetCurrentFileInfo64(m_zipFile, nullptr, zipName, sizeof(zipName), nullptr, 0, nullptr, 0))
            {
            BeAssert(false);
            continue;
            }

        Utf8String      nameString(zipName);
        if (nameString.EndsWith(".ibl"))
            return SUCCESS;
        }
    return ERROR;
    }


};  // SmartIblFile



/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Environment Environment::FromSmartIBL (BeFileNameCR fileName)
    {
    Environment         environment;
    SmartIblFile        iblFile;

    if (SUCCESS != iblFile.Open(fileName))
        return environment;

    return environment;
    }



    