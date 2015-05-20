/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/IPointCloudFileQuery.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/ExportMacros.h>

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

/*=================================================================================**//**
* \addtogroup Pointcloud
*/
//@{

/*---------------------------------------------------------------------------------**//**
* Interface to query information about a point cloud file. 
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudFileQuery : public IRefCounted
    {
/*__PUBLISH_SECTION_END__*/
    protected:
        virtual WCharCP     _GetFileName() const = 0;
        virtual uint64_t    _GetNumberOfPoints() const = 0;
        virtual uint32_t    _GetNumberOfClouds() const = 0;
        virtual bool        _HasIntensityChannel() const = 0;
        virtual bool        _HasClassificationChannel() const = 0;
        virtual bool        _HasRGBChannel() const = 0;
        virtual bool        _HasNormalChannel() const = 0;
        virtual StatusInt   _GetMetaData (WStringR name, uint32_t& num_clouds, uint64_t& num_points, DPoint3dP lowerBound, DPoint3dP upperBound) const = 0;
        virtual StatusInt   _GetMetaTag(WStringCR tagName, WStringR value) const = 0;
        virtual uint32_t    _GetNumUserMetaSections() const = 0;
        virtual uint32_t    _GetNumUserMetaTagsInSection(int32_t sectionIndex) const = 0;
        virtual StatusInt   _GetUserMetaSectionName (int32_t sectionIndex, WStringR name) const = 0;
        virtual StatusInt   _GetUserMetaTagByIndex(int32_t sectionIndex, int32_t tagIndex, WStringR tagName, WStringR tagValue) const = 0;
        virtual StatusInt   _GetUserMetaTagByName(WStringCR sectionDotName, WStringR tagValue) const = 0;

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
    public:

        /*---------------------------------------------------------------------------------**//**
        * @return   Returns the file name of a point cloud
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT WCharCP   GetFileName() const;
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns the number of points of a point cloud
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT uint64_t     GetNumberOfPoints() const;
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns the number of clouds of a point cloud
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT uint32_t     GetNumberOfClouds() const;
        
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns whether the point cloud has an intensity channel
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT bool        HasIntensityChannel() const;
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns whether the point cloud has a classification channel
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT bool        HasClassificationChannel() const;
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns whether the point cloud has an RGB channel
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT bool        HasRGBChannel() const;
        /*---------------------------------------------------------------------------------**//**
        * @return   Returns whether the point cloud has a normal channel
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT bool        HasNormalChannel() const;

        /*---------------------------------------------------------------------------------**//**
        * Returns basic meta data from the POD file
        *
        * @param    name     OUT      the name of the Scene.
        * @param    num_clouds  OUT      the number of point clouds in the Scene file
        * @param    num_points  OUT      Receives the total num of points in the Scene file, 
        *                                note the use of 64bit unsigned integer since the number
        *                                of points can exceed the capacity of a 32bit integer.
        * @param    lowerBound  OUT      the lower bounding box extent of the point cloud Scene
        * @param    upperBound  OUT      the upper bounding box extent of the point cloud Scene
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt   GetMetaData (WStringR name, uint32_t& num_clouds, uint64_t& num_points, DPoint3dP lowerBound, DPoint3dP upperBound) const;

        /*---------------------------------------------------------------------------------**//**
        * Returns a specific meta data tag from the Scene. 
        * Note that the meta tag may not be available in the Scene.
        *
        * @param    tagName     IN       the name of the tag formatted as "section.tagname". See remarks for accepted values
        * @param    value       OUT      the value of the tag
        * @return   SUCCESS or ERROR
        * @note
        * <BR>The following tag name values are accepted<BR>
        * <TABLE BORDER="1" CELLPADDING="5" CELLSPACING="0">
        * <CAPTION></CAPTION>
        * <TR> <TH>Tag Name</TH>                        <TH>Description</TH> </TR>
        * <TR> <TD>Instrument.ScannerManufacturer</TD>  <TD>The Manufacturer of the sensor used to capture the data</TD> </TR>
        * <TR> <TD>Instrument.ScannerModel</TD>         <TD>The name of the scanner Model.</TD> </TR>
        * <TR> <TD>Instrument.ScannerSerial</TD>        <TD>The serial number of the scanner</TD> </TR>
        * <TR> <TD>Instrument.CameraModel</TD>          <TD>The camera model used to capture RGB</TD> </TR>
        * <TR> <TD>Instrument.CameraSerial</TD>         <TD>The camera serial number</TD> </TR>
        * <TR> <TD>Instrument.CameraLens</TD>           <TD>The camera lens</TD> </TR>
        * <TR> <TD>Survey.Company</TD>                  <TD>The company that captured the data</TD> </TR>
        * <TR> <TD>Survey.Operator</TD>                 <TD>The operator name</TD> </TR>
        * <TR> <TD>Survey.ProjectName</TD>              <TD>The project name</TD> </TR>
        * <TR> <TD>Survey.ProjectCode</TD>              <TD>The project code, does not have to conform to particular convention</TD> </TR>
        * <TR> <TD>Survey.DateOfCapture</TD>            <TD>The date of capture, must be specified as YYYY-MM-DD, for example 2009-06-29</TD> </TR>
        * <TR> <TD>Survey.Site</TD>                     <TD>Text describing the site or object captured</TD> </TR>
        * <TR> <TD>Survey.SiteLong</TD>                 <TD>Site's Longitude. This does not affect data positioning and is for information only</TD> </TR>
        * <TR> <TD>Survey.SiteLat</TD>                  <TD>Sites Latitude. This does not affect data positioning and is for information only</TD> </TR>
        * <TR> <TD>Survey.CoordinateSystem</TD>         <TD>Coordinate system descriptor. This does not affect data positioning and is for information only</TD> </TR>
        * <TR> <TD>Survey.Postcode</TD>                 <TD> Postal code of site</TD> </TR>
        * <TR> <TD>Survey.ZipCode</TD>                  <TD> Zip code of site</TD> </TR>
        * <TR> <TD>Description.Description</TD>         <TD>Description of the scans contents</TD> </TR>
        * <TR> <TD>Description.Keywords</TD>            <TD>Keywords describing data, multiple keywords are separated by semicolons</TD> </TR>
        * <TR> <TD>Description.Category</TD>            <TD>Category, one of:
        *                                                     <BR>Aerial Lidar
        *                                                     <BR>Terrestrial Phase Based
        *                                                     <BR>Terrestrial Time of Flight
        *                                                     <BR>Mobile mapping
        *                                                     <BR>Bathymetric
        *                                                     <BR>Photogrammetric
        *                                                     <BR>Synthesized"</TD> </TR>
        * <TR> <TD>Audit.ScanPaths</TD>             <TD>Original file paths of source input files. To retrieve multiple file paths the function can be called multiple times each time returning one of the file paths until an empty string is returned.</TD> </TR>
        * <TR> <TD>Audit.OriginalNumScans</TD>      <TD>Number of original scans, note this may differ from number of original files</TD> </TR>
        * <TR> <TD>Audit.CreatorApp</TD>            <TD>The application that created the POD file</TD> </TR>
        * <TR> <TD>Audit.Generation</TD>            <TD>The generation of the file, where each modification and resave of the file increments the generation number.</TD> </TR>
        * <TR> <TD>Audit.DateCreated</TD>           <TD>The date the file was originally created, this may differ to the system date created value of the file.</TD> </TR>
        * </TABLE>
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt   GetMetaTag(WStringCR tagName, WStringR value) const;

        /*---------------------------------------------------------------------------------**//**
        * Returns the number of user meta data sections
        * @return   the number of user meta data sections
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT uint32_t      GetNumUserMetaSections() const;
        
        /*---------------------------------------------------------------------------------**//**
        * Returns the number of user meta data tags in section
        * @param    sectionIndex    IN      The zero based index of the section of meta data tags
        * @return   the number  of user meta data tags in section
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT uint32_t      GetNumUserMetaTagsInSection(int32_t sectionIndex) const;
        
        /*---------------------------------------------------------------------------------**//**
        * Returns the number of user meta data tags in section
        * @param    sectionIndex    IN      The zero based index of the section of meta data tags
        * @param    name    OUT      The name of the user meta data section
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt   GetUserMetaSectionName (int32_t sectionIndex, WStringR name) const;
        
        /*---------------------------------------------------------------------------------**//**
        * Returns a user meta data tag name and value pair by the section and tag index.
        * @param    sectionIndex    IN      The zero based index of the section of meta data tags
        * @param    tagIndex    IN      The zero based index of the user meta tag within the section
        * @param    tagName     OUT      the name of the tag.
        * @param    tagValue    OUT      the value of the tag
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt   GetUserMetaTagByIndex(int32_t sectionIndex, int32_t tagIndex, WStringR tagName, WStringR tagValue) const;

        /*---------------------------------------------------------------------------------**//**
        * Returns a user meta data tag name and value pair by the section and tag index.
        * @param    sectionDotName    IN      The case-sensitive name of the meta tag formatted as “section.name”
        * @param    tagValue    OUT      the value of the tag
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt   GetUserMetaTagByName(WStringCR sectionDotName, WStringR tagValue) const;
    };


//@}

END_BENTLEY_BEPOINTCLOUD_NAMESPACE

