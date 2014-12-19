//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequestProcessor.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequestProcessor
//-----------------------------------------------------------------------------
#pragma once

#include "HCSRequestProcessor.h"

class PUBQueryAnalyzer;
class PUBCache;
class PUBRequestDispatcher;
class PUBConfiguration;

class PUBRequestProcessor : public HCSRequestProcessor
    {
public:
    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    PUBRequestProcessor(HCSConnectionPool&    pi_rPool,
                        PUBCache&             pi_rCache,
                        PUBRequestDispatcher& pi_rDispatcher);
    virtual             ~PUBRequestProcessor();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Thread implementation
    virtual void        Go();

    // configuration methods
    virtual void        SetConfiguration(const PUBConfiguration& pi_rConfiguration);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The analyzer used to verify and standardize a request
    mutable HFCExclusiveKey m_AnalyzerKey;
    HAutoPtr<PUBQueryAnalyzer>
    m_pAnalyzer;

    // A reference to the cache engine
    PUBCache&               m_rCache;

    // A reference to the request dispatcher
    PUBRequestDispatcher&   m_rDispatcher;
    };