//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBFileCache.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBFileCache
//-----------------------------------------------------------------------------

#include "HFCURL.h"
#include "HFCURLFile.h"

//-----------------------------------------------------------------------------
// Public
// changes the location of future cache entries
//-----------------------------------------------------------------------------
inline void PUBFileCache::SetLocation(const HFCURLFile& pi_rLocation)
    {
    m_pLocation = HFCURL::Instanciate(pi_rLocation.GetURL());
    HASSERT(m_pLocation != 0);
    }


//-----------------------------------------------------------------------------
// Public
// returns the location of future cache entries
//-----------------------------------------------------------------------------
inline const HFCURLFile& PUBFileCache::GetLocation() const
    {
    return (*((HFCPtr<HFCURLFile>&)m_pLocation))
    }
