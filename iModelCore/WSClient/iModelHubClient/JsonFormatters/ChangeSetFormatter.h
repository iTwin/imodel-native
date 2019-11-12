/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/ChangeSetArguments.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
// @bsiclass                                      Algirdas.Mikoliunas            10/2019
//=======================================================================================
struct ChangeSetFormatter
{
public:
    static Json::Value Format
        (
        Dgn::DgnRevisionPtr       changeSet,
        BeSQLite::BeBriefcaseId   briefcaseId,
        PushChangeSetArgumentsPtr pushArguments
        );
};

END_BENTLEY_IMODELHUB_NAMESPACE
