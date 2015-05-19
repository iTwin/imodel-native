/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RealityPlatformAPI.h"
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

enum class PointCloudView
{
    Top,
    Front,
    Right,
    Iso
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct MemoryStruct
    {
    void*   memory;
    size_t  size;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct MemoryStructTest
    {
    char*   memory;
    size_t  size;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct RealityDataHandler : public RefCountedBase
{
public:
    REALITYDATAPLATFORM_EXPORT StatusInt GetFootprint(DRange2dP pFootprint) const;

    //&&JFC TODO: Remove HBITMAP and windows header dependency.
    REALITYDATAPLATFORM_EXPORT StatusInt GetThumbnail(HBITMAP *pThumbnailBmp) const;

protected:
   
    virtual ~RealityDataHandler() {};
    virtual StatusInt  _GetFootprint(DRange2dP pFootprint) const = 0;
    virtual StatusInt  _GetThumbnail(HBITMAP *pThumbnailBmp) const = 0;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct RasterDataHandler : public RealityDataHandler
{
public:
    REALITYDATAPLATFORM_EXPORT static RefCountedPtr<RealityDataHandler> Create(WCharCP inFilename);

protected:

    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(HBITMAP *pThumbnailBmp) const override;

    RasterDataHandler(WCharCP filename);
    virtual ~RasterDataHandler();

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height) const;

    bool Initialize();
    void Terminate();

    WCharCP m_filename;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct PointCloudDataHandler : public RealityDataHandler
{
public:
    REALITYDATAPLATFORM_EXPORT static RefCountedPtr<RealityDataHandler> Create(WCharCP inFilename, PointCloudView view);

protected:
    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(HBITMAP *pThumbnailBmp) const override;

    PointCloudDataHandler(WCharCP filename, PointCloudView view);
    virtual ~PointCloudDataHandler();

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height) const;

    void GetFile(WCharCP inFilename);
    void CloseFile();
    bool Initialize();
    void Terminate();

    PointCloudView  m_view;
    uint32_t        m_cloudFileHandle;
    uint32_t        m_cloudHandle;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct WMSDataHandler : public RealityDataHandler
{
public:
    REALITYDATAPLATFORM_EXPORT static RefCountedPtr<RealityDataHandler> Create(WCharCP url, WCharCP outFilename);

protected:
    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(HBITMAP *pThumbnailBmp) const override;

    WMSDataHandler(StatusInt& status, WCharCP url, WCharCP outFilename);
    virtual ~WMSDataHandler();

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail() const;

    StatusInt GetFromServer(Utf8StringR url) const;

    bool Initialize();
    void Terminate();

    WCharCP m_url;
    WCharCP m_outFilename;
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE