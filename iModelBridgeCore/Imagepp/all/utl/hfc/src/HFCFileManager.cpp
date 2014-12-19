//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCFileManager.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HFCFileManager.h>


HFC_IMPLEMENT_SINGLETON(HFCFileManager)

/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
 */
HFCFileManager::HFCFileManager()
    {
    }

/** ---------------------------------------------------------------------------
    Remove file from disk.
    ---------------------------------------------------------------------------
 */
bool HFCFileManager::Remove(const HFCPtr<HFCURLFile>& pi_rpURL)
    {
    bool result = false;

#ifdef _WIN32
    //:> Extract file name from URL
    WString fileName(pi_rpURL->GetHost() + L"\\" +  ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath());

    //:> Remove file
    if( _wremove(fileName.c_str()) != -1 ) //:> Succeeded!
        result = true;
#endif

    return result;
    }

