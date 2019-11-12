/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/Response/WSObjectsReader.h>
#include <WebServices/iModelHub/Client/Result.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct BridgeProperties> BridgePropertiesPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(BridgeProperties);

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2019
//=======================================================================================
struct BridgeProperties : RefCountedBase
{
private:
    friend struct ChangeSetInfo;
    friend struct ChangeSetFormatter;

    BeSQLite::BeGuid m_jobId;
    bvector<Utf8String> m_changedFiles;
    bvector<Utf8String> m_users;

    BridgeProperties() {}
    BridgeProperties(BeSQLite::BeGuid jobId, bvector<Utf8String> changedFiles, bvector<Utf8String> users) :
        m_jobId(jobId), m_changedFiles(changedFiles), m_users(users) {}

    static BridgePropertiesPtr Parse(RapidJsonValueCR properties);
    static BridgePropertiesPtr ParseFromRelated(WebServices::WSObjectsReader::Instance instance);
    static void FormatRelated(JsonValueR mainInstance, BridgePropertiesPtr bridgeProperties);
    bool IsVectorValid(bvector<Utf8String> vector) const;
public:
    //! Create an instance of bridge properties.
    //! @param[in] jobId
    //! @param[in] changedFiles
    //! @param[in] users
    //! @return Returns a shared pointer to the created instance.
    static BridgePropertiesPtr Create(BeSQLite::BeGuid jobId, bvector<Utf8String> changedFiles, bvector<Utf8String> users)
        { return BridgePropertiesPtr(new BridgeProperties(jobId, changedFiles, users)); }

    BeSQLite::BeGuidCR GetJobId() const { return m_jobId; }
    const bvector<Utf8String> GetChangedFiles() const { return m_changedFiles; }
    const bvector<Utf8String> GetUsers() const { return m_users; }

    bool IsEmpty() { return !m_jobId.IsValid() && m_changedFiles.empty() && m_users.empty(); }

    //! Validates bridge properties instance.
    IMODELHUBCLIENT_EXPORT StatusResult Validate() const;
};
END_BENTLEY_IMODELHUB_NAMESPACE
