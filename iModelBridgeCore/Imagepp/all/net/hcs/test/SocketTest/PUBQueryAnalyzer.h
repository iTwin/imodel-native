//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBQueryAnalyzer.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBQueryAnalyzer
//-----------------------------------------------------------------------------
#pragma once

class PUBConfiguration;

class PUBQueryAnalyzer
    {
public:
    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    PUBQueryAnalyzer();
    virtual             ~PUBQueryAnalyzer();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Verifies if a query is valid or not
    virtual bool       IsQueryValid    (const string& pi_rQuery) const = 0;

    // Standardizes a query in other to maximize cache hits
    virtual string      Standardize     (const string& pi_rQuery) const = 0;

    // configuration methods
    virtual void        SetConfiguration(const PUBConfiguration& pi_rConfiguration) = 0;
    };

#include "PUBQueryAnalyzer.hpp"