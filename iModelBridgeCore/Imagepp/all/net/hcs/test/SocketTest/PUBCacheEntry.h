//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBCacheEntry.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBCacheEntry
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCBuffer.h"

class PUBCacheEntry : public HFCShareableObject<PUBCacheEntry>,
    public HFCExclusiveKey
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBCacheEntry(const string& pi_rQuery)
        : m_Buffer(1024),
          m_Query(pi_rQuery)
        {
        HPRECONDITION(!pi_rQuery.empty());
        };
    virtual             ~PUBCacheEntry()
        {
        };


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Add response data
    void                AddData(const string& pi_rData)
        {
        HPRECONDITION(!pi_rData.empty());

        m_Buffer.AddData((const Byte*)pi_rData.data(), pi_rData.size());
        }

    void                AddData(const Byte* pi_pData, size_t pi_DataSize)
        {
        HPRECONDITION(pi_pData != 0);
        HPRECONDITION(pi_DataSize > 0);

        m_Buffer.AddData(pi_pData, pi_DataSize);
        }

    // returns the response size
    size_t              GetResponseSize() const
        {
        return m_Buffer.GetDataSize();
        }

    // returns the response data
    const Byte*        GetResponseData() const
        {
        HPRECONDITION(GetResponseSize() > 0);

        return m_Buffer.GetData();
        }


    const string&       GetQuery() const
        {
        return m_Query;
        }

private:
    string      m_Query;
    HFCBuffer   m_Buffer;
    };