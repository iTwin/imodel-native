//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DWorld.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor

    Creates a world with an undefined identifier and units set to meters

    -----------------------------------------------------------------------------
*/
inline HGF2DWorld::HGF2DWorld()
    {
    }

/** -----------------------------------------------------------------------------
    Constructor

    Creates a world with the specified identifier and units set to meters.

    @param pi_Indentifier IN The indentifier of the world.

    -----------------------------------------------------------------------------
*/
inline HGF2DWorld::HGF2DWorld (HGF2DWorldIdentificator pi_Identifier)
    : m_Identifier(pi_Identifier)
    {
    }

/** -----------------------------------------------------------------------------
    Construtor

    Creates a world based upon another corrdinate system.

    @param pi_rModel IN A constant reference to a reference model, that will
                        expressed the direct relation between the current coordinate
                        system and the other specified coordinate system, which
                        becomes the reference.

    @param pi_rpRefSys IN Reference to a smart pointer to the coordinate system
                          which is linked to the present one through the
                          given transformation model.

    @param pi_Indentifier IN The indentifier of the world.

    @code
        HGF2DWorld*     pRefSystem = new HGF2DWorld(HGF2DWorld_DGNWORLD);

        // A translation is a kind of transformation model
        HGF2DTranslation    TransModel (HGF2DDisplacement(10.4, 34.2);
        HGF2DWorld          ImageASys (TransModel,
                                       HFCPtr<HGF2DCoordSys> (pRefSystem),
                                       HGF2DWorld_GEOTIFFUNKNOWN);
    @end

    -----------------------------------------------------------------------------
*/
inline HGF2DWorld::HGF2DWorld(const HGF2DTransfoModel& pi_rModel,
                              const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys,
                              HGF2DWorldIdentificator         pi_Identifier)
    : HGF2DCoordSys(pi_rModel, pi_rpCoordSys),
      m_Identifier(pi_Identifier)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DCoordSys object.
// PRIVATE
//-----------------------------------------------------------------------------
inline HGF2DWorld::HGF2DWorld(const HGF2DWorld& pi_rObj)
    : HGF2DCoordSys(pi_rObj),
      m_Identifier(pi_rObj.m_Identifier)
    {
    }

/** -----------------------------------------------------------------------------
    The destruction will advise other coordinate system it is related to of its
    destruction, thus permitting them to disconnect. The destruction will also
    provoke the destruction of all transformation models that belong to the
    coordinate system.
    -----------------------------------------------------------------------------
*/
inline HGF2DWorld::~HGF2DWorld()
    {
    }

/** -----------------------------------------------------------------------------
    This method sets of changes the identificator of the world.

    @param pi_rIdentifier IN An HGF2DWorldIdentificator identifying the world

    @see GetIdentificator()
    -----------------------------------------------------------------------------
*/
inline void HGF2DWorld::SetIdentificator(HGF2DWorldIdentificator pi_Identifier)
    {
    m_Identifier = pi_Identifier;
    }


/** -----------------------------------------------------------------------------
    This method returns a const reference to the world identificator

    @return A const reference to the world identifier

    @see SetIdentificator()
    -----------------------------------------------------------------------------
*/
inline HGF2DWorldIdentificator HGF2DWorld::GetIdentificator() const
    {
    return(m_Identifier);
    }


END_IMAGEPP_NAMESPACE