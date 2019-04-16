/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
ImageLight::HDRMapPtr   ReadEnvironmentMap(HDRImage::Encoding encoding)    { return ReadHDRMap ("EV", encoding);  }
ImageLight::HDRMapPtr   ReadReflectionMap(HDRImage::Encoding encoding)     { return ReadHDRMap ("REF", encoding);  }
ImageLight::MapPtr   ReadBackgroundMap()                                   { return ReadJPEGMap ("BG"); }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::HDRMapPtr   ReadHDRMap (Utf8CP prefix, HDRImage::Encoding encoding)
    {
    Utf8String      fileName, preString(prefix);
    ByteStream      fileBytes;

    if (SUCCESS != FindString(fileName, (preString + "file = ").c_str()) ||
        SUCCESS != LocateFile (fileName) ||
        SUCCESS != ReadCurrentFile(fileBytes))
        return nullptr;


    HDRImage  image  = HDRImage::FromHDR(fileBytes.GetData(), fileBytes.GetSize(), encoding);

    if (!image.IsValid())
        return nullptr;

    int             iblMapping;
    double          gamma;
    DPoint2d        offset;

    ReadValue(gamma,        (preString +"gamma = ").c_str(), 1.0);
    ReadValue(offset.x,     (preString +"u = ").c_str(), 0.0);
    ReadValue(offset.y,     (preString +"v = ").c_str(), 0.0);
    ReadValue(iblMapping,   (preString +"map = ").c_str(), 0); 

    return new ImageLight::HDRMap(std::move(image), ConvertIblMapping(iblMapping), offset, gamma, false);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::MapPtr   ReadJPEGMap (Utf8CP prefix)
    {
    Utf8String      fileName, preString(prefix);
    ByteStream      fileBytes;


    if (SUCCESS != FindString(fileName, (preString + "file = ").c_str()) ||
        SUCCESS != LocateFile (fileName) ||
        SUCCESS != ReadCurrentFile(fileBytes))
        return nullptr;

    ImageSource     imageSource(ImageSource::Format::Jpeg, std::move(fileBytes));
    Image           image(imageSource, Image::Format::Rgb);

    if (!image.IsValid())
        return nullptr;

    int             iblMapping;
    DPoint2d        offset;

    ReadValue(offset.x,     (preString +"u = ").c_str(), 0.0);
    ReadValue(offset.y,     (preString +"b = ").c_str(), 0.0);
    ReadValue(iblMapping,   (preString +"map = ").c_str(), 0); 

    return new ImageLight::Map(std::move(image), ConvertIblMapping(iblMapping), offset, 1.0, false);
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

    bvector<uint8_t>    buffer(1024 * 128);
    int                 nRead;

    while (0 != (nRead = unzReadCurrentFile(m_zipFile, buffer.data(), buffer.size())))
        byteStream.Append(buffer.data(), nRead);

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
ImageLight::HDRMapPtr ImageLight::DiffuseFromSmartIBL (BeFileNameCR fileName, HDRImage::Encoding encoding)
    {
    SmartIblFile        iblFile;
    HDRMap              map;

    if (SUCCESS != iblFile.Open(fileName))
        return nullptr;
        
    return iblFile.ReadEnvironmentMap(encoding);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::HDRMapPtr ImageLight::ReflectionFromSmartIBL (BeFileNameCR fileName, HDRImage::Encoding encoding)
    {
    SmartIblFile        iblFile;
    HDRMap              map;

    if (SUCCESS != iblFile.Open(fileName))
        return nullptr;
        
    return iblFile.ReadReflectionMap(encoding);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ImageLight::MapPtr ImageLight::BackgroundFromSmartIBL (BeFileNameCR fileName)
    {
    SmartIblFile        iblFile;
    HDRMap              map;

    if (SUCCESS != iblFile.Open(fileName))
        return nullptr;
        
    return iblFile.ReadBackgroundMap();
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ImageLight::SolarFromSmartIBL(ImageLight::Solar& solar, BeFileNameCR fileName)
    {
    SmartIblFile        iblFile;

    if (SUCCESS != iblFile.Open(fileName))
        return ERROR;

    DVec2d      uv;

    if (SUCCESS != iblFile.ReadValue(uv.x, "SUNu = ", 0.0) ||
        SUCCESS != iblFile.ReadValue(uv.y, "SUNv = ", 0.0))
        {
        return ERROR;
        }

    iblFile.ReadValue (solar.m_intensity, "SUNmulti = ", 1.0);

    double      theta = msGeomConst_2pi * (.25 - uv.x);
    double      alpha = msGeomConst_pi * (.5 - uv.y);
    double      radius = cos(alpha);

    solar.m_direction = DVec3d::From (cos(theta) * radius, sin(theta) * radius, sin(alpha));

    return SUCCESS;
    }
    

    
