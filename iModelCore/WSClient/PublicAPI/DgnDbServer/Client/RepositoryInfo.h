/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/RepositoryInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/FileInfo.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct RepositoryInfo
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_repositoryUrl;
    Utf8String m_description;
    Utf8String m_repositoryId;
    Utf8String m_userUploaded;
    DateTime   m_uploadedDate;
 //__PUBLISH_SECTION_START__
public:
    RepositoryInfo(Utf8StringCR repositoryUrl, Utf8StringCR description, Utf8StringCR repositoryId, Utf8StringCR userUploaded, DateTimeCR uploadedDate);

    Utf8StringCR GetDescription();
    Utf8StringCR GetRepositoryURL();
    Utf8StringCR GetRepositoryId();
    Utf8StringCR GetUserUploaded();
    DateTimeCR   GetUploadedDate();
};

typedef RepositoryInfo& RepositoryInfoR;
typedef const RepositoryInfo& RepositoryInfoCR;
END_BENTLEY_DGNDBSERVER_NAMESPACE
