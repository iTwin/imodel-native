//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBCache.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBCache
//-----------------------------------------------------------------------------

#include "PUBCacheEntry.h"

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
inline PUBCache::PUBCache(size_t pi_MaximumItems)
    {
    m_MaximumItems = pi_MaximumItems;
    }


//-----------------------------------------------------------------------------
// Public
// Destroyer
//-----------------------------------------------------------------------------
inline PUBCache::~PUBCache()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Adds an entry to the cache
//-----------------------------------------------------------------------------
inline HFCPtr<PUBCacheEntry> PUBCache::AddEntry(const string& pi_rRequest)
    {
    HFCMonitor Monitor(m_Key);

    // create the entry
    HFCPtr<PUBCacheEntry> pResult = new PUBCacheEntry(pi_rRequest);

    // add the new entry in the list
    m_EntryList.push_back(pResult);
    EntryList::iterator ListItr = m_EntryList.rend().base();

    // add it in the map
    m_EntryMap.insert(EntryMap::value_type(pi_rRequest, EntryType(ListItr, pResult)));

    // make the entries fit in the map
    while (m_EntryList.size() > 1)
        {
        // remove from the list
        HFCPtr<PUBCacheEntry> pEntry(m_EntryList.front());
        m_EntryList.pop_front();

        // remove from the map
        m_EntryMap.erase(pEntry->GetQuery());
        }

    // lock the entry for this thread
    Monitor.ReleaseKey();
    pResult->Claim();

    return pResult;
    }

//-----------------------------------------------------------------------------
// Public
// Gets an entry from  the cache
//-----------------------------------------------------------------------------
inline HFCPtr<PUBCacheEntry> PUBCache::GetEntry(const string& pi_rRequest)
    {
    HFCMonitor Monitor(m_Key);
    HFCPtr<PUBCacheEntry> pResult;

    // find the entry in the map
    EntryMap::iterator MapItr = m_EntryMap.find(pi_rRequest);

    // Use it if in the map
    if (MapItr != m_EntryMap.end())
        {
        // Get the entry
        pResult = (*MapItr).second.second;

        // remove the entry from the map and the list
        EntryList::iterator ListItr = (*MapItr).second.first;
        m_EntryMap.erase(MapItr);
        m_EntryList.erase(ListItr);

        // add at the end of the list
        m_EntryList.push_back(pResult);
        ListItr = m_EntryList.rend().base();

        // add it in the map
        m_EntryMap.insert(EntryMap::value_type(pi_rRequest, EntryType(ListItr, pResult)));

        // lock the entry for this thread
        Monitor.ReleaseKey();
        pResult->Claim();
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
// Removes an entry from the cache
//-----------------------------------------------------------------------------
inline void PUBCache::RemoveEntry(const string& pi_rRequest)
    {
    HFCMonitor Monitor(m_Key);
    HFCPtr<PUBCacheEntry> pResult;

    // find the entry in the map
    EntryMap::iterator MapItr = m_EntryMap.find(pi_rRequest);

    // Use it if in the map
    if (MapItr != m_EntryMap.end())
        {
        // Get the entry
        pResult = (*MapItr).second.second;

        // remove the entry from the map and the list
        EntryList::iterator ListItr = (*MapItr).second.first;
        m_EntryMap.erase(MapItr);
        m_EntryList.erase(ListItr);
        }
    }
