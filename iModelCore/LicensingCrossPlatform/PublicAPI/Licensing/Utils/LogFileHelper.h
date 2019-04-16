#pragma once
/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <regex>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LogFileHelper
{
public:
    LICENSING_EXPORT bvector<WString> GetLogFiles(Utf8StringCR logFilesDir);
};

END_BENTLEY_LICENSING_NAMESPACE
