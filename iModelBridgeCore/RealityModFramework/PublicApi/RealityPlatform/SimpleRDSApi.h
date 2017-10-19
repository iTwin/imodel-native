#include "WSGServices.h"
#include "RealityDataService.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef std::function<void(const char* pMsg)> RDS_FeedbackFunction;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedResponse
//! struct used to hold and return all pertinent elements regarding a curl response.
//=====================================================================================
struct ConnectedResponse : public RawServerResponse
    { 
public:
    Utf8String simpleMessage;
    bool simpleSuccess;

    REALITYDATAPLATFORM_EXPORT bool GetSuccess()               { return simpleSuccess; }
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSimpleMessage() { return simpleMessage; }
    REALITYDATAPLATFORM_EXPORT void Clone(const RawServerResponse& raw);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! RDSRequestManager
//! Entity that sets common variables for required for multiple requests
//=====================================================================================
struct RDSRequestManager
    {    
    REALITYDATAPLATFORM_EXPORT void SetCallback(RDS_FeedbackFunction piFunc) { m_callback = piFunc; }
    REALITYDATAPLATFORM_EXPORT void SetErrorCallback(RDS_FeedbackFunction piFunc) { m_errorCallback = piFunc; }

    REALITYDATAPLATFORM_EXPORT static RDSRequestManager& GetInstance();

    void Report(Utf8String message);
    void ReportError(Utf8String message);

private:
    static RDSRequestManager*      s_instance;
    RDSRequestManager();
    RDSRequestManager(RDS_FeedbackFunction callbackFunction = nullptr, RDS_FeedbackFunction errorCallback = nullptr);

    Utf8String MakeBuddiCall();
    RDS_FeedbackFunction    m_callback;
    RDS_FeedbackFunction    m_errorCallback;
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
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetStats();
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetAllStats(bvector<ConnectedRealityDataEnterpriseStat>& statVec);
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataEnterpriseStat(const RealityDataEnterpriseStat& stat);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(const RealityDataEnterpriseStat& stat);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedRealityDataProjectRelationship
//! Extends RealityDataProjectRelationship, directly integrating requests to describe, 
//! create, upload, download, and delete entries on the server
//=====================================================================================
struct ConnectedRealityDataProjectRelationship: public RealityDataProjectRelationship
    {
public:
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForRDId(bvector<ConnectedRealityDataProjectRelationshipPtr>& relationshipVector);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForProjectId(bvector<ConnectedRealityDataProjectRelationshipPtr>& relationshipVector);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Create();
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityDataProjectRelationship(RealityDataProjectRelationshipPtr relationship);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataProjectRelationshipPtr relationship);
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
    //REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForRealityData(bvector<ConnectedRealityDataDocumentPtr>& docVector);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();
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
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();
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
    REALITYDATAPLATFORM_EXPORT ConnectedResponse RetrieveAllForUltimateId(bvector<ConnectedRealityDataPtr>& dataVector);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse GetInfo();
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Upload(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Download(BeFileName filePath, Utf8String serverPath);
    REALITYDATAPLATFORM_EXPORT ConnectedResponse Delete();
    REALITYDATAPLATFORM_EXPORT ConnectedRealityData(RealityDataPtr realityData);
private:
    REALITYDATAPLATFORM_EXPORT void Clone(RealityDataPtr realityData);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE