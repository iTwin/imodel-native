/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerRevision.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/RevisionManager.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct DgnDbServerRevision> DgnDbServerRevisionPtr;

struct DgnDbServerRevision
    {
//__PUBLISH_SECTION_END__
private:
    DgnRevisionPtr m_revision;
    int64_t m_index;
    Utf8String m_url;

    DgnDbServerRevision(DgnRevisionPtr revision);
//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static DgnDbServerRevisionPtr Create(DgnRevisionPtr revision);
    DGNDBSERVERCLIENT_EXPORT DgnRevisionPtr GetRevision();
    DGNDBSERVERCLIENT_EXPORT int64_t GetIndex();
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetURL();

//__PUBLISH_SECTION_END__
    void SetIndex(int64_t index);
    void SetURL(Utf8StringCR url);
//__PUBLISH_SECTION_START__
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
