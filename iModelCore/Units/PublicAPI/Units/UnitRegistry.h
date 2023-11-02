/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! A central place to store registered units with the system.  Users interact
//! with the units system here.
// @bsiclass
//=======================================================================================
struct UnitRegistry : IUnitsContext
{
friend struct Unit;
private:
    bmap<Utf8CP, UnitSystemP, less_str> m_systems;
    bmap<Utf8CP, PhenomenonP, less_str> m_phenomena;
    bmap<Utf8CP, UnitP, less_str> m_units;

    UnitRegistry(const UnitRegistry& rhs) = delete;
    UnitRegistry & operator= (const UnitRegistry& rhs) = delete;

    void AddSystem(UnitSystemR unitSystem); // Used for creating the static definitions.

    PhenomenonP AddPhenomenonInternal(Utf8CP phenomenaName, Utf8CP definition);
    UnitP AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant);
    UnitP AddInvertedUnitInternal(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName);
    UnitSystemP AddSystemInternal(Utf8CP name);

protected:
    PhenomenonP _LookupPhenomenonP(Utf8CP name, bool = false) const override {auto val_iter = m_phenomena.find(name); return val_iter == m_phenomena.end() ? nullptr : (*val_iter).second;}
    void _AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const override;

    UnitP _LookupUnitP(Utf8CP name, bool = false) const override {auto val_iter = m_units.find(name); return val_iter == m_units.end() ? nullptr : (*val_iter).second;}
    void _AllUnits(bvector<UnitCP>& allUnits) const override;

    UnitSystemP _LookupUnitSystemP(Utf8CP name, bool = false) const override {auto val_iter = m_systems.find(name); return val_iter == m_systems.end() ? nullptr : (*val_iter).second;}
    void _AllSystems(bvector<UnitSystemCP>& allUnitSystems) const override;

    bool NamedItemExists(Utf8CP name) {return HasUnit(name) || HasPhenomenon(name) || HasSystem(name);}

    UnitCP CreateDummyUnit(Utf8CP unitName);
public:
    //! Constructs a new instance of the UnitRegistry with a default set of Units/Phenomenon/UnitSystems.
    UNITS_EXPORT UnitRegistry();

    ~UnitRegistry()
        {
        for_each(m_systems.begin(), m_systems.end(), [](auto pair) { delete pair.second; });
        for_each(m_phenomena.begin(), m_phenomena.end(), [](auto pair) { delete pair.second; });
        for_each(m_units.begin(), m_units.end(), [](auto pair) { delete pair.second; });
        }

    //! Populates the provided vector with the name of all the Units in the registry. If includeSynonyms is true, all Unit synonym names
    //! will be included.
    //! @param[out] allUnitNames     The vector to populate with the unit names
    //! @param[in] includeSynonyms  If true, will include all units synonyms
    UNITS_EXPORT void AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const;

    //! Creates an invalid, "dummy", Unit with the provided name.
    //! @param[in] unitName Name of the dummy unit to be created.
    //! @return A dummy Unit if successfully created and added to this registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddDummyUnit(Utf8CP unitName) {return CreateDummyUnit(unitName);}

    //! Creates a Unit and adds it to the registry.
    //! @param[in] phenomName Name of the Phenomenon the Unit must be added to.
    //! @param[in] systemName Name of the UnitSystem the Unit must be added to.
    //! @param[in] unitName Name of the Unit to be created.
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @param[in] offset
    //! @return A Unit if successfully created and added to this registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator = 1, double denominator = 1, double offset = 0);

    //! Creates an inverted Unit for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the inverted Unit for
    //! @param[in] unitName Name of the Inverted Unit to be created
    //! @param[in] unitSystemName Name of the unit system this inverted unit belongs to
    //! @return An inverted Unit if successfully created and added to this registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddInvertedUnit(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName);

    //! Creates a Constant and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant is needs to be added to
    //! @param[in] systemName Name of the UnitSystem the new Constant belongs to
    //! @param[in] constantName Name of the Constant to be created
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @return A constant Unit if successfully created and added to the registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddConstant(Utf8CP phenomName, Utf8CP systemName, Utf8CP constantName, Utf8CP definition, double numerator, double denominator = 1);

    //! Creates a Phenomenon and adds it to this registry.
    //! @param[in] name Name of the Phenomenon to be created.
    //! @param[in] definition
    //! @return A Phenomenon if successfully created and added to the registry, nullptr otherwise.
    UNITS_EXPORT PhenomenonCP AddPhenomenon(Utf8CP name, Utf8CP definition);

    //! Creates a UnitSystem and adds it to this registry.
    //! @param[in] name Name of the System to be created
    //! @return A UnitSystem if successfully created and added to the registry, nullptr otherwise.
    UNITS_EXPORT UnitSystemCP AddSystem(Utf8CP name);

    //! Gets the Constant from this registry.
    //! @param[in] name Name of the Constant to retrieve.
    //! @return A Constant Unit if it found in this registry, nullptr otherwise
    UNITS_EXPORT UnitCP LookupConstant(Utf8CP name) const;

    //! Indicates if the UnitSystem is in this registry.
    bool HasSystem(Utf8CP systemName) const {return m_systems.end() != m_systems.find(systemName);}
    //! Indicates if the Phenomenon is in this registry.
    bool HasPhenomenon(Utf8CP phenomenonName) const {return m_phenomena.end() != m_phenomena.find(phenomenonName);}
    //! Indicates if the Unit is in this registry.
    bool HasUnit(Utf8CP unitName) const {return m_units.end() != m_units.find(unitName);}

    //! Removes the UnitSystem from this UnitRegistry and returns the pointer to that UnitSystem.
    //! It does not delete the UnitSystem rather just removes it from the registry.
    //! @return True if the system was found and removed, false if not found.
    UNITS_EXPORT bool RemoveSystem(Utf8CP systemName);

    //! Removes the Phenomenon from this UnitRegistry and returns the pointer to that Phenomenon.
    //! It does not delete the Phenomenon rather just removes it from the registry.
    //! @return True if the phenomenon was found and removed, false if not found.
    UNITS_EXPORT bool RemovePhenomenon(Utf8CP phenomenonName);

    //! Removes the Unit from this UnitRegistry and returns the pointer to that Unit.
    //! It does not delete the Unit rather just removes it from the registry.
    //! @return True if the unit was found and removed, false if not found.
    UNITS_EXPORT bool RemoveUnit(Utf8CP unitName);

    //! Removes the Unit from this UnitRegistry and returns the pointer to that Unit.
    //! It does not delete the Unit rather just removes it from the registry.
    //! @return True if the unit was found and removed, false if not found.
    UNITS_EXPORT bool RemoveInvertedUnit(Utf8CP unitName);

    //! Removes the constant from this UnitRegistry and returns the pointer to that constant.
    //! It does not delete the constant rather just removes it from the registry.
    //! @return True if the constant was found and removed, false if not found.
    UNITS_EXPORT bool RemoveConstant(Utf8CP constantName);

    //! Gets the Unit by the old name
    //! @see UnitNameMappings
    //! @return A Unit if it found in this registry, nullptr otherwise
    UNITS_EXPORT UnitCP LookupUnitUsingOldName(Utf8CP oldName) const;

    UnitCP GetPlatformLengthUnit() {return LookupUnit("M");}
};

/** @endGroup */
END_BENTLEY_UNITS_NAMESPACE
