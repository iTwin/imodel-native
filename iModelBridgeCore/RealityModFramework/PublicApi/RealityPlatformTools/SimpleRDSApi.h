/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatformTools/SimpleWSGBase.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! RDSRequestManager
//! Entity that sets common variables for required for multiple requests
//=====================================================================================
struct RDSRequestManager : public WSGRequestManager
    {    
    //! Sets all necessary parameters to complete calls
    //! the serverUrl can be provider by the user, if it is known
    //! otherwise (if the ConnectionClient is available) the serverUrl will be determined automatically
    //! serverUrl format example "https://connect-realitydataservices.bentley.com/"
    REALITYDATAPLATFORM_EXPORT static void Setup(Utf8String serverUrl = "");
    };

struct ConnectedNavNode : public NavNode
    {
    //! Gets all root nodes accessible to the user
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse GetRootNodes(bvector<ConnectedNavNode>& nodes);

    //! Gets all nodes beneath this one
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetChildNodes(bvector<ConnectedNavNode>& children);
    REALITYDATAPLATFORM_EXPORT ConnectedNavNode(const NavNode& node);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const NavNode& stat);
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              05/2018
//! ConnectedRealityDataLocation
//! Extends DataLocation, directly integrating requests to get stats 
//! from the server
//=====================================================================================
struct ConnectedRealityDataLocation : public RealityDataLocation
    {
public:


    //! Server request
    //! Will retrieve all reality data entities for a given ultimateId
    //! If no ultimateId is provided, your ultimateId will be extracted from your token
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse RetrieveAllDataLocations(bvector<ConnectedRealityDataLocation>& dataLocations);


    //! Gets stats for the user's enterprise
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetDataLocation();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataLocation(Utf8String guid);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataLocation(const ConnectedRealityDataLocation& location);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataLocation(const RealityDataLocation& location);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const RealityDataLocation& location);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataLocation() : RealityDataLocation(){}

    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataEnterpriseStat
//! Extends RealityDataEnterpriseStat, directly integrating requests to get stats 
//! from the server
//=====================================================================================
struct ConnectedRealityDataEnterpriseStat : public RealityDataEnterpriseStat
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataEnterpriseStat() : RealityDataEnterpriseStat(){}

    //! Gets stats for the user's enterprise
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetEnterpriseStats();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataEnterpriseStat(const RealityDataEnterpriseStat& stat);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const RealityDataEnterpriseStat& stat);
    //REALITYDATAPLATFORM_EXPORT static ConnectedResponse GetAllStats(bvector<ConnectedRealityDataEnterpriseStat>& statVec);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataServiceStat
//! Extends RealityDataServiceStat, directly integrating requests to get stats 
//! from the server
//=====================================================================================
struct ConnectedRealityDataServiceStat : public RealityDataServiceStat
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataServiceStat() : RealityDataServiceStat(){}

    //! Gets stats for the user's enterprise
//    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetServiceStats();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataServiceStat(const RealityDataServiceStat& stat);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const RealityDataServiceStat& stat);
    };
	
//=====================================================================================
//! @bsiclass                                   Alain.RObert             04/2018
//! ConnectedRealityDataUserStat
//! Extends RealityDataUserStat, directly integrating requests to get stats 
//! from the server
//=====================================================================================
struct ConnectedRealityDataUserStat : public RealityDataUserStat
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataUserStat() : RealityDataUserStat(){}

    //! Gets stats for the user's enterprise
//     REALITYDATAPLATFORM_EXPORT ConnectedResponse GetUserStats();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataUserStat(const RealityDataUserStat& stat);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const RealityDataUserStat& stat);
    };
	
//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataRelationship
//! Extends RealityDataRelationship, directly integrating requests to describe, 
//! create, upload, download, and delete entries on the server
//=====================================================================================
struct ConnectedRealityDataRelationship: public RealityDataRelationship
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataRelationship() : RealityDataRelationship() {}

    //! Server request
    //! Will retrieve all relationships for a reality data
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse RetrieveAllForRDId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector, Utf8String rdId);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForRDId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector);
    
    //! Server request
    //! Will retrieve all relationships for a project
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse RetrieveAllForProjectId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector, Utf8String projectId);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForProjectId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector);
    
    //! Server request
    //! Will create a relationship between the realitydata and project set in this object
    REALITYDATAPLATFORM_EXPORT ConnectedResponse CreateOnServer();

    //! Server request
    //! Will delete a relationship between the realitydata and project set in this object
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();

    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataRelationship(RealityDataRelationshipPtr relationship);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataRelationshipPtr relationship);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataDocument
//! Extends RealityDataDocument, directly integrating requests to describe, 
//! upload, download, and delete entries on the server
//=====================================================================================
struct ConnectedRealityDataDocument : public RealityDataDocument
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataDocument() : RealityDataDocument(){}

    //! Server request
    //! Will retrieve server paths for every document contained withing a reality data
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse RetrieveAllForRealityData(bvector<bpair<Utf8String, uint64_t>>& docVector, Utf8String realityDataGUID);

    //! Server request
    //! Will fill this object with all relevant properties found on the server
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();

    //! Server request
    //! Will upload the document to the server. Ideally used to replace an existing document, in a reality data
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8String serverPath, bool overwrite = true, bool listable = true);

    //! Server request
    //! Will download this document to the file path specified
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);

    //! Server request
    //! Deletes the document from the server
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();

    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataDocument(Utf8String navString);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataDocument(RealityDataDocumentPtr doc);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataDocumentPtr doc);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataFolder
//! Extends RealityDataFolder, directly integrating requests to describe, 
//! upload, download, and delete entries on the server
//=====================================================================================
struct ConnectedRealityDataFolder : public RealityDataFolder
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataFolder() : RealityDataFolder() {}

    //! Server request
    //! Will fill this object with all relevant properties found on the server
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();

    //! Server request
    //! Will upload everything contained in the folder (at the specified file path) to the server.
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8String serverPath, bool overwrite = true, bool listable = true);

    //! Server request
    //! Will download everything contained in the folder (at the specified file path) to the server.
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);

    //! Server request
    //! Will delete everything contained in the folder on the server.
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();

    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataFolder(Utf8String navString);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataFolder(RealityDataFolderPtr folder);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataFolderPtr folder);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityData
//! Extends RealityData, directly integrating requests to list, describe, 
//! upload, download, and delete entries on the server
//=====================================================================================
struct ConnectedRealityData : public RealityData
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedRealityData() : RealityData() {}

    //! Server request
    //! Will retrieve all reality data entities for a given ultimateId
    //! If no ultimateId is provided, your ultimateId will be extracted from your token
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse RetrieveAllForUltimateId(bvector<ConnectedRealityDataPtr>& dataVector, Utf8String ultimateId);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForUltimateId(bvector<ConnectedRealityDataPtr>& dataVector);

    //! Server request
    //! Will retrieve all info for the reality data 
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();

    //! Server request
    //! Will update the reality data on the server, with the info in the object
    REALITYDATAPLATFORM_EXPORT ConnectedResponse UpdateInfo();

    //! Server request
    //! Will upload the reality data at the specified file path, to the specified server path (GUID)
    //! If serverPath is an empty string, a new reality data entry will be created on the server
    //! and the auto-generated guid will be assigned to the serverPath variable
    //! If a new entry is created, any properties set on this object will be set on the server
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8StringR serverPath, bool overwrite = true, bool listable = true);

    //! Server request
    //! Will download the reality data to the specified file path, from the specified server path (GUID)
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);

    //! Server request
    //! Will delete the reality data at the specified server path (GUID) (if defined)
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();

    REALITYDATAPLATFORM_EXPORT ConnectedRealityData(Utf8String guid);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityData(RealityDataPtr realityData);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataPtr realityData);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE