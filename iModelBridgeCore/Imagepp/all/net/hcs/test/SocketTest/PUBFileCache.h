//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBFileCache.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBFileCache
//-----------------------------------------------------------------------------
#pragma once

#include "PUBCache.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

class PUBFileCache : PUBCache
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBFileCache(const HFCURLFile& pi_rLocation);
    virtual             ~PUBFileCache();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Adds an entry to the cache
    virtual HFCPtr<PUBFileCacheEntry>
    AddEntry(const string& pi_rRequest);

    // Gets an entry from  the cache
    virtual HFCPtr<PUBFileCacheEntry>
    GetEntry(const string& pi_rRequest);

    // Removes an entry from the cache
    virtual void        RemoveEntry(const string& pi_rRequest);

    // returns and changes the location of future cache entries
    virtual void        SetLocation(const HFCURLFile& pi_rLocation);
    virtual const HFCURLFile&
    GetLocation() const;


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The exclusive key for the cahe
    mutable HFCExclusiveKey m_Key

    // The location to give to the cache entries
    HFCPtr<HFCURL>          m_pLocation;

    // the map of cache entries based ont he request string
    typedef map<string, HFCPtr<PUBCacheEntry> >
    Entries;
    Entries                 m_Entries;
    };

#include "PUBFileCache.hpp"