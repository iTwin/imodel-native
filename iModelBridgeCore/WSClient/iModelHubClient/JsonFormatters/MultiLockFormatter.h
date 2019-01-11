/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/JsonFormatters/MultiLockFormatter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
