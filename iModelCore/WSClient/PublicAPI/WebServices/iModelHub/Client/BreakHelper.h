/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>

//__BENTLEY_INTERNAL_ONLY__
BEGIN_BENTLEY_IMODELHUB_NAMESPACE

enum Breakpoints
    { 
    None,
    AfterDownloadChangeSets,
    BeforeStartCreateChangeSet, 
    AfterStartCreateChangeSet,
    BeforePushChangeSetToServer,
    AfterPushChangeSetToServer,
    AfterFinishCreateChangeSet,

    Client_AfterCreateRequest,
    Client_AfterDownloadBriefcaseFile,
    Client_AfterDeleteBriefcase,
    Client_AfterOpenBriefcaseForMerge,
    Client_AfterMergeChangeSets,
    Client_AfterCreateBriefcaseInstance,

    iModelConnection_AfterCreateNewServerFile, 
    iModelConnection_AfterUploadServerFile,
    iModelConnection_AfterWriteiModelInfo,
    iModelConnection_AfterCreateChangeSetRequest,
    iModelConnection_AfterUploadChangeSetFile
    };

//=======================================================================================
// @bsiclass                                    Algirdas.Mikoliunas             10/2016
//=======================================================================================
struct BreakHelper
{
private:
    static Breakpoints s_breakpoint;
public:
    //! Sets breakpoint, that should throw exception during execution
    //! @param[in] breakpoint
    IMODELHUBCLIENT_EXPORT static void SetBreakpoint(Breakpoints breakpoint);
    //! Hits breakpoint and throws exception if set breakpoint is equal to current
    //! @param[in] breakpoint
    IMODELHUBCLIENT_EXPORT static void HitBreakpoint(Breakpoints breakpoint);
};
END_BENTLEY_IMODELHUB_NAMESPACE
