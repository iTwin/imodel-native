//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/bmap.h>

#include "IGeoTiffKeysList.h"

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

class BaseGeoTiffKeysList : public IGeoTiffKeysList
    {

public:

    BaseGeoTiffKeysList() {};

    virtual         ~BaseGeoTiffKeysList() {m_GeoKeyList.clear();};

    // From IGeoTiffKeysList
    virtual bool     GetFirstKey(GeoKeyItem* po_Key) const override
        {
        if (po_Key != nullptr)
            {
            m_GeoKeyListItr = m_GeoKeyList.begin();
     
            if (m_GeoKeyListItr != m_GeoKeyList.end())
                {
                *po_Key =  (*m_GeoKeyListItr).second;
                return true;
                }
            }
        return false;
        }

    virtual bool     GetNextKey(GeoKeyItem* po_Key) const override
        {
        if (po_Key != nullptr)
            {
            m_GeoKeyListItr++;
        
            if (m_GeoKeyListItr != m_GeoKeyList.end())
                {
                *po_Key =  (*m_GeoKeyListItr).second;
                return true;
                }
            }
        
        return false;
        }

    virtual void     AddKey (unsigned short pi_KeyID, uint32_t pi_value) override
        {
        GeoKeyItem CurKey;
    
        CurKey.KeyID = pi_KeyID;
        CurKey.KeyDataType = IGeoTiffKeysList::LONG;
        CurKey.KeyValue.LongVal = pi_value;
    
        m_GeoKeyList.insert(GeoKeyList::value_type(CurKey.KeyID, CurKey));
        }

    virtual void     AddKey (unsigned short pi_KeyID, double pi_value) override
        {
        GeoKeyItem CurKey;

        CurKey.KeyID = pi_KeyID;
        CurKey.KeyDataType = IGeoTiffKeysList::DOUBLE;
        CurKey.KeyValue.DoubleVal = pi_value;

        m_GeoKeyList.insert(GeoKeyList::value_type(CurKey.KeyID, CurKey));
        }

    virtual void     AddKey (unsigned short pi_KeyID, const std::string& pi_value) override
        {
        GeoKeyItem CurKey;
        
        CurKey.KeyID = pi_KeyID;
        CurKey.KeyDataType = IGeoTiffKeysList::ASCII;
        
        char* pStr = new char[pi_value.size()+1];
        BeStringUtilities::Strncpy(pStr, pi_value.size()+1, pi_value.c_str(), pi_value.size());
        
        CurKey.KeyValue.StringVal = pStr;
        
        m_GeoKeyList.insert(GeoKeyList::value_type(CurKey.KeyID, CurKey));
        }

    virtual void            AddKey (const GeoKeyItem& key) override
        {
        GeoKeyItem CurKey = key;
        
        m_GeoKeyList.insert(GeoKeyList::value_type(CurKey.KeyID, CurKey));
        }

private:

    // Not implemented
    BaseGeoTiffKeysList&   operator=(const BaseGeoTiffKeysList& pi_rObj);
    BaseGeoTiffKeysList(const BaseGeoTiffKeysList& pi_rObj);

    // The key is the key value.
    typedef bmap<unsigned short, GeoKeyItem> GeoKeyList;
    GeoKeyList                              m_GeoKeyList;
    mutable GeoKeyList::const_iterator      m_GeoKeyListItr;
    };

} // ends GeoCoordinates namespace
END_BENTLEY_NAMESPACE
