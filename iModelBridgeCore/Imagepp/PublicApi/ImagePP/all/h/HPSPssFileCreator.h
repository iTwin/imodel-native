/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HPSPssFileCreator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Imagepp/all/h/HPSPssFile.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFile;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Stephane Poulin

    This class is used to create picture script files. Images that are added to
    the picture script file can optionally have a coordinate system. If an image
    coordinate system and a destination coordinate system are both provided, then
    the image will be reprojected to the destination coordinate system.

    @code
        // Example code
        HPSPssFileCreator MyCreator;
        WString FileName = "c:\\image.itiff";
        WString Projection = "Format to be defined";
        UInt32   PageNumber = 0;

        // Add one or several images
        MyCreator.AddImage(FileName, PageNumber, Projection);

        // Set the destination Coordinate System
        WString DestinationProjection = "Format to be defined";
        MyCreator.SetDstProjection(DestinationProjection);


        // Create PSS file
        WString PSSName = "c:\\MyPictureScript.pss";
        MyCreator.CreateFileW(PSSName);
    @end
    -----------------------------------------------------------------------------
*/
class HPSPssFileCreator
    {
public:
    IMAGEPP_EXPORT                  HPSPssFileCreator ();
    IMAGEPP_EXPORT                  ~HPSPssFileCreator();
    IMAGEPP_EXPORT HSTATUS          CreateFileW(WString const& pi_rPssFileName);
    IMAGEPP_EXPORT void             AddImage(WString const& pi_rImageName, uint32_t pi_pageNumber, IRasterBaseGcsP pi_projection);

    IMAGEPP_EXPORT void              SetDstProjection(IRasterBaseGcsP pi_projection);
    IMAGEPP_EXPORT IRasterBaseGcsCP  GetDstProjectionCP() const;

protected:
    HFCPtr<HRFRasterFile>   GetRasterFile(WString const& rImageName) const;

private:

    struct ImageDef
        {
        uint32_t                    m_pageNumber;
        WString                     m_imageName;
        IRasterBaseGcsPtr           m_projection;
        };

    typedef list <ImageDef> ImageList;
    ImageList                       m_imageList;
    IRasterBaseGcsPtr               m_dstProjection;
    HFCPtr<HGFHMRStdWorldCluster>   m_pWorldCluster;
    };
END_IMAGEPP_NAMESPACE