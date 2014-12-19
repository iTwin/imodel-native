//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBCache.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBCache
//-----------------------------------------------------------------------------
#pragma once

class HFCURL;
class PUBConfiguration;
class PUBCacheEntry;

class PUBCache
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBCache(size_t pi_MaximumItems = INT_MAX);
    virtual             ~PUBCache();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Adds an entry to the cache
    virtual HFCPtr<PUBCacheEntry>
    AddEntry(const string& pi_rRequest);

    // Gets an entry from  the cache
    virtual HFCPtr<PUBCacheEntry>
    GetEntry(const string& pi_rRequest);

    // Removes an entry from the cache
    virtual void        RemoveEntry(const string& pi_rRequest);


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // the maximum number of items
    size_t              m_MaximumItems;

    // The list to implement as first in first out
    typedef list<HFCPtr<PUBCacheEntry> >
    EntryList;
    EntryList           m_EntryList;

    // The map of entries based on the request string
    typedef pair<EntryList::iterator, HFCPtr<PUBCacheEntry> >
    EntryType;
    typedef map<string, EntryType>
    EntryMap;
    EntryMap            m_EntryMap;

    // the key to protect the access to the entry map
    mutable HFCExclusiveKey
    m_Key;
    };

#include "PUBCache.hpp"