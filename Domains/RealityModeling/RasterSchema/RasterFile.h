/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_RASTER_NAMESPACE

//=======================================================================================
// This is a wrapper class around a HFCPtr<HRFRasterFile> basically. It is needed because we want to 
// avoid to include directly a HFCPtr<HRFRasterFile> member in a public header like RasterFileHandler.h.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct RasterFile : public RefCountedBase
{
private:
            ImagePP::HFCPtr<ImagePP::HRFRasterFile>         m_HRFRasterFilePtr;
            ImagePP::HFCPtr<ImagePP::HRARaster>             m_rasterPtr;
            ImagePP::HFCPtr<ImagePP::HRFPageFile>           m_pageFilePtr;            // Sister file decorator.
            ImagePP::HFCPtr<ImagePP::HGF2DCoordSys>         m_pPhysicalCoordSys;

                                                            RasterFile(Utf8StringCR resolvedName);
            virtual                                         ~RasterFile();
            int                                             ComputeBufferSize(size_t& bufferSize, const Point2d& imageSize, int imageFormat) const;
    
            StatusInt                                       GetSampleStatistics(double& minValue, double& maxValue);
            ImagePP::HFCPtr<ImagePP::HRFPageDescriptor>     GetPageDescriptor() const;
            ImagePP::HFCPtr<ImagePP::HRARaster>             DecorateRaster(ImagePP::HFCPtr<ImagePP::HRAStoredRaster>& pStoredRaster);

public:
    static  ImagePP::HGF2DWorldCluster&                     GetWorldCluster();

    static  RasterFilePtr                                   Create(Utf8StringCR resolvedName);
            ImagePP::HFCPtr<ImagePP::HRFRasterFile>         OpenRasterFile(Utf8StringCR resolvedName);
            ImagePP::HRFRasterFile&                         GetHRFRasterFile() const;
            uint32_t                                        GetWidth() const;
            uint32_t                                        GetHeight() const;
            void                                            GetSize(Point2d* sizeP) const;
            ImagePP::HRARaster*                             GetRasterP();
            ImagePP::HFCPtr<ImagePP::HGF2DCoordSys>         GetPhysicalCoordSys();            
            void                                            GetCorners (DPoint3dP corners) const;

            DMatrix4d                                       GetPhysicalToLowerLeft() const;
            DMatrix4d                                       GetGeoTransform() const;
            GeoCoordinates::BaseGCSPtr                      GetBaseGcs() const;
};

END_BENTLEY_RASTER_NAMESPACE


