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
//! The following classes represents the foundation of the Reality Data Service
//! client SDK. These classes enable to represent the information related to spatial 
//! entities as inventoried by the Reality Data Service.
//! The main class is SpatialEntity that represents a data that is geocoordinated. This class
//! does not contain the data proper but rather represents the informative portion of the
//! data. The class contains names, description, data provider, spatial footprint (2D),
//! and so on and makes reference to additional information but a potention link to a 
//! SpatialEntityMetadata instance. 
//!
//! Along with Reality Data are provided classes to represent documents and folder.
//!
//! The reality data service is a services that:
//!  - Exposes a list of reality data including footprint, type, name, description,
//!    owner, enterprise, and so on. This data is descriptive and serves as an access
//!    point to the data proper located in a dedicated Azure Blob Container.
//!  - Each Reality Data is provided with a specific Azure Blob Container in which, 
//!    documents are stored in a tree-like file struture like for any disc drive.
//!  - Offers Bentley CONNECT integrated permission access to create, access, manage,
//!    update or delete reality data according to rules defined within the CONNECT
//!    environment by individual enterprise administrator according to the usual roles 
//!    and permission system of Bentley CONNECT as well a to project related permission
//!    using common project permissions defined through the role base access control (RBAC)
//!    CONNECT sub-system
//!  - Although Reality Data is managed as Enterprise Asset than can optionnally used
//!    for CONNECT projects, the service offers a service maintaining a list of 
//!    project/reality data relationship. This registry is required for project related
//!    permissions and to ease the maintenance of the Enterprise Reality Data pool
//!    by the administrator.
//=====================================================================================



//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a pair RealityDataId/ProjectId indicating a relationship between them. 
//! RealityData can be linked with many projects and projects can link with many different reality data
//=====================================================================================
struct RealityDataProjectRelationship : public RefCountedBase
    {
public:
    //! Extracts a relationship from the given json instance.
    REALITYDATAPLATFORM_EXPORT static RealityDataProjectRelationshipPtr Create(Json::Value jsonInstance) { return new RealityDataProjectRelationship(jsonInstance); }

    //! Get/Set
    //! The RealityDataId of the RealityData linked to the project. Normally both RealityData and Project are of the same enterprise but if the RealityData is marked as public
    //! it may be referenced by projects from external enterprises. (Not so sure about linking external projects to public data)
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;

    //! The project id that is linked with reality data.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProjectId() const;
    
private:
    RealityDataProjectRelationship(Json::Value jsonInstance);
    RealityDataProjectRelationship() {};
    Utf8String m_realityDataId;
    Utf8String m_projectId;
    }; 


//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a document stored in the Reality Data Service.
//! It does not contain the actual data of the document but only information about
//! this document.
//=====================================================================================
struct RealityDataDocument : public RefCountedBase
    {
public:
    //! Create a document
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
//! This class is used to represent a folder. Here the folder object only contains the
//! information about this folder, not the list of sub-folder and documents it contains.
//=====================================================================================
struct RealityDataFolder : public RefCountedBase
    {
public:
    //! Creates a representation of the folder. 
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