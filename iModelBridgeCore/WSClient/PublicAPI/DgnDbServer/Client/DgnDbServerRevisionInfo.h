/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerRevisionInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef std::shared_ptr<struct DgnDbServerRevisionInfo> DgnDbServerRevisionInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerRevisionInfo);
DEFINE_TASK_TYPEDEFS(DgnDbServerRevisionInfoPtr, DgnDbServerRevisionInfo);

//=======================================================================================
//! Information about revision.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct DgnDbServerRevisionInfo
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_id;
        Utf8String m_parentRevisionId;
        Utf8String m_dbGuid;
        int64_t    m_index;
        Utf8String m_url;

        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index, Utf8String url);

        //__PUBLISH_SECTION_END__
        bool operator==(DgnDbServerRevisionInfoCR briefcase) const;
        static DgnDbServerRevisionInfoPtr Parse(WSObjectsReader::Instance instance);
        //! DEPRECATED: Use Parse from Instance
        static DgnDbServerRevisionInfoPtr Parse(JsonValueCR json);
        //__PUBLISH_SECTION_START__
        DGNDBSERVERCLIENT_EXPORT Utf8String GetId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetParentRevisionId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDbGuid() const;
        DGNDBSERVERCLIENT_EXPORT int64_t    GetIndex() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetUrl() const;
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
