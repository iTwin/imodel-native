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
            ImagePP::HFCPtr<ImagePP::HRFRasterFile>       m_HRFRasterFilePtr;
            ImagePP::HFCPtr<ImagePP::HRAStoredRaster>     m_storedRasterPtr;
            ImagePP::HFCPtr<ImagePP::HGF2DWorldCluster>   m_worldClusterPtr;

                                        RasterFile(WCharCP inFilename);
            int                         ComputeBufferSize(size_t& bufferSize, const Point2d& imageSize, int imageFormat) const;
            ImagePP::HGF2DWorldCluster*          GetWorldClusterP();

public:
    static  RasterFilePtr               Create(WCharCP inFilename);
            ImagePP::HFCPtr<ImagePP::HRFRasterFile>       OpenRasterFile(WCharCP inFilename);
            ImagePP::HRFRasterFile*              GetHRFRasterFileP() const;
            uint32_t                    GetWidth() const;
            uint32_t                    GetHeight() const;
            void                        GetSize(Point2d* sizeP) const;
            void                        GetBitmap(ImagePP::HFCPtr<ImagePP::HRABitmapBase> pBitmap);
            ImagePP::HRAStoredRaster*            GetStoredRasterP();
            ImagePP::HFCPtr<ImagePP::HGF2DCoordSys>       GetPhysicalCoordSys();
            DMatrix4d                   GetPhysicalToLowerLeft() const;
            void                        GetCorners(DPoint3dP corners);
            GeoCoordinates::BaseGCSPtr  GetBaseGcs();
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


