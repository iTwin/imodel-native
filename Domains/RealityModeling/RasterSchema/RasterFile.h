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
            HFCPtr<HRFRasterFile>       m_HRFRasterFilePtr;
            HFCPtr<HRAStoredRaster>     m_storedRasterPtr;
            HFCPtr<HGF2DWorldCluster>   m_worldClusterPtr;

                                        RasterFile(WCharCP inFilename);
            int                         ComputeBufferSize(size_t& bufferSize, const Point2d& imageSize, int imageFormat) const;
            HGF2DWorldCluster*          GetWorldClusterP();

public:
    static  RasterFilePtr               Create(WCharCP inFilename);
            HFCPtr<HRFRasterFile>       OpenRasterFile(WCharCP inFilename);
            HRFRasterFile*              GetHRFRasterFileP() const;
            uint32_t                    GetWidth() const;
            uint32_t                    GetHeight() const;
            void                        GetSize(Point2d* sizeP) const;
            void                        GetBitmap(HFCPtr<HRABitmapBase> pBitmap);
            HRAStoredRaster*            GetStoredRasterP();
            HFCPtr<HGF2DCoordSys>       GetPhysicalCoordSys();
            HFCPtr<HGF2DTransfoModel>   GetSLOTransfoModel() const;
            void                        GetCorners(DPoint3dP corners);
            GeoCoordinates::BaseGCSPtr  GetBaseGcs();
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


