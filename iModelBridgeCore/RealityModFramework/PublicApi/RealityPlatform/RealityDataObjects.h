/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <Bentley/DateTime.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Overview:
//! The following classes represents the foundation of the Reality Data Service
//! client SDK. These classes enable to represent the information related to spatial 
//! entities as inventoried by the Reality Data Service.
//! The main class is RealityData that represents a data that is geocoordinated. This class
//! does not contain the data proper but rather represents the informative portion of the
//! data. The class contains names, description, data provider, spatial footprint (2D),
//! and so on and makes reference to additional information but a potential link to a 
//! RealityDataMetadata instance. 
//!
//! Along with Reality Data are provided classes to represent documents and folder.
//!
//! The reality data service is a services that:
//!  - Exposes a list of reality data including footprint, type, name, description,
//!    owner, enterprise, and so on. This data is descriptive and serves as an access
//!    point to the data proper located in a dedicated Azure Blob Container.
//!  - Each Reality Data is provided with a specific Azure Blob Container in which, 
//!    documents are stored in a tree-like file structure like for any disc drive.
//!  - Offers Bentley CONNECT integrated permission access to create, access, manage,
//!    update or delete reality data according to rules defined within the CONNECT
//!    environment by individual enterprise administrator according to the usual roles 
//!    and permission system of Bentley CONNECT as well a to project related permission
//!    using common project permissions defined through the role base access control (RBAC)
//!    CONNECT sub-system
//!  - Although Reality Data is managed as Enterprise Asset than can optionally used
//!    for CONNECT projects, the service offers a service maintaining a list of 
//!    project/reality data relationship. This registry is required for project related
//!    permissions and to ease the maintenance of the Enterprise Reality Data pool
//!    by the administrator.
//=====================================================================================



//=====================================================================================
//! @bsiclass                                           Spencer.Mason 2017/01
//! This class is used to represent a triplet RealityDataId/RelatedId/RelationType indicating a relationship between them. 
//! RealityData can be linked with many projects and projects can link with many different reality data
//=====================================================================================
struct RealityDataRelationship : public RefCountedBase
    {
public:
    //! Extracts a relationship from the given json instance.
    REALITYDATAPLATFORM_EXPORT static RealityDataRelationshipPtr Create(Json::Value jsonInstance) { return new RealityDataRelationship(jsonInstance); }
    REALITYDATAPLATFORM_EXPORT static RealityDataRelationshipPtr Create() { return new RealityDataRelationship(); }

    //! Get/Set
    //! The RealityDataId of the RealityData linked to the project. Normally both RealityData and Project are of the same enterprise but if the RealityData is marked as public
    //! it may be referenced by projects from external enterprises. (Not so sure about linking external projects to public data)
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;
    REALITYDATAPLATFORM_EXPORT void SetRealityDataId(Utf8StringCR realityDataId);
    
    //! The project id that is linked with reality data.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRelatedId() const;
    REALITYDATAPLATFORM_EXPORT void SetRelatedId(Utf8StringCR relatedId);

    //! The relation type. The type of relationship is a string key that can be in the reserved list or not (user defined type).
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRelationType() const;
    REALITYDATAPLATFORM_EXPORT void SetRelationType(Utf8StringCR relationType);

    //! The date the relation was created.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetCreationDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetCreationDateTime(DateTimeCR createdDate);

    //! The date the relation was modified.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetModifiedDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetModifiedDateTime(DateTimeCR createdDate);
    
protected:
    REALITYDATAPLATFORM_EXPORT RealityDataRelationship(Json::Value jsonInstance);
    REALITYDATAPLATFORM_EXPORT RealityDataRelationship();
    Utf8String m_realityDataId;
    Utf8String m_relatedId;
    Utf8String m_relationType;
    DateTime   m_createdDateTime;
    DateTime   m_modifiedDateTime;

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
    REALITYDATAPLATFORM_EXPORT static RealityDataDocumentPtr Create() { return new RealityDataDocument(); }

    //! The reality data the document is part of.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;
    REALITYDATAPLATFORM_EXPORT void SetRealityDataId(Utf8StringCR realityDataId);

    //! The Id of a document is the full encoded path and name of the document on the service.
    //! As a convenience the id can be set by providing the folder or folder id and name
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8StringCR id);
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8StringCR folderId, Utf8StringCR name);
    REALITYDATAPLATFORM_EXPORT void SetId(RealityDataFolderCR folder, Utf8StringCR name);

    //! Name of the document (name of the file) including extensions.
    //! The name of the document can be set by setting the id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;

    //! Cannot be set. Implied by the given Id. The folder id can be set by setting the id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFolderId() const;

    //! Cannot be set. Implied by the Id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAccessUrl() const;

    //! Cannot be set. Implied by the Reality Data Id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContainerName() const;

    //! Cannot be set. Implied by the Name of the document. The content type is the extension of the Name of the document.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContentType() const;
    
    //! Cannot be set. Computed based on the file content in the service
    REALITYDATAPLATFORM_EXPORT uint64_t GetSize() const;

protected:
    REALITYDATAPLATFORM_EXPORT RealityDataDocument(Json::Value jsonInstance);
    REALITYDATAPLATFORM_EXPORT RealityDataDocument();
    Utf8String m_containerName;
    Utf8String m_name;
    Utf8String m_id;
    Utf8String m_folderId;
    Utf8String m_accessUrl;
    Utf8String m_realityDataId;
    Utf8String m_contentType;
    uint64_t m_size;
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

    //! The reality data the folder is part of.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;
    REALITYDATAPLATFORM_EXPORT void SetRealityDataId(Utf8StringCR realityDataId);

    //! The Id of a folder is the full encoded path folder on the service
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8StringCR id);
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8StringCR parentId, Utf8StringCR folderName);
    REALITYDATAPLATFORM_EXPORT void SetId(RealityDataFolderCR parentFolder, Utf8StringCR folderName); 

    //! Name of the folder. The name can be set by setting the id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;

    //! The parent id. The parent id is implied in the id
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetParentId() const;

protected:
    REALITYDATAPLATFORM_EXPORT RealityDataFolder(Json::Value jsonInstance);
    REALITYDATAPLATFORM_EXPORT RealityDataFolder();
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_parentId;
    Utf8String m_realityDataId;
    }; 


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! The central class of the Reality Data model. It represents a spatial
//! geocoordinated data meant to represent reality modeling data. 
//! In the context of our services the concepts Reality Data and Spatial Entities are similar
//! except for a few fields and the way data sources are designated, a reality data
//! having a single data source while spatial entities often have many data sources.
//! It contains all fields necessary to represent the reality data
//! except the actual data.
//=====================================================================================
struct RealityDataBase : public RefCountedBase
{
public:

    // This enum is intentionally not a en enum class to enable conversion to int and oring values
    // The classifications represent the main classification of the data. Most reality data will be model
    // classified except for specialized data such as imagery and terrain.
    enum Classification
        {
        UNDEFINED_CLASSIF = 0x00,
        IMAGERY = 0x1,
        TERRAIN = 0x2,
        MODEL = 0x4,
        PINNED = 0x8,
        };

    // This enum is intentionally not a en enum class to enable conversion to int and oring values.
    enum Visibility
        {
        UNDEFINED_VISIBILITY = 0x00,
        PUBLIC_VISIBILITY = 0x01,
        ENTERPRISE = 0x02,
        PERMISSION = 0x04,
        PRIVATE = 0x08
        };


    //! Get/Set
    //! Identifier of spatial entity/reality data
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetIdentifier() const;
    REALITYDATAPLATFORM_EXPORT void SetIdentifier(Utf8CP identifier);

    //! Get/Set
    //! Name of spatial entity/ reality data
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
    REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

    //! Get/Set
    //! A string formed by 2 floating point values separated by a ‘x’ character. 
    //! The first double indicates the resolution in meter in the first axis and the second in the second axis.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetResolution() const; // in meters.
    REALITYDATAPLATFORM_EXPORT void SetResolution(Utf8CP resolution); // in meters.

    //! Returns the mean squared of x and y resolution in the field
    REALITYDATAPLATFORM_EXPORT double GetResolutionValue() const; // in meters.

    //! A string indicating the accuracy in number or numberxnumber format. 
    //! If the accuracy is different in X and Y then two numbers may be provided
    //! separated by a 'x' character. Ex: 15.56x13.45
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAccuracy() const;
    REALITYDATAPLATFORM_EXPORT void SetAccuracy(Utf8CP accuracy);

    //! Returns the mean squared of x and y accuracy in the field
    REALITYDATAPLATFORM_EXPORT double GetAccuracyValue() const; // in meters.

    //! The main classification of the spatial entity. See Classification for details.
    REALITYDATAPLATFORM_EXPORT Classification GetClassification() const;
    REALITYDATAPLATFORM_EXPORT void SetClassification(Classification classification);

    //! Returns the classification tag: one of "Model", "Imagery", "Terrain", "Pinned" or "Undefined"
    REALITYDATAPLATFORM_EXPORT Utf8String GetClassificationTag() const;

    //! Enables setting the classification by a string. The only valid values are "Model", "Imagery", "Terrain", "Pinned" and "Undefined"
    //! Returns SUCCESS if value set and ERROR if given tag is invalid
    REALITYDATAPLATFORM_EXPORT StatusInt SetClassificationByTag(Utf8CP classificationTag);

    //! Static helper method: converts a classification tag to a classification
    //! Returns SUCCESS if value valid and ERROR if given tag is invalid. In case of error the classification value remains unchanged
    REALITYDATAPLATFORM_EXPORT static StatusInt GetClassificationFromTag(Classification& returnedClassification, Utf8CP classificationTag);
    REALITYDATAPLATFORM_EXPORT static Utf8String GetTagFromClassification(Classification classification);

    //! For later ... currently unused
    // REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSubClassification() const;
    // REALITYDATAPLATFORM_EXPORT void SetSubClassification(Utf8CP subClassification);


    //! Code indicating the visibility of the data. The recognized keywords are:
    //! PUBLIC_VISIBILITY - Usually indicates data is public thus usable by anyone
    //! ENTERPRISE - Usually indicates data can be used by any member of the owning enterprise regardless of access rights
    //! PERMISSION - Usually indicates that data can be used by someone having the appropriate access rights. This is the default value.
    //! PRIVATE - Usually indicates only the owner of the data can use it.
    REALITYDATAPLATFORM_EXPORT Visibility GetVisibility() const;
    REALITYDATAPLATFORM_EXPORT void SetVisibility(Visibility visibility);

    //! Enables setting the visibility by a string. The only valid values are "PUBLIC", "ENTERPRISE", "PERMISSION" and "PRIVATE"
    //! Returns SUCCESS if value set and ERROR if given tag is invalid
    REALITYDATAPLATFORM_EXPORT Utf8String GetVisibilityTag() const;
    REALITYDATAPLATFORM_EXPORT StatusInt SetVisibilityByTag(Utf8CP visibility);

    //! Static helper method: converts a visibility tag to a visibility
    //! Returns SUCCESS if value valid and ERROR if given tag is invalid. In case of error the visibility value remains unchanged
    REALITYDATAPLATFORM_EXPORT static StatusInt GetVisibilityFromTag(Visibility& returnedVisibility, Utf8CP visibilityTag);
    REALITYDATAPLATFORM_EXPORT static Utf8String GetTagFromVisibility(Visibility returnedVisibility);

    //! Get/Set
    //! Key to the dataset. Enables grouping of multiple results on the client side. 
    //! Can also be used to discriminate exactly like the DataProvider key. 
    //! Currently used keys are ‘CDEM’, ‘NAIP’, ‘SRTM1’, ‘SRTM3’, 'Landsat 8', 'GeoGratis'
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;
    REALITYDATAPLATFORM_EXPORT void SetDataset(Utf8CP dataset);

    //! Get/Set
    //! The footprint of the spatial entity. A list of points in longitude/latitude pairs that define the boundary of the data.
    //! The path defined by the given polygon must not auto-cross, contains segments that overlap.
    //! The final closing point is mandatory.
    REALITYDATAPLATFORM_EXPORT const bvector<GeoPoint2d>& GetFootprint() const;
    REALITYDATAPLATFORM_EXPORT virtual void SetFootprint(bvector<GeoPoint2d> const& footprint, Utf8String coordSys = "4326");
    REALITYDATAPLATFORM_EXPORT virtual Utf8String GetFootprintString() const;
    REALITYDATAPLATFORM_EXPORT void SetFootprintString(Utf8CP footprint);

    //! Get/Set
    //! Indicates if the footprint is approximate or not. A typical approximate footprint 
    //! is the result of a raster containing a border as a result of a rotated image. 
    //! The footprint contains the extent of the raster including this border of non-information.
    REALITYDATAPLATFORM_EXPORT bool HasApproximateFootprint() const;
    REALITYDATAPLATFORM_EXPORT void SetApproximateFootprint(bool approximateFootprint);

    //! Returns the footprint extent. This range includes the footprint specified by calling SetFootprint.
    //! If the footprint has not been set an empty range with all zero bounds is returned.
    REALITYDATAPLATFORM_EXPORT DRange2dCR GetFootprintExtent() const;

    //! Textual description of the data
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);
    
    //! Helper static methods. These convert or interpret string formatted footprints in the GCS (Context Services) format
    REALITYDATAPLATFORM_EXPORT static Utf8String FootprintToGCSString(bvector<GeoPoint2d> footprint, Utf8String coordSys);
    REALITYDATAPLATFORM_EXPORT static bvector<GeoPoint2d> GCSStringToFootprint(Utf8String footprint, Utf8String& coordSys);

    //! Helper static methods. These convert or interpret string formatted footprints in the RDS (ProjectWise Context Share) format
    REALITYDATAPLATFORM_EXPORT static Utf8String FootprintToRDSString(bvector<GeoPoint2d> footprint, Utf8String coordSys);
    REALITYDATAPLATFORM_EXPORT static bvector<GeoPoint2d> RDSJSONToFootprint(const Json::Value& footprintJson, Utf8String& coordSys);

protected:
    RealityDataBase();
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataBase(){}

    Utf8String m_identifier;
    Utf8String m_name;
    Utf8String m_resolution;
    mutable bool m_resolutionValueUpToDate;
    mutable double m_resolutionValue;
    Utf8String m_accuracy;
    mutable bool m_accuracyValueUpToDate;
    mutable double m_accuracyValue;
    Classification m_classification;
    Utf8String m_dataset;
    bvector<GeoPoint2d> m_footprint;
    mutable Utf8String m_footprintString;
    mutable DRange2d m_footprintExtent;
    mutable bool m_footprintExtentComputed;
    Utf8String m_description;
    bool m_approximateFootprint;

    mutable Utf8String m_coordSys;

    Visibility m_visibility;
    }; 


enum class RealityDataField
    {
    Id,
    OrganizationId,
    ContainerName,
    DataLocationGuid,
    Name,
    Dataset,
    Description,
    RootDocument,
    Size,
    Classification,
    Type,
    Streamed,
    Footprint,
    ThumbnailDocument,
    MetadataUrl,
    UltimateId,
    UltimateSite,
    Copyright,
    TermsOfUse,
    ResolutionInMeters,
    AccuracyInMeters,
    Visibility,
    Listable,
    CreatedTimestamp,
    ModifiedTimestamp,
    LastAccessedTimestamp,
    OwnedBy,
    CreatorId,
    Group,
    Hidden,
    DelegatePermissions,
    ApproximateFootprint
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! The central class of the Reality Data model. It represents a spatial
//! geocoordinated data meant to represent reality modeling data. 
//! It contains all fields necessary to represent the spatial
//! entity except the actual data which is stored in the.
//=====================================================================================
struct RealityData : public RealityDataBase
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Create();

    // Creator for spatio temporal selector ... fills out some of the most basic fields
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Create(Utf8StringCR identifier, const DateTime& date, Utf8StringCR resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name = "", Utf8StringCR coordSys = "4326");

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetRealityDataType(Utf8CP realityDataType);

    //! [Optional | default is true] Indicates if the reality data is meant to be streamed or if the data set is meant to be downloaded to use.  
    REALITYDATAPLATFORM_EXPORT bool IsStreamed() const;
    REALITYDATAPLATFORM_EXPORT void SetStreamed(bool streamed);

    //! Get/Set
    //! URL to the thumbnail
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetThumbnailDocument() const;
    REALITYDATAPLATFORM_EXPORT void SetThumbnailDocument(Utf8CP thumbnailDocument);

    //! Get/Set
    //! The approximate file size is not stored in data model but returned as an indication 
    //! based on the file size of the first data source. Informative field only.
    REALITYDATAPLATFORM_EXPORT uint64_t GetTotalSize() const; // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetTotalSize(uint64_t size); // in bytes.

    //! Get/Set
    //! The date of the production of the data or an invalid date if it is unknown.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetCreationDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetCreationDateTime(DateTimeCR creationDateTime);

    //! Indicates the last time the data was read or modified.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastAccessedDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetLastAccessedDateTime(DateTimeCR lastAccessedDateTime);

    //! The last modified time
    REALITYDATAPLATFORM_EXPORT DateTime GetModifiedDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetModifiedDateTime(DateTime modifiedDate);

    //! Get/Set
    //! The id of the organization the data belongs to
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOrganizationId() const;
    REALITYDATAPLATFORM_EXPORT void SetOrganizationId(Utf8CP organizationId);

    //! [RDS Specific] The name of the RDS container containing the data
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContainerName() const;
    REALITYDATAPLATFORM_EXPORT void SetContainerName(Utf8CP containerName);

    //! [RDS Specific] The GUID of the Data Location. This represents the identifier
    //! of the Azure Data Center reserved in the Data Location registry in CONNECT
    //! When set it shall be a valid known data location guid.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataLocationGuid() const;
    REALITYDATAPLATFORM_EXPORT void SetDataLocationGuid(Utf8CP dataLocationGuid);

    //! Given the data can be accessed by a root document , the location of this document
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRootDocument() const;
    REALITYDATAPLATFORM_EXPORT void SetRootDocument(Utf8CP rootDocument);

    //! Indicates if the data can be listed. For data containing hundred of thousand of
    //! components it is advisable to set not-listable. The default is listable.
    REALITYDATAPLATFORM_EXPORT bool IsListable() const;
    REALITYDATAPLATFORM_EXPORT void SetListable(bool listable);

    //! A string indicating the owner. The ownership here is different than
    //! Copyright owner. Here we expect the mail address of a CONNECT user OR
    //! A list of semicolon separated such CONNECT User or another
    //! format understandable by the containing service.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOwner() const;
    REALITYDATAPLATFORM_EXPORT void SetOwner(Utf8CP owner);

    //! A string indicating the user id of the creator.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCreatorId() const;
    REALITYDATAPLATFORM_EXPORT void SetCreatorId(Utf8CP creatorId);

    //! Get/Set
    //! A reference to a meta-data object. This object can be used by many spatial entities.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadataUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetMetadataUrl(Utf8CP metadataUrl);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateId() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateId(Utf8CP ultimateId);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateSite() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateSite(Utf8CP ultimateSite);
    
    //! A text indicating the copyright to the data or a URL to a web page containing 
    //! information. The text can be a mix of text and URL. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCopyright() const;
    REALITYDATAPLATFORM_EXPORT void SetCopyright(Utf8CP copyright);

    //! If known or different from Legal field, a text indicating the terms of use applicable to the data
    //! or a URL to a web page containing the information. The text can be a mix of text and URL. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetTermsOfUse() const;
    REALITYDATAPLATFORM_EXPORT void SetTermsOfUse(Utf8CP termsOfUse);
  

    //! The Group code. This field can be used to group many data together if another key other than the dataset is required.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGroup() const;
    REALITYDATAPLATFORM_EXPORT void SetGroup(Utf8CP group);

    REALITYDATAPLATFORM_EXPORT Utf8String GetFootprintString() const override;
    
    //! Indicates if the data is hidden.
    REALITYDATAPLATFORM_EXPORT bool IsHidden() const;
    REALITYDATAPLATFORM_EXPORT void SetHidden(bool hidden);

    //! Indicates if the data has delegated permissions.
    REALITYDATAPLATFORM_EXPORT bool HasDelegatePermissions() const;
    REALITYDATAPLATFORM_EXPORT void SetDelegatePermissions(bool dp);

protected:
    REALITYDATAPLATFORM_EXPORT RealityData();
    REALITYDATAPLATFORM_EXPORT virtual ~RealityData(){}

    Utf8String  m_realityDataType;
    bool        m_streamed;
    DateTime    m_creationDate;
    DateTime    m_modifiedDate;

    Utf8String  m_thumbnailDocument;

    Utf8String  m_organizationId;
    Utf8String  m_containerName;
    Utf8String  m_dataLocationGuid;
    Utf8String  m_rootDocument;
    Utf8String  m_metadataUrl;
    Utf8String  m_ultimateId;
    Utf8String  m_ultimateSite;
    Utf8String  m_copyright;
    Utf8String  m_termsOfUse;

    bool        m_listable;

    Utf8String  m_owner;
    Utf8String  m_creatorId;
    Utf8String  m_group;
    uint64_t    m_totalSize;

    bool        m_hidden;
    bool        m_delegatePermissions;
    DateTime    m_lastAccessedDate;
    };

struct RealityDataExtended: public RealityData
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static RealityDataExtendedPtr Create();

    //! OriginService
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOriginService() const;
    REALITYDATAPLATFORM_EXPORT void SetOriginService(Utf8CP originService);

    //! PermissionOverride
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUsePermissionOverride() const;
    REALITYDATAPLATFORM_EXPORT void SetUsePermissionOverride(Utf8CP usePermissionOverride);

    //! ManagePermissionOverride
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetManagePermissionOverride() const;
    REALITYDATAPLATFORM_EXPORT void SetManagePermissionOverride(Utf8CP managePermissionOverride);

    //! AssignPermissionOverride
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAssignPermissionOverride() const;
    REALITYDATAPLATFORM_EXPORT void SetAssignPermissionOverride(Utf8CP assignPermissionOverride);
protected:
    REALITYDATAPLATFORM_EXPORT RealityDataExtended();

    Utf8String  m_originService;
    Utf8String  m_usePermissionOverride;
    Utf8String  m_managePermissionOverride;
    Utf8String  m_assignPermissionOverride;
    };

//! The DataLocation identifier represents locations where data can be stored.
//! The data location presently represents Microsoft Azure Blob Storages yet 
//! The structure of the class allows for additional storage types given the keys are
//! registered and custom service is applied.
struct RealityDataLocation
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT RealityDataLocation();

    //! Copy constructor and assignment operator.
    REALITYDATAPLATFORM_EXPORT RealityDataLocation(const RealityDataLocation& object);
    REALITYDATAPLATFORM_EXPORT RealityDataLocation& operator=(const RealityDataLocation& object);

    //! Identifier: This identifier to the CONNECT Data Locations registry identifier.
    //! This is the GUID of the data location in the data location registry that 
    //! uniquely identifies a data center. This id enables to extract the original information from which
    //! Provider and Location were extracted. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetIdentifier() const;
    REALITYDATAPLATFORM_EXPORT void SetIdentifier(Utf8CP identifier);

    //! Provider: The name of the provider (ex: Microsoft, Amazon,...)
    //! Currently known key is Microsoft that implies the data location is located
    //! in the Azure cloud blob storage.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
    REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

    //! Location: Geographical Location of Data Location (ex: East Japan)
    //! This is a human readable text that may be used as a seed for translation
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLocation() const;
    REALITYDATAPLATFORM_EXPORT void SetLocation(Utf8CP location);

protected:
    Utf8String m_identifier;
    Utf8String m_provider;
    Utf8String m_location;
    };

struct RealityDataEnterpriseStat
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT RealityDataEnterpriseStat();

    //! Copy constructor and assignment operator.
    REALITYDATAPLATFORM_EXPORT RealityDataEnterpriseStat(const RealityDataEnterpriseStat& object);
    REALITYDATAPLATFORM_EXPORT RealityDataEnterpriseStat& operator=(const RealityDataEnterpriseStat& object);

    //! number of entries belonging to the organization
    REALITYDATAPLATFORM_EXPORT uint64_t GetNbRealityData() const;
    REALITYDATAPLATFORM_EXPORT void SetNbRealityData(uint64_t nbRealityData);

    //! total size of the organizations data (at time of last calculation)
    REALITYDATAPLATFORM_EXPORT uint64_t GetTotalSizeKB() const;
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(uint64_t totalSizeKB);
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(double totalSizeKB);

    //! UltimateId 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOrganizationId() const;
    REALITYDATAPLATFORM_EXPORT void SetOrganizationId(Utf8CP organizationId);

    //! UltimateId 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateId() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateId(Utf8CP ultimateId);

    //! UltimateSite
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateSite() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateSite(Utf8CP ultimateSite);

    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

protected:
    uint64_t   m_nbRealityData;
    uint64_t   m_totalSizeKB;
    Utf8String m_organizationId;
    Utf8String m_ultimateId;
    Utf8String m_ultimateSite;
    DateTime   m_date;
    };
	
struct RealityDataServiceStat
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT RealityDataServiceStat();

    //! Copy constructor and assignment operator.
    REALITYDATAPLATFORM_EXPORT RealityDataServiceStat(const RealityDataServiceStat& object);
    REALITYDATAPLATFORM_EXPORT RealityDataServiceStat& operator=(const RealityDataServiceStat& object);

    //! number of entries belonging to the organization
    REALITYDATAPLATFORM_EXPORT uint64_t GetNbRealityData() const;
    REALITYDATAPLATFORM_EXPORT void SetNbRealityData(uint64_t nbRealityData);

    //! total size of the organizations data (at time of last calculation)
    REALITYDATAPLATFORM_EXPORT uint64_t GetTotalSizeKB() const;
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(uint64_t totalSizeKB);
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(double totalSizeKB);

    //! UltimateId 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateId() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateId(Utf8CP ultimateId);


    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);
	
    //! Service ID
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServiceId() const;
    REALITYDATAPLATFORM_EXPORT void SetServiceId(Utf8CP originService);	
		

protected:
    uint64_t   m_nbRealityData;
    uint64_t   m_totalSizeKB;
    Utf8String m_ultimateId;
    Utf8String m_serviceId;
    DateTime   m_date;
    };	
   
struct RealityDataUserStat
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT RealityDataUserStat();

    //! Copy constructor and assignment operator.
    REALITYDATAPLATFORM_EXPORT RealityDataUserStat(const RealityDataUserStat& object);
    REALITYDATAPLATFORM_EXPORT RealityDataUserStat& operator=(const RealityDataUserStat& object);

    //! number of entries belonging to the organization
    REALITYDATAPLATFORM_EXPORT uint64_t GetNbRealityData() const;
    REALITYDATAPLATFORM_EXPORT void SetNbRealityData(uint64_t nbRealityData);

    //! total size of the organizations data (at time of last calculation)
    REALITYDATAPLATFORM_EXPORT uint64_t GetTotalSizeKB() const;
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(uint64_t totalSizeKB);
    REALITYDATAPLATFORM_EXPORT void SetTotalSizeKB(double totalSizeKB);

    //! UserId 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUserId() const;
    REALITYDATAPLATFORM_EXPORT void SetUserId(Utf8CP userId);
	
    //! User Mail
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUserEmail() const;
    REALITYDATAPLATFORM_EXPORT void SetUserEmail(Utf8CP userEmail);
	
    //! UltimateId 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateId() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateId(Utf8CP ultimateId);

    //! Data Location GUID
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataLocationGuid() const;
    REALITYDATAPLATFORM_EXPORT void SetDataLocationGuid(Utf8CP dataLocationGuid);
	
    //! Service ID
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServiceId() const;
    REALITYDATAPLATFORM_EXPORT void SetServiceId(Utf8CP originService);	
	
	//! Date
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

protected:
    uint64_t   m_nbRealityData;
    uint64_t   m_totalSizeKB;
    Utf8String m_userId;
    Utf8String m_userEmail;	
    Utf8String m_ultimateId;
    Utf8String m_dataLocationGuid;
	Utf8String m_serviceId;
    DateTime   m_date;
    };   

struct RealityDataPublicKey
    {
public:
    //! Create invalid public Key.
    REALITYDATAPLATFORM_EXPORT RealityDataPublicKey();

    //! Copy constructor and assignment operator.
    REALITYDATAPLATFORM_EXPORT RealityDataPublicKey(const RealityDataPublicKey& object);
    REALITYDATAPLATFORM_EXPORT RealityDataPublicKey& operator=(const RealityDataPublicKey& object);

    //! Identifier of the public key. This identifier is of the format 'PK-' followed
    //! by a GUID. This identifier refers to a public key stored in the server and defining
    //! the reality data and access rights and limitations.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetIdentifier() const;
    REALITYDATAPLATFORM_EXPORT void SetIdentifier(Utf8CP identifier);

    //! The reality data referenced by the public key.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() const;
    REALITYDATAPLATFORM_EXPORT void SetRealityDataId(Utf8StringCR realityDataId);

    //! Description: This description enables to describe to users the intent of the key
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);
	
    //! User Id. The identifier of the owner of the public key.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUserId() const;
    REALITYDATAPLATFORM_EXPORT void SetUserId(Utf8CP userId);
	
    //! UltimateId of the enterprise owning the reality data referenced.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUltimateId() const;
    REALITYDATAPLATFORM_EXPORT void SetUltimateId(Utf8CP ultimateId);

    //! Get/Set
    //! The date of the creation of the public key.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetCreationDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetCreationDateTime(DateTimeCR creationDateTime);

    //! The last modified time
    REALITYDATAPLATFORM_EXPORT DateTime GetModifiedDateTime() const;
    REALITYDATAPLATFORM_EXPORT void SetModifiedDateTime(DateTime modifiedDate);

    //! Authorized User Ids: The collection of authorized user Ids (GUIDs) for 
    //! read only access, separated by semicolon (;). If empty, all users allowed.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAuthorizedUserIds() const;
    REALITYDATAPLATFORM_EXPORT void SetAuthorizedUserIds(Utf8CP authorizedUserIds);

    //! Valid Until Date. The date after which the public key is no longer valid. 
    //! This validity date cannot be further than 3 months from current date
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetValidUntilDate() const;
    REALITYDATAPLATFORM_EXPORT void SetValidUntilDate(DateTimeCR validUntilDate);


protected:
    Utf8String m_identifier;
    Utf8String m_realityDataId;
    Utf8String m_userId;
    Utf8String m_description;
    Utf8String m_ultimateId;
    DateTime   m_creationDateTime;
    DateTime   m_modifiedDateTime;
    Utf8String m_authorizedUserIds;
    DateTime   m_validUntilDate;
    };   
END_BENTLEY_REALITYPLATFORM_NAMESPACE