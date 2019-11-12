/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CrashProcessor.h"
#include <WebServices/Configuration/BuddiClient.h>

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// folly and crashpad conflict over CHECK, DCHECK, and PCHECK macros,
// so move code that needs folly into a separate CPP file.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
    N.B.

    - Be careful adding assert calls. Unit tests cannot deliver the
    required handler .exe, so this code needs to fail gracefully (and not
    needlessly break tests).

    - Use fprintf(stderr, ...) instead of logging because crash reporting
    may be initialized /before/ logging.
*/
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_WEBSERVICES

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
std::string CrashProcessor::GetSentryReportingUrl(Utf8CP appName)
    {
    // Not configured? Don't even try...
    if (Utf8String::IsNullOrEmpty(appName))
        {
        fprintf(stderr, "No appName provided, so cannot upload crash reports.\n");
        return "";
        }
    
    Utf8String urlName(appName);
    urlName.append(".SentryMinidump");
    
    BuddiClient buddi;
    
    // Every bridge can already provide an appName, but uploading will be phased in... don't fail annoyingly if there's no URL yet.
    BuddiUrlResult result = buddi.GetUrl(urlName.c_str())->GetResult();
    if (!result.IsSuccess())
        {
        fprintf(stderr, "Could not resolve crash report upload URL from BUDDI for appName %s, so cannot upload crash reports.\n", appName);
        return "";
        }

    return std::string(result.GetValue().c_str());
    }
