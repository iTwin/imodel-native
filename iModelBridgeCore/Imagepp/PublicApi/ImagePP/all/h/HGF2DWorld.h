//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DWorld.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DWorld
//-----------------------------------------------------------------------------
// Description of 2D world. A world is simply a coordinate system
// which possess an identification
//-----------------------------------------------------------------------------

#pragma once


#include "HGF2DTransfoModel.h"
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>

BEGIN_IMAGEPP_NAMESPACE

typedef uint32_t HGF2DWorldIdentificator;

#define HGF2DWorld_UNKNOWNWORLD    ((HGF2DWorldIdentificator)0)
#define HGF2DWorld_HMRWORLD        ((HGF2DWorldIdentificator)1)
#define HGF2DWorld_DGNWORLD        ((HGF2DWorldIdentificator)2)
#define HGF2DWorld_INTERGRAPHWORLD ((HGF2DWorldIdentificator)3)
#define HGF2DWorld_ITIFFWORLD      ((HGF2DWorldIdentificator)4)
#define HGF2DWorld_GEOTIFFUNKNOWN  ((HGF2DWorldIdentificator)8)
#define HGF2DWorld_GEOGRAPHIC      ((HGF2DWorldIdentificator)9)

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a named 2D coordinate system. A world is a coordinate
    system to which has been appended an identificator. This identificator is
    a simple string, the interpretation of which is left to the user of the
    present class. what are the units used with an instance of a coordinate system.
    -----------------------------------------------------------------------------
*/
class HGF2DWorld : public HGF2DCoordSys
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGF2DWorldId_Base)

public:

    // Primary methods
    HGF2DWorld();
    HGF2DWorld(HGF2DWorldIdentificator pi_Identifier);
    HGF2DWorld(const HGF2DTransfoModel&      pi_rModel,
               const HFCPtr<HGF2DCoordSys>&  pi_rpRefSys);
    HGF2DWorld(const HGF2DTransfoModel&      pi_rModel,
               const HFCPtr<HGF2DCoordSys>&  pi_rpRefSys,
               HGF2DWorldIdentificator       pi_Identifier);
    virtual         ~HGF2DWorld();

    // Identification
    void            SetIdentificator(HGF2DWorldIdentificator pi_Identifier);
    HGF2DWorldIdentificator
    GetIdentificator() const;


protected:

private:

    // Attributes
    HGF2DWorldIdentificator       m_Identifier;

    // Copy constructor
    HGF2DWorld(const HGF2DWorld&      pi_rObj);
    // Assignement operator
    HGF2DWorld&  operator=(const HGF2DWorld& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DWorld.hpp"

