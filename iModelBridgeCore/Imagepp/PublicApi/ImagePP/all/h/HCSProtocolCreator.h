//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSProtocolCreator.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSProtocolCreator
//-----------------------------------------------------------------------------

#pragma once

template<class Context> class HCSProtocol;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif


    This class defines the interface to implement protocol creators. Such creators
    will upon declaration, automatically register itself within the
    HCSServerHandler of the same template argument type in the internal static
    protocol creator list.

    Such creators remove the need to register protocols manually when creating server
    handlers.
    -----------------------------------------------------------------------------
*/
template<class Context>
class HCSProtocolCreator
    {
public:
    //--------------------------------------
    // Constructor
    //--------------------------------------

    HCSProtocolCreator(bool pi_Default = false);
    virtual         ~HCSProtocolCreator();

    //--------------------------------------
    // Methods
    //--------------------------------------

    virtual HCSProtocol<Context>*
    Create() const = 0;
    bool           IsDefault() const;

protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    static void     RegisterProtocol(const HCSProtocolCreator<Context>* pi_pCreator);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    bool           m_Default;
    };

#include "HCSProtocolCreator.hpp"

