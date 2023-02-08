/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

class ECDbValidationChecks {
public:
    struct CheckResult {
        Utf8String status;
        Utf8String details;
    };

    struct SummarizedResult {
        Utf8String checkName;
        Utf8String status;
    };

    static std::vector<SummarizedResult> PerformAllChecks(ECDbCR ecdb);
    static std::vector<CheckResult> NavigationPropertyIdCheck(ECDbCR ecdb);
    static std::vector<CheckResult> ClassIdCheck(ECDbCR ecdb);
};

END_BENTLEY_SQLITE_EC_NAMESPACE