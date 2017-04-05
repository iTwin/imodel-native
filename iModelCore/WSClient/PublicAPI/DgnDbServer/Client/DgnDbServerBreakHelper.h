/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerBreakHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>

//__BENTLEY_INTERNAL_ONLY__
BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

enum DgnDbServerBreakpoints
    { 
    None,
    AfterDownloadRevisions,
    BeforeStartCreateRevision, 
    AfterStartCreateRevision,
    BeforePushRevisionToServer,
    AfterPushRevisionToServer,
    AfterFinishCreateRevision,

    DgnDbClient_AfterCreateRequest,
    DgnDbClient_AfterDownloadBriefcaseFile,
    DgnDbClient_AfterDeleteBriefcase,
    DgnDbClient_AfterOpenBriefcaseForMerge,
    DgnDbClient_AfterMergeRevisions,
    DgnDbClient_AfterCreateBriefcaseInstance,

    DgnDbRepositoryConnection_AfterCreateNewServerFile, 
    DgnDbRepositoryConnection_AfterUploadServerFile,
    DgnDbRepositoryConnection_AfterWriteRepositoryInfo,
    DgnDbRepositoryConnection_AfterCreateRevisionRequest,
    DgnDbRepositoryConnection_AfterUploadRevisionFile
    };

//=======================================================================================
// @bsiclass                                    Algirdas.Mikoliunas             10/2016
//=======================================================================================
struct DgnDbServerBreakHelper
{
private:
    static DgnDbServerBreakpoints s_breakpoint;
public:
    //! Sets breakpoint, that should throw exception during execution
    //! @param[in] breakpoint
    DGNDBSERVERCLIENT_EXPORT static void SetBreakpoint(DgnDbServerBreakpoints breakpoint);
    //! Hits breakpoint and throws exception if set breakpoint is equal to current
    //! @param[in] breakpoint
    DGNDBSERVERCLIENT_EXPORT static void HitBreakpoint(DgnDbServerBreakpoints breakpoint);
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
