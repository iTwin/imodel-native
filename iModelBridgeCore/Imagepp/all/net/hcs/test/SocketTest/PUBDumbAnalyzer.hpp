//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBDumbAnalyzer.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBDumbAnalyzer
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
inline PUBDumbAnalyzer::PUBDumbAnalyzer()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
inline PUBDumbAnalyzer::~PUBDumbAnalyzer()
    {
    }



//-----------------------------------------------------------------------------
// Public
// Verifies if a query is valid or not
//-----------------------------------------------------------------------------
inline bool PUBDumbAnalyzer::IsQueryValid(const string& pi_rQuery) const
    {
    return (!pi_rQuery.empty());
    }


//-----------------------------------------------------------------------------
// Public
// Standardizes a query in other to maximize cache hits
//-----------------------------------------------------------------------------
inline string PUBDumbAnalyzer::Standardize(const string& pi_rQuery) const
    {
    HPRECONDITION(IsQueryValid(pi_rQuery));

    return (pi_rQuery);
    }


//-----------------------------------------------------------------------------
// Public
// configuration methods
//-----------------------------------------------------------------------------
inline void PUBDumbAnalyzer::SetConfiguration(const PUBConfiguration& pi_rConfiguration)
    {
    }
