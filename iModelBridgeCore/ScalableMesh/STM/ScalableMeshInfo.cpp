/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshInfo.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ScalableMeshInfo.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class BingMapLogoRetriever
    {
    private : 

        static BingMapLogoRetriever* s_instance;

        //Bing Map logo that must be displayed for 3SM using BingMap.
        bvector<Byte> m_bingMapLogo;
        DPoint2d      m_bingMapLogoSize;
        WString       m_bingMapLogoURI;
        int32_t       m_bingMapLogoRetryCount;

        BingMapLogoRetriever()
            {
            m_bingMapLogoURI = WString(L"http://dev.virtualearth.net/Branding/logo_powered_by.png");
            m_bingMapLogoRetryCount = 0;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                   Mathieu.St-Pierre  09/2017
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool DownloadBitmapToRgba(bvector<Byte>& imageData, DPoint2d& size, WChar const* pURI, DPoint2d* pRequestedSize)
            {
            WChar localFilename[MAX_PATH];

            if (0 != URLDownloadToCacheFileW(NULL, pURI, localFilename, MAX_PATH, 0, NULL))
                return false;

            ImageUtilities::RgbImageInfo info;
            BeFile pngFile;

            BeFileStatus fileStatus = pngFile.Open(localFilename, BeFileAccess::Read);

            if (fileStatus != BeFileStatus::Success)
                return false;

            BentleyStatus status = ImageUtilities::ReadImageFromPngFile(imageData, info, pngFile);

            if (status != SUCCESS)
                {
                return false;
                }

            size.x = info.width;
            size.y = info.height;

            return true;
            }

    public : 

        static BingMapLogoRetriever* GetInstance()
            {
            if (s_instance == nullptr)
                s_instance = new BingMapLogoRetriever;

            return s_instance;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                   Mathieu.St-Pierre  09/2017
        +---------------+---------------+---------------+---------------+---------------+------*/
        Byte const* GetImageryLogo(DPoint2d& size)
            {
            if (m_bingMapLogoURI.empty() || m_bingMapLogoRetryCount > 2)  // Stop trying after 3 attempts.
                return NULL;

            if (m_bingMapLogo.size() == 0 && !DownloadBitmapToRgba(m_bingMapLogo, m_bingMapLogoSize, m_bingMapLogoURI.c_str(), NULL))
                {
                // An error occurred, we should not try over and over. 
                ++m_bingMapLogoRetryCount;
                return NULL;
                }

            size = m_bingMapLogoSize;
            return &m_bingMapLogo[0];
            }        
    };

BingMapLogoRetriever* BingMapLogoRetriever::s_instance = nullptr;

/*----------------------------------------------------------------------------+
|IScalableMeshTextureInfo Method Definition Section - Begin
+----------------------------------------------------------------------------*/
SMTextureType IScalableMeshTextureInfo::GetTextureType() const
    {
    return _GetTextureType();
    }

bool IScalableMeshTextureInfo::IsTextureAvailable() const
    {
    return _IsTextureAvailable();
    }

bool IScalableMeshTextureInfo::IsUsingBingMap() const
    {
    return _IsUsingBingMap();
    }

const Byte* IScalableMeshTextureInfo::GetBingMapLogo(DPoint2d& bingMapLogoSize)
    {
    return BingMapLogoRetriever::GetInstance()->GetImageryLogo(bingMapLogoSize);
    }
/*----------------------------------------------------------------------------+
|IScalableMeshTextureInfo Method Definition Section - End
+----------------------------------------------------------------------------*/
ScalableMeshTextureInfo::ScalableMeshTextureInfo(SMTextureType textureType, bool isUsingBingMap, bool isTextureAvailable)
    {
    assert(isTextureAvailable || isUsingBingMap);
    m_textureType = textureType;
    m_isUsingBingMap = isUsingBingMap;
    m_isTextureAvailable = isTextureAvailable;
    }

SMTextureType ScalableMeshTextureInfo::_GetTextureType() const
    {
    return m_textureType;
    }

bool ScalableMeshTextureInfo::_IsTextureAvailable() const
    {
    return m_isTextureAvailable;
    }

bool ScalableMeshTextureInfo::_IsUsingBingMap() const
    {
    return m_isUsingBingMap; 
    }        



END_BENTLEY_SCALABLEMESH_NAMESPACE
