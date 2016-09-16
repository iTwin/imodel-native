/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/PointCloudDataHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PointCloudVortex.h"

#include <atlimage.h>

#include <RealityPlatform/RealityDataHandler.h>
#include <RealityPlatform/RealityPlatformUtil.h>

#define THUMBNAIL_WIDTH     256
#define THUMBNAIL_HEIGHT    256

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

enum WktFlavor
    {
    WktFlavor_Oracle9 = 1,
    WktFlavor_Autodesk,
    WktFlavor_OGC,
    WktFlavor_End,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MapWktFlavorEnum(GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, WktFlavor wktFlavor)
    {
    //Temporary use numeric value until the basegcs enum match the csmap's one.
    switch (wktFlavor)
        {
            case WktFlavor::WktFlavor_Oracle9:
                baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOracle9;
                break;

            case WktFlavor::WktFlavor_Autodesk:
                baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorAutodesk;
                break;

            case WktFlavor::WktFlavor_OGC:
                baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOGC;
                break;

            default:
                return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS::WktFlavor GetWKTFlavor(WString* wktStrWithoutFlavor, const WString& wktStr)
    {
    WktFlavor wktFlavor = WktFlavor::WktFlavor_Oracle9;

    size_t charInd = wktStr.size() - 1;

    for (charInd = wktStr.size() - 1; charInd >= 0; charInd--)
        {
        if (wktStr[charInd] == L']')
            {
            break;
            }
        else
            if (((short) wktStr[charInd] >= 1) || ((short) wktStr[charInd] < WktFlavor::WktFlavor_End))
                {
                wktFlavor = (WktFlavor) wktStr[charInd];
                }
        }

    if (wktStrWithoutFlavor != 0)
        {
        *wktStrWithoutFlavor = wktStr.substr(0, charInd + 1);
        }

    GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

    bool result = MapWktFlavorEnum(baseGcsWktFlavor, wktFlavor);

    assert(result == true);

    return baseGcsWktFlavor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudData::PointCloudData(Utf8CP inFilename, PointCloudView view)
    {
    //&&JFC TODO: Need to do this only once per session and out of the properties object.
    Initialize();

    m_view = view;

    GetFile(inFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudData::~PointCloudData()
    {
    //CloseFile();
    Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudData::Initialize()
    {
    PointCloudVortex::Initialize();

    if (!SessionManager::InitBaseGCS())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudData::Terminate()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataPtr PointCloudData::Create(Utf8CP inFilename, PointCloudView pcView)
    {
    return new PointCloudData(inFilename, pcView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudData::GetFile(Utf8CP inFilename)
    {
    WString filename;
    BeStringUtilities::Utf8ToWChar(filename, inFilename);

    PointCloudVortex::OpenPOD(m_cloudFileHandle, m_cloudHandle, filename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudData::CloseFile()
    {
    PointCloudVortex::ClosePOD(m_cloudFileHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudData::_GetFootprint(DRange2dP pFootprint) const
    {
    return ExtractFootprint(pFootprint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudData::ExtractFootprint(DRange2dP pFootprint) const
    {
    if (NULL == m_cloudFileHandle || NULL == m_cloudHandle)
        return 0;

    // Get bounds for first point cloud in the scene
    double   lower[3], upper[3];
    PointCloudVortex::GetPointCloudBounds(m_cloudHandle, lower, upper);

    PtHandle metadataHandle = PointCloudVortex::GetMetadataHandle(m_cloudHandle);
    if (0 == metadataHandle)
        return 0;

    wchar_t buffer[1024] = L"";

    if (!PointCloudVortex::GetMetaTag(metadataHandle, L"Survey.GeoReference", buffer))
        return 0;

    WString podWkt(buffer);
    if (podWkt.empty())
        return 0;

    // SrcGCS
    StatusInt warning;
    WString warningErrorMsg;

    WString wktWithoutFlavor;
    GeoCoordinates::BaseGCS::WktFlavor wktFlavor = GetWKTFlavor(&wktWithoutFlavor, podWkt);

    GeoCoordinates::BaseGCSPtr pSrcGcs = GeoCoordinates::BaseGCS::CreateGCS();
    if (pSrcGcs.IsValid())
        {
        pSrcGcs->InitFromWellKnownText(&warning, &warningErrorMsg, wktFlavor, wktWithoutFlavor.GetWCharCP());
        }

    // DestGCS
    GeoCoordinates::BaseGCSPtr pDestGcs = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    if (!pDestGcs->IsValid())
        return 0;

    double lowerX = 0.;
    double lowerY = 0.;
    double upperX = 0.;
    double upperY = 0.;

    baseGeoCoord_reproject(&lowerX, &lowerY, lower[0], lower[1], &*pSrcGcs, &*pDestGcs);
    baseGeoCoord_reproject(&upperX, &upperY, upper[0], upper[1], &*pSrcGcs, &*pDestGcs);

    pFootprint->InitFrom(lowerX, lowerY, upperX, upperY);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudData::_GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const
    {
    return ExtractThumbnail(pThumbnailBmp, width, height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudData::ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const
    {
    if (NULL == m_cloudFileHandle || NULL == m_cloudHandle)
        return ERROR;

    // Set density (number of points retrieved) according to number of pixels in bitmap. Using the total number of pixels in the bitmap * 4
    // seems to produce generally good results.
    float densityValue = (float) (width * height * 4);

    // Get transfo matrix to fit the point cloud in the thumbnail
    Transform transform;
    PointCloudVortex::GetTransformForThumbnail(transform, m_cloudHandle, width, height, m_view);

    bool needsWhiteBackground = PointCloudVortex::PointCloudNeedsWhiteBackground(m_cloudHandle);

    HRESULT hr = PointCloudVortex::ExtractPointCloud(pThumbnailBmp, width, height, m_cloudHandle, densityValue, transform, needsWhiteBackground);
    if (*pThumbnailBmp == NULL)
        return ERROR;

    if (hr != NOERROR)
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt PointCloudData::_GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const
    {
    return ExtractThumbnail(data, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt PointCloudData::ExtractThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const
    {
    if (NULL == m_cloudFileHandle || NULL == m_cloudHandle)
        return ERROR;
    
    // Set density (number of points retrieved) according to number of pixels in bitmap. Using the total number of pixels in the bitmap * 4
    // seems to produce generally good results.
    float densityValue = (float) (width * height * 4);
    
    // Get transfo matrix to fit the point cloud in the thumbnail
    Transform transform;
    PointCloudVortex::GetTransformForThumbnail(transform, m_cloudHandle, width, height, m_view);
    
    bool needsWhiteBackground = PointCloudVortex::PointCloudNeedsWhiteBackground(m_cloudHandle);
    
    HBITMAP* pThumbnailBmp = 0;
    HRESULT hr = PointCloudVortex::ExtractPointCloud(pThumbnailBmp, width, height, m_cloudHandle, densityValue, transform, needsWhiteBackground);
    if (*pThumbnailBmp == NULL)
        return ERROR;
    
    if (hr != NOERROR)
        return ERROR;
    
    // Get image data from HBitmap.
    BITMAPINFO bmpInfo;
    memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    GetDIBits(NULL, *pThumbnailBmp, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    GetDIBits(NULL, *pThumbnailBmp, 0, bmpInfo.bmiHeader.biHeight, buffer.data(), &bmpInfo, DIB_RGB_COLORS);
    if (buffer.empty())
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt PointCloudData::_SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const
    {
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt PointCloudData::_SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const
    {
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt PointCloudData::_SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const
    {
    CImage image;
    image.Attach(*pThumbnailBmp);
    image.Save(outFilename.GetNameUtf8().c_str());

    return SUCCESS;
    }
