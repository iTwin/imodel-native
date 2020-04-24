//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFLocalCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the stretcher implementation
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"

#include "HRFCacheFileCreator.h"

//-----------------------------------------------------------------------------
// This is a helper class to instantiate a cache file object
// without knowing the different cache file format.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFLocalCacheFileCreator :  public HRFCacheFileCreator
    {
public:
    // Destructor
    virtual ~HRFLocalCacheFileCreator();

    // Sets tags to the file and verifies if the cache is valid
    static void                     SetCacheTags(HFCPtr<HRFRasterFile>& pi_rpFile);
    static bool                     IsValidCache(const HFCPtr<HRFRasterFile>& pi_rpFile);
    HFCPtr<HFCURL>                  GetTentativeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName,
                                                       const Utf8String&        pi_Extension,
                                                       uint64_t             pi_Offset = 0) const;

    HFCPtr<HFCURL>                  ComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName,
                                                  const Utf8String&        pi_Extension,
                                                  uint64_t             pi_Offset = 0) const;

    HFCPtr<HFCURL>                  ComposeURLFor_longFilename(const HFCPtr<HFCURL>& pi_rpURLFileName,
                                                               const Utf8String&     pi_Extension,
                                                               uint64_t              pi_Offset = 0) const;

protected:
    HFCPtr<HFCURL> m_pPath;
    HFCPtr<HFCURL> m_pDefaultPath;


    Utf8String                         ComposeFilenameFor(const HFCPtr<HFCURL>& pi_rpURLFileName) const;

    HRFLocalCacheFileCreator();
private:
    void internComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName,
                             const Utf8String&     pi_Extension,
                             uint64_t              pi_Offset,
                             BeFileName&           po_FileName) const;

    // Disabled methods
    HRFLocalCacheFileCreator(const HRFLocalCacheFileCreator&);
    HRFLocalCacheFileCreator& operator=(const HRFLocalCacheFileCreator&);
    };
END_IMAGEPP_NAMESPACE

