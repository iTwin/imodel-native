/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbBriefcaseInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbBriefcaseInfo> DgnDbBriefcaseInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbBriefcaseInfo);
DEFINE_TASK_TYPEDEFS(bvector<std::shared_ptr<DgnDbBriefcaseInfo>>, DgnDbBriefcasesInfo);

//=======================================================================================
//! Information about briefcase.
//@bsiclass                                      julius.cepukenas               08/2015
//=======================================================================================
struct DgnDbBriefcaseInfo
    {
    //__PUBLISH_SECTION_END__
    private:
        BeSQLite::BeBriefcaseId m_id;
        Utf8String m_userOwned;
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT DgnDbBriefcaseInfo();
        DGNDBSERVERCLIENT_EXPORT DgnDbBriefcaseInfo(BeSQLite::BeBriefcaseId id);
        DGNDBSERVERCLIENT_EXPORT DgnDbBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned);

        //__PUBLISH_SECTION_END__
        bool operator==(DgnDbBriefcaseInfoCR briefcase) const;
        static DgnDbBriefcaseInfoPtr FromJson(JsonValueCR json);
        //__PUBLISH_SECTION_START__
        DGNDBSERVERCLIENT_EXPORT BeSQLite::BeBriefcaseId GetId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserOwned() const;
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
