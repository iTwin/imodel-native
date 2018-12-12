/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshInfo.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_IMAGEPP
#include "ScalableMeshInfo.h"
#include "RasterUtilities.h"


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
#ifndef LINUX_SCALABLEMESH_BUILD
            WChar localFilename[MAX_PATH];

            if (0 != URLDownloadToCacheFileW(NULL, pURI, localFilename, MAX_PATH, 0, NULL))
                return false;

            DRange2d extentInTargetCS(DRange2d::NullRange());
            GCSCPTR targetCS(nullptr);
            HFCPtr<HRFRasterFile> pRasterFile;

            WString localFileUrl(L"file://");
            localFileUrl += WString(localFilename);

            HFCPtr<HRARASTER> pRaster(RasterUtilities::LoadRaster(pRasterFile, localFileUrl, targetCS, extentInTargetCS));

            HRFResolutionEditor* pResEditor(pRasterFile->CreateResolutionEditor(0, 0, HFC_SHARE_READ_ONLY));

            size.x = pResEditor->GetResolutionDescriptor()->GetWidth();
            size.y = pResEditor->GetResolutionDescriptor()->GetHeight();

            DRange2d area(DRange2d::From(0, 0, size.x, -size.y));

            StatusInt status = RasterUtilities::CopyFromArea(imageData, size.x, size.y, area, nullptr, *pRaster, true, false);

            if (status == SUCCESS)
                return true;
#endif
            return false;               
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
WString IScalableMeshTextureInfo::GetBingMapsType() const
    {   
    return _GetBingMapsType();
    }

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
ScalableMeshTextureInfo::ScalableMeshTextureInfo(SMTextureType textureType, bool isUsingBingMap, bool isTextureAvailable, const WString& bingMapType)
    {
    assert(isTextureAvailable || isUsingBingMap);
    m_textureType = textureType;
    m_isUsingBingMap = isUsingBingMap;
    m_isTextureAvailable = isTextureAvailable;
    m_bingMapType = bingMapType;
    }

WString ScalableMeshTextureInfo::_GetBingMapsType() const
    {
    assert(IsUsingBingMap());

    return m_bingMapType;
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
