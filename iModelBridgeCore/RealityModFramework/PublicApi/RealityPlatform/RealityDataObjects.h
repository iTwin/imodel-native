/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataObjects.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Overview:
//! The following classes represents the foundation of the GeoCoordination Service
//! client SDK. These classes enable to represent the information related to spatial 
//! entities as inventoried by the GeoCoordination Service.
//! The main class is SpatialEntity that represents a data that is geocoordinated. This class
//! does not contain the data proper but rather represents the informative portion of the
//! data. The class contains names, description, data provider, spatial footprint (2D),
//! and so on and makes reference to additional information but a potention link to a 
//! SpatialEntityMetadata instance. 
//!
//! The SpatialEntityMetadata class enables to represent a few fields extracted from 
//! the metadata and possibly a reference to an external page or document containing this
//! metadata. An instance of a metadata may or may not be shared by spatial entities.
//!
//! The spatial entity makes reference to a list of SpatialEntityDataSource. It 
//! indicates the location of the various data sources (files or stream) where can
//! be obtained the data. There can be many data source for a same spatial entity
//! for example when mirror site are used for distribution or if the data
//! is distributed through different file formats or different geographic coordinate
//! system. A data source thus contains the file location (URL) and indications about the
//! format of this file for example if it is a zip file that contains the final file 
//! (a compound document) and the name of the file in the compound (Location in
//! compound) as well as geographic coordinate system and file format (usually file
//! extension).
//!
//! Spatial data sources refer to a SpatialEntityServer class that keeps information
//! about the server. This class contains information about the server proper including
//! authentication required, communication protocol (http, ftp) and data access 
//! protocol (WMS, S3MX, ftp, http). It may contain statitics concerning the
//! robustness of the server (uptime, last time checked, etc)
//!
//! An additional class is provided that is not used by the SpatialEntity model used
//! to represent and store thumbnails. Although unused at the moment it may be
//! in a future to manage thumbnails stored as part of the service.
//=====================================================================================



//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a pair RealityDataId/ProjectId indicating a relationship beteen them. 
//! RealityData can be linked with many projects and projects can link with many different reality data
//=====================================================================================
struct RealityDataProjectRelationship : public RefCountedBase
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static RealityDataProjectRelationshipPtr Create(Json::Value jsonInstance) { return new RealityDataProjectRelationship(jsonInstance); }

    //! Get/Set
    //! The RealityDataId of the RealityData linked to the project. Normally both RealityData and Project are of the same enterprise but if the RealityData is marked as public
    //! it may be referenced by projects from external enterprises
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;

    //! Get/Set
    //! The project that is linked with reality data.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProjectId() const;
    
private:
    RealityDataProjectRelationship(Json::Value jsonInstance);
    RealityDataProjectRelationship() {};
    Utf8String m_realityDataId;
    Utf8String m_projectId;
    }; 


//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a document
//=====================================================================================
struct RealityDataDocument : public RefCountedBase
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static RealityDataDocumentPtr Create(Json::Value jsonInstance) { return new RealityDataDocument(jsonInstance); }

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContainerName() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFolderId() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAccessUrl() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContentType() const;
    
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSize() const;

private:
    RealityDataDocument(Json::Value jsonInstance);
    RealityDataDocument() {}
    Utf8String m_containerName;
    Utf8String m_name;
    Utf8String m_id;
    Utf8String m_folderId;
    Utf8String m_accessUrl;
    Utf8String m_realityDataId;
    Utf8String m_contentType;
    Utf8String m_size;
    }; 


//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a folder
//=====================================================================================
struct RealityDataFolder : public RefCountedBase
    {
public:
    //! Create 
    REALITYDATAPLATFORM_EXPORT static RealityDataFolderPtr Create(Json::Value jsonInstance) { return new RealityDataFolder(jsonInstance); }

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetParentId() const;

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;

private:
    RealityDataFolder(Json::Value jsonInstance);
    RealityDataFolder();
    Utf8String m_name;
    Utf8String m_parentId;
    Utf8String m_realityDataId;
    }; 


END_BENTLEY_REALITYPLATFORM_NAMESPACE