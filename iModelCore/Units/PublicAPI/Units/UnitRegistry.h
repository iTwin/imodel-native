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
//! Comparison function that is used within various schema related data structures
//! for string comparison in STL collections.
// @bsistruct
//=======================================================================================
struct less_str
{
bool operator()(Utf8String s1, Utf8String s2) const
    {
    if (BeStringUtilities::Stricmp(s1.c_str(), s2.c_str()) < 0)
        return true;
    return false;
    }
};

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




    uint32_t m_nextId = 0;

    bmap<Utf8CP, UnitSystemP, less_str> m_systems;
    bmap<Utf8CP, PhenomenonP, less_str> m_phenomena;
    bmap<Utf8CP, UnitP, less_str> m_units;

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
    
    void AddBasePhenomenon(Utf8CP basePhenomenonName) {AddPhenomenon(basePhenomenonName, basePhenomenonName);}
    UnitCP AddUnitForBasePhenomenon(Utf8CP unitName, Utf8CP basePhenomenonName);

    template <typename PHENOM_TYPE>
    PHENOM_TYPE* AddPhenomenonInternal(Utf8CP phenomenaName, Utf8CP definition)
        {
        static_assert((std::is_base_of<Phenomenon, PHENOM_TYPE>::value), "PHENOM_TYPE must derive from Units::Phenomenon");
        if (Utf8String::IsNullOrEmpty(phenomenaName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Failed to create Phenomenon because name is null");
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Failed to create Phenomenon because definition is null");
            return nullptr;
            }

        if (NamedItemExists(phenomenaName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create Phenomenon '%s' because name is already in use", phenomenaName);
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create Phenomenon '%s' because the definition is empty.", phenomenaName);
            return nullptr;
            }

        auto phenomena = PHENOM_TYPE::_Create(phenomenaName, definition, m_nextId);
        ++m_nextId;

        m_phenomena.insert(bpair<Utf8CP, PHENOM_TYPE*>(phenomena->m_name.c_str(), phenomena));

        return phenomena;
        }

    template <typename UNIT_TYPE>
    UNIT_TYPE* AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant)
        {
        static_assert((std::is_base_of<Unit, UNIT_TYPE>::value), "UNIT_TYPE must derive from Units::Unit.");
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

        if (Utf8String::IsNullOrEmpty(definition))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create Unit '%s' because the definition is empty.", unitName);
            return nullptr;
            }

        UNIT_TYPE* unit = UNIT_TYPE::_Create(*system, *phenomenon, unitName, m_nextId, definition, numerator, denominator, offset, isConstant);
        if (nullptr == unit)
            return nullptr;

        // Add the unit label as a synonym as long as it is not the same as the actual unit name
        if (!Utf8String::IsNullOrEmpty(unit->GetLabel().c_str()) && (0 != BeStringUtilities::StricmpAscii(unit->GetLabel().c_str(), unitName)))
            unit->AddSynonym(unit->GetLabel().c_str());

        phenomenon->AddUnit(*unit);

        ++m_nextId;

        m_units.insert(bpair<Utf8CP, UnitP>(unit->m_name.c_str(), (UnitP) unit));

        return unit;
        }

    template<typename UNIT_TYPE>
    UNIT_TYPE* AddInvertedUnitInternal(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName)
        {
        static_assert((std::is_base_of<Unit, UNIT_TYPE>::value), "UNIT_TYPE must derive from Units::Unit.");
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

        if (Utf8String::IsNullOrEmpty(unitSystemName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create unit %s because it's unit system name is null", unitName);
            return nullptr;
            }

        UnitCP parentUnit = LookupUnit(parentUnitName);
        if (nullptr == parentUnit)
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create unit %s because it's parent unit %s cannot be found", unitName, parentUnitName);
            return nullptr;
            }

        UnitSystemCP unitSystem = LookupUnitSystem(unitSystemName);
        if (nullptr == unitSystem)
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create unit %s because it's unit system %s cannot be found", unitName, unitSystemName);
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        auto unit = UNIT_TYPE::_Create(*parentUnit, *unitSystem, unitName, m_nextId);
        if (nullptr == unit)
            return nullptr;

        PhenomenonP phenomenon = LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName().c_str());
        phenomenon->AddUnit(*unit);

        ++m_nextId;

        m_units.insert(bpair<Utf8CP, UnitP>(unit->m_name.c_str(), (UnitP) unit));

        return unit;
        }

    template<typename SYSTEM_TYPE>
    SYSTEM_TYPE* AddSystemInternal(Utf8CP name)
        {
        static_assert((std::is_base_of<UnitSystem, SYSTEM_TYPE>::value), "SYSTEM_TYPE must derive from Units::UnitSystem.");
        if (Utf8String::IsNullOrEmpty(name))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->error("Cannot create UnitSystem because name is null");
            return nullptr;
            }
        if (NamedItemExists(name))
            {
            NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->errorv("Cannot create UnitSystem '%s' because that name is already in use.", name);
            return nullptr;
            }

        auto unitSystem = SYSTEM_TYPE::_Create(name);
        // Get the name directly to avoid calling any overrides of GetName defined in a derived class
        m_systems.Insert(unitSystem->m_name.c_str(), unitSystem);

        return unitSystem;
        }

    PhenomenonP LookupPhenomenonP(Utf8CP name) const {auto val_iter = m_phenomena.find(name); return val_iter == m_phenomena.end() ? nullptr : (*val_iter).second;}
    UnitP LookupUnitP(Utf8CP name) const {auto val_iter = m_units.find(name); return val_iter == m_units.end() ? nullptr : (*val_iter).second; }
    UnitSystemP LookupUnitSystemP(Utf8CP name) const {auto val_iter = m_systems.find(name); return val_iter == m_systems.end() ? nullptr : (*val_iter).second;}

    bool NamedItemExists(Utf8CP name) {return HasUnit(name) || HasPhenomenon(name) || HasSystem(name);}

    bool TryGetConversion(uint64_t index, Conversion& conversion);
    void AddConversion(uint64_t index, Conversion& conversion) {m_conversions.Insert(index, conversion);}

    UnitCP CreateDummyUnit(Utf8CP unitName);
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

    //! Creates an invalid, "dummy", Unit with the provided name.
    //! @param[in] unitName Name of the dummy unit to be created.
    //! @return A dummy Unit if successfully created and added to this registry, nullptr otherwise.
    UNITS_EXPORT UnitCP AddDummyUnit(Utf8CP unitName) {return CreateDummyUnit(unitName);}

    //! Creates a Unit, of the provided UNIT_TYPE, and adds it to the registry.
    //! @param[in] phenomName Name of the Phenomenon the Unit must be added to.
    //! @param[in] systemName Name of the UnitSystem the Unit must be added to.
    //! @param[in] unitName Name of the Unit to be created.
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @param[in] offset
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return A UNIT_TYPE if successfully created and added to this registry, nullptr otherwise.
    template <typename UNIT_TYPE> 
    UNIT_TYPE* AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator = 1, double denominator = 1, double offset = 0)
        {return AddUnitInternal<UNIT_TYPE>(phenomName, systemName, unitName, definition, numerator, denominator, offset, false);}

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
        {return AddUnit<Unit>(phenomName, systemName, unitName, definition, numerator, denominator, offset);}

    //! Creates an inverted Unit, of the provided UNIT_TYPE, for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the inverted Unit for
    //! @param[in] unitName Name of the Inversting Unit to be created
    //! @param[in] unitSystemName Name of the unit system this inverted unit belongs to
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return An inverted UNIT_TYPE if successfully created and added to this registry, nullptr otherwise.
    template<typename UNIT_TYPE>
    UNIT_TYPE* AddInvertedUnit(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName) {return AddInvertedUnitInternal<UNIT_TYPE>(parentUnitName, unitName, unitSystemName);}

    //! Creates an inverted Unit for the parent Unit and adds it to this registry.
    //! @param[in] parentUnitName Name of the Unit we are creating the inverted Unit for
    //! @param[in] unitName Name of the Inversting Unit to be created
    //! @param[in] unitSystemName Name of the unit system this inverted unit belongs to
    //! @return An inverted Unit if successfully created and added to this registry, nullptr otherwise.
    UnitCP AddInvertedUnit(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName) {return AddInvertedUnit<Unit>(parentUnitName, unitName, unitSystemName);}

    //! Creates a Constant, of the provided UNIT_TYPE, and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant needs to be added to
    //! @param[in] systemName Name of the UnitSystem the new Constant belongs to
    //! @param[in] constantName Name of the Constant to be created
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @note The UNIT_TYPE provided must derive from Units::Unit
    //! @return A constant UNIT_TYPE if successfully created and added to the registry, nullptr otherwise.
    template <typename UNIT_TYPE>
    UNIT_TYPE* AddConstant(Utf8CP phenomName, Utf8CP systemName, Utf8CP constantName, Utf8CP definition, double numerator, double denominator = 1)
        {return AddUnitInternal<UNIT_TYPE>(phenomName, systemName, constantName, definition, numerator, denominator, 0, true);}

    //! Creates a Constant and adds it to this registry.
    //! @param[in] phenomName Name of the Phenomenon the new Constant is needs to be added to
    //! @param[in] systemName Name of the UnitSystem the new Constant belongs to
    //! @param[in] constantName Name of the Constant to be created
    //! @param[in] definition
    //! @param[in] numerator    Numerator for factor
    //! @param[in] denominator  Denominator for factor
    //! @return A constant Unit if successfully created and added to the registry, nullptr otherwise.
    UnitCP AddConstant(Utf8CP phenomName, Utf8CP systemName, Utf8CP constantName, Utf8CP definition, double numerator, double denominator = 1) 
        {return AddConstant<Unit>(phenomName, systemName, constantName, definition, numerator, denominator);}

    //! Creates a Phenomenon and adds it to this registry.
    //! @param[in] name Name of the Phenomenon to be created.
    //! @param[in] definition
    //! @note The PHENOM_TYPE provided must derive from Units::Phenomenon
    //! @return A Phenomenon if successfully created and added to the registry, nullptr otherwise.
    template <typename PHENOM_TYPE>
    PHENOM_TYPE* AddPhenomenon(Utf8CP name, Utf8CP definition) {return AddPhenomenonInternal<PHENOM_TYPE>(name, definition);}

    //! Creates a Phenomenon and adds it to this registry.
    //! @param[in] name Name of the Phenomenon to be created.
    //! @param[in] definition
    //! @return A Phenomenon if successfully created and added to the registry, nullptr otherwise.
    PhenomenonCP AddPhenomenon(Utf8CP name, Utf8CP definition) {return AddPhenomenon<Phenomenon>(name, definition);}

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

    //! Removes the UnitSystem from this UnitRegistry and returns the pointer to that UnitSystem.
    //! It does not delete the UnitSystem rather just removes it from the registry.
    //! @return A pointer to the UnitSystem that is no longer within this registry.
    UNITS_EXPORT UnitSystemP RemoveSystem(Utf8CP systemName);
    
    //! Removes the Phenomenon from this UnitRegistry and returns the pointer to that Phenomenon.
    //! It does not delete the Phenomenon rather just removes it from the registry.
    //! @return A pointer to the Phenomenon that is no longer within this registry.
    UNITS_EXPORT PhenomenonP RemovePhenomenon(Utf8CP phenomenonName);

    //! Removes the Unit from this UnitRegistry and returns the pointer to that Unit.
    //! It does not delete the Unit rather just removes it from the registry.
    //! @return A pointer to the Unit that is no longer within this registry.
    UNITS_EXPORT UnitP RemoveUnit(Utf8CP unitName);

    //! Removes the Unit from this UnitRegistry and returns the pointer to that Unit.
    //! It does not delete the Unit rather just removes it from the registry.
    //! @return A pointer to the Unit that is no longer within this registry.
    UNITS_EXPORT UnitP RemoveInvertedUnit(Utf8CP unitName);

    //! Removes the constant from this UnitRegistry and returns the pointer to that constant.
    //! It does not delete the constant rather just removes it from the registry.
    //! @return A pointer to the constant that is no longer within this registry.
    UNITS_EXPORT UnitP RemoveConstant(Utf8CP constantName);

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
};

/** @endGroup */
END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
