//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBDumbAnalyzer.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBDumbAnalyzer
//-----------------------------------------------------------------------------
#pragma once

#include "PUBQueryAnalyzer.h"

class PUBDumbAnalyzer : public PUBQueryAnalyzer
    {
public:
    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    PUBDumbAnalyzer();
    virtual             ~PUBDumbAnalyzer();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Verifies if a query is valid or not
    virtual bool       IsQueryValid    (const string& pi_rQuery) const;

    // Standardizes a query in other to maximize cache hits
    virtual string      Standardize     (const string& pi_rQuery) const;

    // configuration methods
    virtual void        SetConfiguration(const PUBConfiguration& pi_rConfiguration);
    };

#include "PUBDumbAnalyzer.hpp"