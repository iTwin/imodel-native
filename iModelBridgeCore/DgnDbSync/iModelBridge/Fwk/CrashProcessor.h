/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwkRegistry.h>
#include <memory>
#include <string>

namespace crashpad { class CrashpadClient; } // from crashpad_client.h
struct _EXCEPTION_POINTERS; // from winnt.h

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     09/2019
//=======================================================================================
struct CrashProcessor final
{
public:
    enum class CommonAnnotation
    {
        JOB_Id,
        JOB_CorrelationId,
        IMH_UserName,
        IMH_RpositoryName,
        IMH_ProjectId
    };

private:
    std::unique_ptr<crashpad::CrashpadClient> m_client;

    // N.B. DO NOT INLINE THESE -- otherwise, it forces all includers to also include the full type definition of CrashpadClient, which also brings in things like windows.h.
    CrashProcessor(); 
    ~CrashProcessor();
    
    static std::string GetSentryReportingUrl(Utf8CP appName);

public:
    // For crashes to upload, appName is expected to be the base of a BUDDI URL of the form appName.SentryMinidump.
    static CrashProcessor& CreateSentryInstance(Utf8CP appName);

    // Only valid after calling a Create...Instance function. The bridge framework does that very early, so you can generally rely on it.
    IMODEL_BRIDGE_FWK_EXPORT static CrashProcessor* GetInstance();

    // An annotation is an arbitrary string key/value pair associated with future crash dumps.
    // Keys and values must each be less than 256 characters. There can be up to 64 annotations.
    // Common keys used by the framework or many bridges exist in the CommonAnnotation enum for convenience.
    // A NULL value will remove the key; multiple calls with the same key will overwrite the value.
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus SetAnnotation(Utf8CP key, Utf8CP val);
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus SetAnnotation(CommonAnnotation, Utf8CP val);

#if defined (BENTLEYCONFIG_OS_WINDOWS)
   void CreateDump(_EXCEPTION_POINTERS*);
#endif
};

END_BENTLEY_NAMESPACE
