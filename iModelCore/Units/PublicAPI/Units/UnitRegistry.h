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

    PhenomenonP AddPhenomenonInternal(Utf8CP phenomenaName, Utf8CP definition)
        {
        if (Utf8String::IsNullOrEmpty(phenomenaName))
            {
            NativeLogging::CategoryLogger("UnitsNative").error("Failed to create Phenomenon because name is null");
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::CategoryLogger("UnitsNative").error("Failed to create Phenomenon because definition is null");
            return nullptr;
            }

        if (NamedItemExists(phenomenaName))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create Phenomenon '%s' because name is already in use", phenomenaName);
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create Phenomenon '%s' because the definition is empty.", phenomenaName);
            return nullptr;
            }

        auto phenomenon = new Phenomenon(phenomenaName, definition);

        phenomenon->m_unitsContext = this;

        m_phenomena.insert(bpair<Utf8CP, PhenomenonP>(phenomenon->GetName().c_str(), phenomenon));

        return phenomenon;
        }


    UnitP AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant)
        {
        if (Utf8String::IsNullOrEmpty(unitName))
            {
            NativeLogging::CategoryLogger("UnitsNative").error("Cannot create base unit because the input name is null");
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        PhenomenonP phenomenon = _LookupPhenomenonP(phenomName);
        if (nullptr == phenomenon)
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Could not find phenomenon '%s'", phenomName);
            return nullptr;
            }

        UnitSystemCP system = LookupUnitSystem(systemName);
        if (nullptr == system)
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Could not find system '%s'", systemName);
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create Unit '%s' because the definition is empty.", unitName);
            return nullptr;
            }

        if (0.0 == numerator || 0.0 == denominator)
            {
             NativeLogging::CategoryLogger("UnitsNative").errorv("Failed to create unit %s because numerator or denominator is 0.  Factor: %.17g / %.17g  Offset: %d", unitName, numerator, denominator, offset);
            return nullptr;
            }
        auto unit = new Unit(*system, *phenomenon, unitName, definition, numerator, denominator, offset, isConstant);

        unit->m_unitsContext = this;
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), unit));

        return unit;
        }

    UnitP AddInvertedUnitInternal(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName)
        {
        if (Utf8String::IsNullOrEmpty(unitName))
            {
            NativeLogging::CategoryLogger("UnitsNative").error("Cannot create unit because the input name is null");
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(parentUnitName))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create unit %s because it's parent name is null", unitName);
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(unitSystemName))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create unit %s because it's unit system name is null", unitName);
            return nullptr;
            }

        UnitCP parentUnit = LookupUnit(parentUnitName);
        if (nullptr == parentUnit)
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create unit %s because it's parent unit %s cannot be found", unitName, parentUnitName);
            return nullptr;
            }

        UnitSystemCP unitSystem = LookupUnitSystem(unitSystemName);
        if (nullptr == unitSystem)
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create unit %s because it's unit system %s cannot be found", unitName, unitSystemName);
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        if (parentUnit->HasOffset())
            {
             NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create inverted unit %s with parent %s because parent has an offset.", unitName, parentUnit->GetName().c_str());
            return nullptr;
            }
        if (parentUnit->IsInvertedUnit())
            {
             NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create inverted unit %s with parent %s because parent is an inverted unit", unitName, parentUnit->GetName().c_str());
            return nullptr;
            }

        auto unit = new Unit(*parentUnit, *unitSystem, unitName);

        unit->m_unitsContext = this;

        PhenomenonP phenomenon = _LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName().c_str());
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), unit));

        return unit;
        }

    UnitSystemP AddSystemInternal(Utf8CP name)
        {
        if (Utf8String::IsNullOrEmpty(name))
            {
            NativeLogging::CategoryLogger("UnitsNative").error("Cannot create UnitSystem because name is null");
            return nullptr;
            }
        if (NamedItemExists(name))
            {
            NativeLogging::CategoryLogger("UnitsNative").errorv("Cannot create UnitSystem '%s' because that name is already in use.", name);
            return nullptr;
            }

        auto unitSystem = new UnitSystem(name);
        unitSystem->m_unitsContext = this;
        m_systems.Insert(unitSystem->GetName().c_str(), unitSystem);

        return unitSystem;
        }

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
    UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator = 1, double denominator = 1, double offset = 0)
        {return AddUnitInternal(phenomName, systemName, unitName, definition, numerator, denominator, offset, false);}

    //! Creates an inverted Unit for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the inverted Unit for
    //! @param[in] unitName Name of the Inversting Unit to be created
    //! @param[in] unitSystemName Name of the unit system this inverted unit belongs to
    //! @return An inverted Unit if successfully created and added to this registry, nullptr otherwise.
    UnitCP AddInvertedUnit(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName) {return AddInvertedUnitInternal(parentUnitName, unitName, unitSystemName);}

    //! Creates a Constant and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant is needs to be added to
    //! @param[in] systemName Name of the UnitSystem the new Constant belongs to
    //! @param[in] constantName Name of the Constant to be created
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @return A constant Unit if successfully created and added to the registry, nullptr otherwise.
    UnitCP AddConstant(Utf8CP phenomName, Utf8CP systemName, Utf8CP constantName, Utf8CP definition, double numerator, double denominator = 1)
        {return AddUnitInternal(phenomName, systemName, constantName, definition, numerator, denominator, 0, true);}

    //! Creates a Phenomenon and adds it to this registry.
    //! @param[in] name Name of the Phenomenon to be created.
    //! @param[in] definition
    //! @return A Phenomenon if successfully created and added to the registry, nullptr otherwise.
    PhenomenonCP AddPhenomenon(Utf8CP name, Utf8CP definition) {return AddPhenomenonInternal(name, definition);}

    //! Creates a UnitSystem and adds it to this registry.
    //! @param[in] name Name of the System to be created
    //! @return A UnitSystem if successfully created and added to the registry, nullptr otherwise.
    UnitSystemCP AddSystem(Utf8CP name) {return AddSystemInternal(name);}

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
