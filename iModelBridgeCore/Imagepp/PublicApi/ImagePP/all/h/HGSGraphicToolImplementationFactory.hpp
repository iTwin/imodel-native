//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolImplementationFactory.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicToolImplementationFactory
//---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Register
//-----------------------------------------------------------------------------
inline void HGSGraphicToolImplementationFactory::Register(const HGSGraphicToolImplementationCreator* pi_pCreator)
    {
    // register the creator
    m_Creators.push_back(pi_pCreator);
    }

