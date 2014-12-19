//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSProtocolCreator.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "HCSServerHandler.h"

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
template<class Context>
inline HCSProtocolCreator<Context>::HCSProtocolCreator(bool pi_Default)
    {
    m_Default = pi_Default;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
template<class Context>
inline HCSProtocolCreator<Context>::~HCSProtocolCreator()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
template<class Context>
inline bool HCSProtocolCreator<Context>::IsDefault() const
    {
    return (m_Default);
    }


//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
template<class Context>
inline void HCSProtocolCreator<Context>::RegisterProtocol(const HCSProtocolCreator<Context>* pi_pCreator)
    {
    HPRECONDITION(pi_pCreator != 0);

    if (HCSServerHandler<Context>::s_pRegisteredProtocols == 0)
        HCSServerHandler<Context>::s_pRegisteredProtocols = new HCSServerHandler<Context>::CreatorList;

    HCSServerHandler<Context>::s_pRegisteredProtocols->push_back(pi_pCreator);
    }


