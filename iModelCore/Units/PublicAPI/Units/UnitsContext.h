/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitsContext.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE
//! @addtogroup UnitsGroup
//! @beginGroup

//=======================================================================================
//! The UnitsContext is an abstact class to provide a generic interface to look up Units, 
//! Phenomena, and UnitSystems within a UnitsContext.
// @bsistruct                                                    Caleb.Shafer       02/18
//=======================================================================================
struct IUnitsContext /* abstract */
{
protected:
    virtual UnitP _LookupUnitP(Utf8CP name) const = 0;
    virtual PhenomenonP _LookupPhenomenonP(Utf8CP name) const = 0;
    virtual UnitSystemP _LookupUnitSystemP(Utf8CP name) const = 0;
    virtual void _AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const = 0;
    virtual void _AllUnits(bvector<UnitCP>& allUnits) const = 0;
    virtual void _AllSystems(bvector<UnitSystemCP>& allUnitSystems) const = 0;

    virtual ~IUnitsContext() {}

public:
    //! Gets the Unit from this context.
    //! @param[in] name Name of the Unit to retrieve.
    //! @return A Unit if it found in this context; otherwise, nullptr.
    virtual UnitCP LookupUnit(Utf8CP name) const {return _LookupUnitP(name);}

    //! Gets the Phenomenon from this context.
    //! @param[in] name Name of the Phenomenon to retrieve.
    //! @return A Phenomenon if it found in this context; otherwise, nullptr.
    virtual PhenomenonCP LookupPhenomenon(Utf8CP name) const {return _LookupPhenomenonP(name);}

    //! Gets the UnitSystem from this context.
    //! @param[in] name Name of the UnitSystem to retrieve.
    //! @return A UnitSystem if it found in this registry; otherwise, nullptr.
    virtual UnitSystemCP LookupUnitSystem(Utf8CP name) const {return _LookupUnitSystemP(name);}

    //! Populates the provided vector with all Phenomena in this context
    //! @param[out] allPhenomena The vector to populate with the phenomena
    void AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const {_AllPhenomena(allPhenomena);}

    //! Populates the provided vector with all UnitSystems in this context
    //! @param[out] allUnitSystems The vector to populate with the unit systems
    void AllSystems(bvector<UnitSystemCP>& allUnitSystems) const {_AllSystems(allUnitSystems);}

    //! Populates the provided vector with all Units in this context
    //! @param[in] allUnits The vector to populate with the units
    void AllUnits(bvector<UnitCP>& allUnits) const { _AllUnits(allUnits); }
};

END_BENTLEY_UNITS_NAMESPACE
