/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/ChunkedWSChangeset.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

struct MultiLockFormatter
{
public:
    static void SetToChunkedChangeset
        (
        const Dgn::DgnLockSet&          locks,
        BeSQLite::BeBriefcaseId         briefcaseId,
        BeSQLite::BeGuidCR              seedFileId,
        Utf8StringCR                    releasedWithChangeSetId,
        ChunkedWSChangeset&             chunkedChangeset,
        const WSChangeset::ChangeState& changeState,
        bool                            includeOnlyExclusive = false,
        bool                            queryOnly = false
        );
};

END_BENTLEY_IMODELHUB_NAMESPACE
