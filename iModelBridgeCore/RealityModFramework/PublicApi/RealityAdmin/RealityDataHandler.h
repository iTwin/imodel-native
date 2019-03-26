/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/RealityDataHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Windows.h>

#include <RealityPlatform/RealityPlatformAPI.h>

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
//! Base class for information extraction on various data types.
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct RealityDataExtract : public RefCountedBase
    {
    public:
        // Footprint.
        REALITYDATAPLATFORM_EXPORT StatusInt GetFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const;
        REALITYDATAPLATFORM_EXPORT StatusInt SaveFootprint(bvector<GeoPoint2d>& data, BeFileNameCR outFilename) const;

        // Thumbnail.
        REALITYDATAPLATFORM_EXPORT StatusInt GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const;
        REALITYDATAPLATFORM_EXPORT StatusInt GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;    
        REALITYDATAPLATFORM_EXPORT StatusInt SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const;
        REALITYDATAPLATFORM_EXPORT StatusInt SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const;

    protected:
        virtual ~RealityDataExtract() {};

        // Footprint.
        virtual StatusInt _GetFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const = 0;
        virtual StatusInt _SaveFootprint(bvector<GeoPoint2d>& data, BeFileNameCR outFilename) const = 0;

        // Thumbnail.       
        virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const = 0;
        virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const = 0;    
        virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const = 0;
        virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const = 0;
    };

//=====================================================================================
//! Extract information on raster data.
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct RasterData : public RealityDataExtract
    {
    public:
        REALITYDATAPLATFORM_EXPORT static RealityDataExtractPtr Create(Utf8CP inFilename);

        //! Get resolution in meters. Format is "widthxheight".
        REALITYDATAPLATFORM_EXPORT const Utf8String ComputeResolutionInMeters();

    protected:
        RasterData(Utf8CP filename);
        virtual ~RasterData();

        // Footprint.
        virtual StatusInt _GetFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const override;
        virtual StatusInt _SaveFootprint(bvector<GeoPoint2d>& data, BeFileNameCR outFilename) const override;

        // Thumbnail.        
        virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const override;
        virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;
        virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
        virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

    private:
        StatusInt ExtractFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const;

        StatusInt ExtractThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const;
        StatusInt ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;

        bool Initialize();
        void Terminate();

        Utf8String m_filename;
    };

//=====================================================================================
//! Extract information on pointcloud data.
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct PointCloudData : public RealityDataExtract
    {
    public:
        REALITYDATAPLATFORM_EXPORT static RealityDataExtractPtr Create(Utf8CP inFilename, PointCloudView view);

    protected:
        PointCloudData(Utf8CP filename, PointCloudView view);
        virtual ~PointCloudData();

        // Footprint.
        virtual StatusInt _GetFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const override;
        virtual StatusInt _SaveFootprint(bvector<GeoPoint2d>& data, BeFileNameCR outFilename) const override;    

        // Thumbnail.
        virtual StatusInt _GetThumbnail(bvector<Byte>& data, uint32_t width, uint32_t height) const override;
        virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;
        virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
        virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

    private:
        StatusInt ExtractFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const;

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
//! Extract information on wms data.
//! @bsiclass                                   Jean-Francois.Cote               4/2015
//=====================================================================================
struct WmsData : public RealityDataExtract
    {
    public:
        REALITYDATAPLATFORM_EXPORT static RealityDataExtractPtr Create(Utf8CP url);

    protected:
        WmsData(Utf8CP url);
        virtual ~WmsData();

        // Footprint.
        virtual StatusInt _GetFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const override;
        virtual StatusInt _SaveFootprint(bvector<GeoPoint2d>& data, BeFileNameCR outFilename) const override;    

        // Thumbnail.
        virtual StatusInt _GetThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const override;
        virtual StatusInt _GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const override;
        virtual StatusInt _SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const override;
        virtual StatusInt _SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const override;

    private:
        StatusInt ExtractFootprint(bvector<GeoPoint2d>* pFootprint, DRange2dP pFootprintExtents) const;

        StatusInt ExtractThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const;
        StatusInt ExtractThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const;

        StatusInt GetFromServer(bvector<Byte>& buffer, Utf8StringCR url) const;

        bool Initialize();
        void Terminate();

        Utf8String m_url;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE