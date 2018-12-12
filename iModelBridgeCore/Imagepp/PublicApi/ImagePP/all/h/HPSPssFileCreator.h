/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HPSPssFileCreator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ImagePP/all/h/HPSPssFile.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>

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
        Utf8String FileName = "c:\\image.itiff";
        Utf8String Projection = "Format to be defined";
        UInt32   PageNumber = 0;

        // Add one or several images
        MyCreator.AddImage(FileName, PageNumber, Projection);

        // Set the destination Coordinate System
        Utf8String DestinationProjection = "Format to be defined";
        MyCreator.SetDstProjection(DestinationProjection);


        // Create PSS file
        Utf8String PSSName = "c:\\MyPictureScript.pss";
        MyCreator.CreateFileW(PSSName);
    @end
    -----------------------------------------------------------------------------
*/
class HPSPssFileCreator
    {
public:
    IMAGEPP_EXPORT                  HPSPssFileCreator ();
    IMAGEPP_EXPORT                  ~HPSPssFileCreator();
    IMAGEPP_EXPORT HSTATUS          CreateFileW(Utf8String const& pi_rPssFileName);
    IMAGEPP_EXPORT void             AddImage(Utf8String const& pi_rImageName, uint32_t pi_pageNumber, GeoCoordinates::BaseGCSP pi_projection);

    IMAGEPP_EXPORT void             SetDstProjection(GeoCoordinates::BaseGCSP pi_projection);
    IMAGEPP_EXPORT GeoCoordinates::BaseGCSCP  GetDstProjectionCP() const;

protected:
    HFCPtr<HRFRasterFile>   GetRasterFile(Utf8String const& rImageName) const;

private:

    struct ImageDef
        {
        uint32_t                    m_pageNumber;
        Utf8String                     m_imageName;
        GeoCoordinates::BaseGCSPtr  m_projection;
        };

    typedef list <ImageDef> ImageList;
    ImageList                       m_imageList;
    GeoCoordinates::BaseGCSPtr      m_dstProjection;
    HFCPtr<HGFHMRStdWorldCluster>   m_pWorldCluster;
    };
END_IMAGEPP_NAMESPACE
