/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SmartIBL.cpp $
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
    unzFile         m_zipFile = nullptr;
    Utf8String      m_iblPath;
    ByteStream      m_iblStream;

    ~SmartIblFile() { if (nullptr != m_zipFile) unzClose (m_zipFile); }
                      
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Open(BeFileNameCR fileName)
    {
    if (nullptr == (m_zipFile = unzOpen64 (Utf8String(fileName.c_str()).c_str())) ||
        SUCCESS != FindIBL () ||
        SUCCESS != ReadCurrentFile(m_iblStream))
        return ERROR;

    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::HDRMapPtr   ReadEnvironmentMap()
    {
    Utf8String      evFileName;
    ByteStream      evFileBytes;

    if (SUCCESS != FindString(evFileName, "EVfile = ") ||
        SUCCESS != LocateFile (evFileName) ||
        SUCCESS != ReadCurrentFile(evFileBytes))
        return nullptr;


    HDRImage  image  = HDRImage::FromHDR(evFileBytes.GetData(), evFileBytes.GetSize());

    if (!image.IsValid())
        return nullptr;

    int             iblMapping;
    double          gamma;
    DPoint2d        offset;

    ReadValue(gamma,        "EVgamma = ", 1.0);
    ReadValue(offset.x,     "EVu = ", 0.0);
    ReadValue(offset.y,     "EVb = ", 0.0);
    ReadValue(iblMapping,   "EVmap = ", 0); 

    return new ImageLight::HDRMap(std::move(image), ConvertIblMapping(iblMapping), offset, gamma, false);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::Mapping    ConvertIblMapping(int iblMapping)
    {
    switch (iblMapping)
        {
        default:
            BeAssert(false && "Invalid ibl mapping");
            // fall through...
        case 1:
            return ImageLight::Mapping::Spherical;

        case 2:
            return ImageLight::Mapping::Cylindrical;

        case 3:
            return ImageLight::Mapping::Angular;

        case 4:
            return ImageLight::Mapping::Rectangular;
        
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LocateFile(Utf8StringCR fileName)
    {
    return  UNZ_OK == unzLocateFile(m_zipFile, (m_iblPath + fileName).c_str(), 2) ? SUCCESS : ERROR;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ReadValue(double& value, Utf8CP label, double defaultValue)
    {
    Utf8String      valueString;

    if (SUCCESS != FindString(valueString, label) || 1 != std::sscanf(valueString.c_str(), "%lf", &value))
        {
        value = defaultValue;
        return ERROR;
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ReadValue(int& value, Utf8CP label, int defaultValue)
    {
    Utf8String      valueString;

    if (SUCCESS != FindString(valueString, label) || 1 != std::sscanf(valueString.c_str(), "%d", &value))
        {
        value = defaultValue;
        return ERROR;
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   FindString(Utf8StringR string, Utf8CP label)
    {
    Utf8CP   pStart, pEnd, pStream = reinterpret_cast<Utf8CP>(m_iblStream.data());

    if (nullptr == (pStart = std::strstr (pStream, label)) ||
        nullptr == (pEnd = std::strchr (pStart, '\n')))
        return ERROR;

    pStart += std::strlen(label);

    string = Utf8String(pStart, pEnd - pStart);

    string.Trim("\"");      // Remove quotes.
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
            {
            size_t      separatorPosition;

            if (std::string::npos != (separatorPosition = nameString.find_last_of('/', nameString.size())))
                m_iblPath = nameString.substr(0, separatorPosition + 1);

            return SUCCESS;
            }
        }
    return ERROR;
    }


};  // SmartIblFile


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::HDRMapPtr ImageLight::DiffuseFromSmartIBL (BeFileNameCR fileName)
    {
    SmartIblFile        iblFile;
    HDRMap              map;

    if (SUCCESS != iblFile.Open(fileName))
        return nullptr;
        
    return iblFile.ReadEnvironmentMap();
    }


    