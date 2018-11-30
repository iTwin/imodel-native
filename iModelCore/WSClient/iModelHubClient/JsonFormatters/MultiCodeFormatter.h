/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/JsonFormatters/MultiCodeFormatter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/ChunkedWSChangeset.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

struct MultiCodeFormatter
{
public:
    static void SetToChunkedChangeset
        (
        const Dgn::DgnCodeSet           codes,
        Dgn::DgnCodeState               state,
        BeSQLite::BeBriefcaseId         briefcaseId,
        ChunkedWSChangeset&             chunkedChangeset,
        const WSChangeset::ChangeState& changeState,
        bool                            queryOnly = false
        );

    static ObjectId GetCodeId (Dgn::DgnCodeCR code, const BeSQLite::BeBriefcaseId briefcaseId);
};

END_BENTLEY_IMODELHUB_NAMESPACE
