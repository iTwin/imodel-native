/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RealityPlatformAPI.h"

#include <Bentley/BeFilename.h>
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
struct RealityData : public RefCountedBase
{
public:
    REALITYDATAPLATFORM_EXPORT StatusInt GetFootprint(DRange2dP pFootprint) const;
    REALITYDATAPLATFORM_EXPORT StatusInt GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const;
    REALITYDATAPLATFORM_EXPORT StatusInt GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;

    REALITYDATAPLATFORM_EXPORT StatusInt SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const;
    REALITYDATAPLATFORM_EXPORT StatusInt SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const;
    REALITYDATAPLATFORM_EXPORT StatusInt SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const;

protected:
    virtual ~RealityData() {};

    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const = 0;
    virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const = 0;
    virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const = 0;

    virtual StatusInt _SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const = 0;
    virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const = 0;
    virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const = 0;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct RasterData : public RealityData
{
public:
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Create(Utf8CP inFilename);

protected:
    RasterData(Utf8CP filename);
    virtual ~RasterData();

    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const override;
    virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;

    virtual StatusInt _SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const;
    StatusInt ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;


    bool Initialize();
    void Terminate();

    Utf8String m_filename;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct PointCloudData : public RealityData
{
public:
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Create(Utf8CP inFilename, PointCloudView view);

protected:
    PointCloudData(Utf8CP filename, PointCloudView view);
    virtual ~PointCloudData();

    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const override;
    virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;

    virtual StatusInt _SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const;
    StatusInt ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;

    void GetFile(Utf8CP inFilename);
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
struct WmsData : public RealityData
{
public:
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Create(Utf8CP url);

protected:
    WmsData(Utf8CP url);
    virtual ~WmsData();

    virtual StatusInt _GetFootprint(DRange2dP pFootprint) const override;
    virtual StatusInt _GetThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const override;
    virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;

    virtual StatusInt _SaveFootprint(DRange2dCR data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
    virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

private:
    StatusInt ExtractFootprint(DRange2dP pFootprint) const;
    StatusInt ExtractThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const;
    StatusInt ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;

    StatusInt GetFromServer(bvector<Byte>& buffer, Utf8StringCR url) const;

    bool Initialize();
    void Terminate();

    Utf8String m_url;
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE