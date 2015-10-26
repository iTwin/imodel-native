/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

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
            int                                             ComputeBufferSize(size_t& bufferSize, const Point2d& imageSize, int imageFormat) const;
    static  ImagePP::HGF2DWorldCluster&                     GetWorldCluster();
            StatusInt                                       GetSampleStatistics(double& minValue, double& maxValue);
            ImagePP::HFCPtr<ImagePP::HRFPageDescriptor>     GetPageDescriptor() const;
            ImagePP::HFCPtr<ImagePP::HRARaster>             DecorateRaster(ImagePP::HFCPtr<ImagePP::HRAStoredRaster>& pStoredRaster);

public:
    static  RasterFilePtr                                   Create(Utf8StringCR resolvedName);
            ImagePP::HFCPtr<ImagePP::HRFRasterFile>         OpenRasterFile(Utf8StringCR resolvedName);
            ImagePP::HRFRasterFile*                         GetHRFRasterFileP() const;
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

END_BENTLEY_RASTERSCHEMA_NAMESPACE


