/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatformTools/WSGServices.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef std::function<void(const char* pMsg)> WSG_FeedbackFunction;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! ConnectedResponse
//! struct used to hold and return all pertinent elements regarding a server response.
//=====================================================================================
struct ConnectedResponse : public RawServerResponse
    {
public:
    Utf8String simpleMessage;
    bool simpleSuccess;

    //! Simple boolean representation of whether the operation was successful or not
    REALITYDATAPLATFORM_EXPORT bool GetSuccess() { return simpleSuccess; }

    //! Will contain a user-friendly message, explaining what might have gone wrong with the request 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSimpleMessage() { return simpleMessage; }
    REALITYDATAPLATFORM_EXPORT void Clone(const RawServerResponse& raw);
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              10/2017
//! RDSRequestManager
//! Entity that sets common variables for required for multiple requests
//=====================================================================================
struct WSGRequestManager
    {
    REALITYDATAPLATFORM_EXPORT static void SetCallback(WSG_FeedbackFunction piFunc) { s_callback = piFunc; }
    REALITYDATAPLATFORM_EXPORT static void SetErrorCallback(WSG_FeedbackFunction piFunc) { s_errorCallback = piFunc; }
    REALITYDATAPLATFORM_EXPORT static Utf8String MakeBuddiCall(WString serviceName);

protected:
    REALITYDATAPLATFORM_EXPORT static void Report(Utf8String message);
    REALITYDATAPLATFORM_EXPORT static void ReportError(Utf8String message);
    
    static WSG_FeedbackFunction    s_callback;
    static WSG_FeedbackFunction    s_errorCallback;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE