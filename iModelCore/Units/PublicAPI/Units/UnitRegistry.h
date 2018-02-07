/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitRegistry.h $
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
//! A central place to store registered units with the system.  Users interact
//! with the units system here.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================

struct UnitRegistry
{
friend struct Unit;
private:
    static UnitRegistry * s_instance;

    //=====================================================================================//
    // Comparison function that is used within various data structures
    // for string comparison in STL collections.
    // @bsistruct
    //+===============+===============+===============+===============+===============+====//
    struct less_str
        {
        bool operator()(Utf8CP s1, Utf8CP s2) const
            {
            if (BeStringUtilities::StricmpAscii(s1, s2) < 0)
                return true;

            return false;
            }
        };

    uint32_t m_nextId = 0;

    bmap<Utf8CP, UnitSystemP, less_str> m_systems;
    bmap<Utf8String, PhenomenonP> m_phenomena;
    bmap<Utf8String, UnitP> m_units;

    bmap<uint64_t, Conversion> m_conversions;

    UnitRegistry();
    UnitRegistry(const UnitRegistry& rhs) = delete;
    UnitRegistry & operator= (const UnitRegistry& rhs) = delete;
    
    // TODO: Remove
    // The Default methods below are used to populate the UnitRegistry with the definitions that were previously
    // added to the registry and should be phased out in place of User defined definitions inside of an ECSchema.
    void AddDefaultPhenomena();
    void AddDefaultUnits();
    void AddDefaultConstants();

    void AddBaseSystems();
    void AddBaseUnits();
    void AddBasePhenomena();

    void InsertUnique(Utf8Vector &vec, Utf8String &str);
    
    void AddSystem(UnitSystemR unitSystem); // Used for creating the static definitions.
    
    void AddBasePhenomenon(Utf8Char baseSymbol);
    UnitCP AddUnitForBasePhenomenon(Utf8CP unitName, Utf8Char baseSymbol);

    template <typename UNIT_TYPE>
    UNIT_TYPE* AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
        {
        static_assert(std::is_base_of<Unit, UNIT_TYPE>::value, "UNIT_TYPE must derive from Units::Unit.");
        if (Utf8String::IsNullOrEmpty(unitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Cannot create base unit because the input name is null");
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        PhenomenonP phenomenon = LookupPhenomenonP(phenomName);
        if (nullptr == phenomenon)
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Could not find phenomenon '%s'", phenomName);
            return nullptr;
            }

        UnitSystemCP system = LookupUnitSystem(systemName);
        if (nullptr == system)
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Could not find system '%s'", systemName);
            return nullptr;
            }

        UNIT_TYPE* unit = UNIT_TYPE::_Create(*system, *phenomenon, unitName, m_nextId, definition, baseSymbol, factor, offset, isConstant);
        if (nullptr == unit)
            return nullptr;

        // Add the unit label as a synonym as long as it is not the same as the actual unit name
        if (!Utf8String::IsNullOrEmpty(unit->GetLabel()) && (0 != BeStringUtilities::StricmpAscii(unit->GetLabel(), unitName)))
            unit->AddSynonym(unit->GetLabel());

        phenomenon->AddUnit(*unit);

        ++m_nextId;

        m_units.insert(bpair<Utf8String, UnitP>(unitName, (UnitP) unit));

        return unit;
        }

    template<typename UNIT_TYPE>
    UNIT_TYPE* AddInvertingUnitInternal(Utf8CP parentUnitName, Utf8CP unitName)
        {
        static_assert(std::is_base_of<Unit, UNIT_TYPE>::value, "UNIT_TYPE must derive from Units::Unit.");
        if (Utf8String::IsNullOrEmpty(unitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Cannot create unit because the input name is null");
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(parentUnitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create unit %s because it's parent name is null", unitName);
            return nullptr;
            }

        UnitCP parentUnit = LookupUnit(parentUnitName);
        if (nullptr == parentUnit)
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create unit %s because it's parent unit %s cannot be found", unitName, parentUnitName);
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        auto unit = UNIT_TYPE::_Create(*parentUnit, unitName, m_nextId);
        if (nullptr == unit)
            return nullptr;

        PhenomenonP phenomenon = LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName());
        phenomenon->AddUnit(*unit);

        ++m_nextId;

        m_units.insert(bpair<Utf8String, UnitP>(unitName, (UnitP) unit));

        return unit;
        }

    template<typename SYSTEM_TYPE>
    SYSTEM_TYPE* AddSystemInternal(Utf8CP name)
        {
        static_assert(std::is_base_of<UnitSystem, SYSTEM_TYPE>::value, "SYSTEM_TYPE must derive from Units::UnitSystem.");
        if (Utf8String::IsNullOrEmpty(name))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Cannot create UnitSystem because name is null");
            return nullptr;
            }
        if (HasSystem(name))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create UnitSystem '%s' because that name is already in use.", name);
            return nullptr;
            }

        auto unitSystem = SYSTEM_TYPE::_Create(name);
        m_systems.Insert(unitSystem->GetName().c_str(), unitSystem);

        return unitSystem;
        }

    PhenomenonP LookupPhenomenonP(Utf8CP name) const {auto val_iter = m_phenomena.find(name); return val_iter == m_phenomena.end() ? nullptr : (*val_iter).second;}
    UnitP LookupUnitP(Utf8CP name) const {auto val_iter = m_units.find(name); return val_iter == m_units.end() ? nullptr : (*val_iter).second; }
    UnitSystemP LookupUnitSystemP(Utf8CP name) const {auto val_iter = m_systems.find(name); return val_iter == m_systems.end() ? nullptr : (*val_iter).second;}

    // TFS#814870 There are currently conflicts with the supported Units, check the TFS item for more details
    //bool NamedItemExists(Utf8CP name) {return HasUnit(name) || HasPhenomenon(name) || HasSystem(name);}
    bool NamedItemExists(Utf8CP name) {return HasUnit(name);}

    bool TryGetConversion(uint64_t index, Conversion& conversion);
    void AddConversion(uint64_t index, Conversion& conversion) {m_conversions.Insert(index, conversion);}

public:
    //! Returns a pointer to the singleton instance of the UnitRegistry
    //!
    //! If the singleton has not yet been instantiated the UnitRegistry will be initialized with 
    //! a set of base Units, Phenomena, and Unit Systems. For more details see the description of the class. TODO
    UNITS_EXPORT static UnitRegistry & Instance();

    //! Clears the singleton instance of all definitions.
    UNITS_EXPORT static void Clear(); // TODO: Remove or hide so cannot be called from public API, only needed for performance testing

    //! Populates the provided vector with all Units in the registry
    //! @param[in] allUnits The vector to populate with the units
    UNITS_EXPORT void AllUnits(bvector<UnitCP>& allUnits) const;

    //! Populates the provided vector with the name of all the Units in the registry. If includeSynonyms is true, all Unit synonym names 
    //! will be included.
    //! @param[out] allUnitNames     The vector to populate with the unit names
    //! @param[in] includeSynonyms  If true, will include all units synonyms
    UNITS_EXPORT void AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const;

    //! Populates the provided vector with all Phenomena in the registry
    //! @param[out] allPhenomena The vector to populate with the phenomena
    UNITS_EXPORT void AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const;

    //! Populates the provided vector with all UnitSystems in the registry
    //! @param[out] allUnitSystems The vector to populate with the unit systems
    UNITS_EXPORT void AllSystems(bvector<UnitSystemCP>& allUnitSystems) const;

    //! Creates a dummy unit with the given name and add 
    //! @param[in] unitName Name of the dummy Unit to be created.
    //! @return A dummy Unit if successfully created and added to this registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddDummyUnit(Utf8CP unitName);

    //! Creates a Unit, of the provided UNIT_TYPE, and adds it to the registry.
    //! @param[in] phenomName Name of the Phenomenon the Unit must be added to.
    //! @param[in] systemName Name of the UnitSystem the Unit must be added to.
    //! @param[in] constantName Name of the Constant to be created.
    //! @param[in] definition 
    //! @param[in] factor
    //! @param[in] offset
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return A UNIT_TYPE if successfully created and added to this registry, nullptr otherwise.
    template <typename UNIT_TYPE> 
    UNIT_TYPE* AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0)
        {return AddUnitInternal<UNIT_TYPE>(phenomName, systemName, unitName, definition, ' ', factor, offset, false);}

    //! Creates a Unit and adds it to the registry.
    //! @param[in] phenomName Name of the Phenomenon the Unit must be added to.
    //! @param[in] systemName Name of the UnitSystem the Unit must be added to.
    //! @param[in] constantName Name of the Constant to be created.
    //! @param[in] definition 
    //! @param[in] factor
    //! @param[in] offset
    //! @return A Unit if successfully created and added to this registry, nullptr otherwise.
    UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0) 
        {return AddUnit<Unit>(phenomName, systemName, unitName, definition, factor, offset);}

    //! Creates an Inverting Unit, of the provided UNIT_TYPE, for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the Inverting Unit for
    //! @param[in] unitName Name of the Inversting Unit to be created
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return An Inverting UNIT_TYPE if successfully created and added to this registry, nullptr otherwise.
    template<typename UNIT_TYPE>
    UNIT_TYPE* AddInvertingUnit(Utf8CP parentUnitName, Utf8CP unitName) {return AddInvertingUnitInternal<UNIT_TYPE>(parentUnitName, unitName);}

    //! Creates an Inverting Unit for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the Inverting Unit for
    //! @param[in] unitName Name of the Inversting Unit to be created
    //! @return An inverting Unit if successfully created and added to this registry, nullptr otherwise.
    UnitCP AddInvertingUnit(Utf8CP parentUnitName, Utf8CP unitName) {return AddInvertingUnit<Unit>(parentUnitName, unitName);}

    //! Creates a Constant, of the provided UNIT_TYPE, and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant is needs to be added to
    //! @param[in] constantName Name of the Constant to be created
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return A constant UNIT_TYPE if successfully created and added to the registry, nullptr otherwise.
    template <typename UNIT_TYPE>
    UNIT_TYPE* AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor)
        {
        // TODO: Find a way to have the CONSTANT be the staticly defined one... Maybe need to forward declare this method...
        return AddUnitInternal<UNIT_TYPE>(phenomName, "CONSTANT", constantName, definition, ' ', factor, 0, true);
        }

    //! Creates a Constant and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant is needs to be added to
    //! @param[in] constantName Name of the Constant to be created
    //! @return A constant Unit if successfully created and added to the registry, nullptr otherwise.
    UnitCP AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor) {return AddConstant<Unit>(phenomName, constantName, definition, factor);}
    
    //! Creates a Phenomenon and adds it to this registry.
    //! @param[in] name Name of the Phenomenon to be created.
    //! @param[in] definition
    //! @return A Phenomenon if successfully created and added to the registry, nullptr otherwise.
    PhenomenonCP AddPhenomenon(Utf8CP name, Utf8CP definition);

    //! Creates a system, of the provided SYSTEM_TYPE, and adds it to this registry.
    //! @param[in] name Name of the System to be created
    //! @note The SYSTEM_TYPE provided must derive from Units::UnitSystem.
    //! @return A SYSTEM_TYPE if successfully created and added to the registry, nullptr otherwise.
    template<typename SYSTEM_TYPE>
    SYSTEM_TYPE* AddSystem(Utf8CP name) {return AddSystemInternal<SYSTEM_TYPE>(name);}

    //! Creates a UnitSystem and adds it to this registry.
    //! @param[in] name Name of the System to be created
    //! @return A UnitSystem if successfully created and added to the registry, nullptr otherwise.
    UnitSystemCP AddSystem(Utf8CP name) {return AddSystem<UnitSystem>(name);}

    //! Gets the Unit from this registry.
    //! @param[in] name Name of the Unit to retrieve.
    //! @return A Unit if it found in this registry, nullptr otherwise
    UnitCP LookupUnit(Utf8CP name) const {return LookupUnitP(name);}

    //! Gets the Constant from this registry.
    //! @param[in] name Name of the Constant to retrieve.
    //! @return A Constant Unit if it found in this registry, nullptr otherwise
    UNITS_EXPORT UnitCP LookupConstant(Utf8CP name) const;

    //! Gets the Phenomenon from this registry.
    //! @param[in] name Name of the Phenomenon to retrieve.
    //! @return A Phenomenon if it found in this registry, nullptr otherwise
    PhenomenonCP LookupPhenomenon(Utf8CP name) const {return LookupPhenomenonP(name);}

    //! Gets the UnitSystem from this registry.
    //! @param[in] name Name of the UnitSystem to retrieve.
    //! @return A UnitSystem if it found in this registry, nullptr otherwise
    UnitSystemCP LookupUnitSystem(Utf8CP name) const {return LookupUnitSystemP(name);}

    //! Indicates if the UnitSystem is in this registry.
    bool HasSystem(Utf8CP systemName) const {return m_systems.end() != m_systems.find(systemName);}
    //! Indicates if the Phenomenon is in this registry.
    bool HasPhenomenon(Utf8CP phenomenonName) const {return m_phenomena.end() != m_phenomena.find(phenomenonName);}
    //! Indicates if the Unit is in this registry.
    bool HasUnit(Utf8CP unitName) const {return m_units.end() != m_units.find(unitName);}

    //! Removes the UnitSystem from the this UnitRegistry and returns the pointer to that UnitSystem.
    //! It does not delete the UnitSystem rather just removes it from the registry.
    //! @return A pointer to the UnitSystem that is no longer within this registry.
    UNITS_EXPORT UnitSystemP RemoveSystem(Utf8CP systemName);

    //! Gets the "new" unit name from a "old" unit name
    //! @see UnitNameMappings for information on the difference between a old and new unit name.
    //! @return The "new" name if found, otherwise nullptr
    UNITS_EXPORT static Utf8CP TryGetNewName(Utf8CP oldName);

    //! Gets the "old" unit name for a new unit name
    //! @see UnitNameMappings for information on the difference between a old and new unit name.
    //! @return The "old" name if found, otherwise nullptr
    UNITS_EXPORT static Utf8CP TryGetOldName(Utf8CP newName);

    //! Gets the EC compatible name for a new unit name
    //! @see UnitNameMappings for information on the difference between an EC and a new unit name.
    //! @return The EC name if found, otherwise nullptr
    UNITS_EXPORT static Utf8CP TryGetECName(Utf8CP name);

    //! Gets the new unit name for an EC compatible name
    //! @see UnitNameMappings for information on the difference between an EC and a new unit name.
    //! @return The new unit name if found, otherwise nullptr
    UNITS_EXPORT static Utf8CP TryGetNameFromECName(Utf8CP ecName);

    //! Gets the Unit by the old name
    //! @see UnitNameMappings
    //! @return A Unit if it found in this registry, nullptr otherwise
    UNITS_EXPORT UnitCP LookupUnitUsingOldName(Utf8CP oldName) const;

    UnitCP GetPlatformLengthUnit() {return LookupUnit("M");}
    UNITS_EXPORT size_t LoadSynonyms(Json::Value jval) const;
    UNITS_EXPORT PhenomenonCP LoadSynonym(Utf8CP unitName, Utf8CP synonym) const;
    UNITS_EXPORT Json::Value SynonymsToJson() const;
    UNITS_EXPORT UnitCP LookupUnitCI(Utf8CP name) const;
};

/** @endGroup */
END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
