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
//! The UnitsContext is an abstact class to provide a generic interface to look up Units, 
//! Phenomena, and UnitSystems within a UnitsContext.
// @bsistruct                                                    Caleb.Shafer       02/18
//=======================================================================================
struct IUnitsContext /* abstract */
{
protected:
    virtual UnitP _LookupUnitP(Utf8CP name) = 0;
    virtual PhenomenonP _LookupPhenomenonP(Utf8CP name) = 0;
    virtual UnitSystemP _LookupUnitSystemP(Utf8CP name) = 0;
    virtual void _AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const = 0;
    virtual void _AllUnits(bvector<UnitCP>& allUnits) const = 0;
    virtual void _AllSystems(bvector<UnitSystemCP>& allUnitSystems) const = 0;

public:
    //! Gets the Unit from this context.
    //! @param[in] name Name of the Unit to retrieve.
    //! @return A Unit if it found in this context; otherwise, nullptr.
    virtual UnitCP LookupUnit(Utf8CP name) const {return const_cast<IUnitsContextP>(this)->_LookupUnitP(name);}

    //! Gets the Phenomenon from this context.
    //! @param[in] name Name of the Phenomenon to retrieve.
    //! @return A Phenomenon if it found in this context; otherwise, nullptr.
    virtual PhenomenonCP LookupPhenomenon(Utf8CP name) const {return const_cast<IUnitsContextP>(this)->_LookupPhenomenonP(name);}

    //! Gets the UnitSystem from this context.
    //! @param[in] name Name of the UnitSystem to retrieve.
    //! @return A UnitSystem if it found in this registry; otherwise, nullptr.
    virtual UnitSystemCP LookupUnitSystem(Utf8CP name) const {return const_cast<IUnitsContextP>(this)->_LookupUnitSystemP(name);}

    //! Populates the provided vector with all Phenomena in this context
    //! @param[out] allPhenomena The vector to populate with the phenomena
    void AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const {_AllPhenomena(allPhenomena);}

    //! Populates the provided vector with all UnitSystems in this context
    //! @param[out] allUnitSystems The vector to populate with the unit systems
    void AllSystems(bvector<UnitSystemCP>& allUnitSystems) const {_AllSystems(allUnitSystems);}

    //! Populates the provided vector with all Units in this context
    //! @param[in] allUnits The vector to populate with the units
    void AllUnits(bvector<UnitCP>& allUnits) const {_AllUnits(allUnits);}
};

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
struct UnitRegistry : IUnitsContext
{
friend struct Unit;
private:
    static UnitRegistry * s_instance;

    bmap<Utf8CP, UnitSystemP, less_str> m_systems;
    bmap<Utf8CP, PhenomenonP, less_str> m_phenomena;
    bmap<Utf8CP, UnitP, less_str> m_units;

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

        auto phenomena = PHENOM_TYPE::_Create(phenomenaName, definition);

        phenomena->m_unitsContext = this;

        m_phenomena.insert(bpair<Utf8CP, PHENOM_TYPE*>(phenomena->GetName().c_str(), phenomena));

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

        PhenomenonP phenomenon = _LookupPhenomenonP(phenomName);
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

        UNIT_TYPE* unit = UNIT_TYPE::_Create(*system, *phenomenon, unitName, definition, numerator, denominator, offset, isConstant);
        if (nullptr == unit)
            return nullptr;

        // Add the unit label as a synonym as long as it is not the same as the actual unit name
        if (!Utf8String::IsNullOrEmpty(unit->GetLabel().c_str()) && (0 != BeStringUtilities::StricmpAscii(unit->GetLabel().c_str(), unitName)))
            unit->AddSynonym(unit->GetLabel().c_str());

        unit->m_unitsContext = this;
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), (UnitP) unit));

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

        auto unit = UNIT_TYPE::_Create(*parentUnit, *unitSystem, unitName);
        if (nullptr == unit)
            return nullptr;

        unit->m_unitsContext = this;

        PhenomenonP phenomenon = _LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName().c_str());
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), (UnitP) unit));

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
        unitSystem->m_unitsContext = this;

        m_systems.Insert(unitSystem->GetName().c_str(), unitSystem);

        return unitSystem;
        }

protected:
    PhenomenonP _LookupPhenomenonP(Utf8CP name) override {auto val_iter = m_phenomena.find(name); return val_iter == m_phenomena.end() ? nullptr : (*val_iter).second;}
    void _AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const;

    UnitP _LookupUnitP(Utf8CP name) override {auto val_iter = m_units.find(name); return val_iter == m_units.end() ? nullptr : (*val_iter).second;}
    void _AllUnits(bvector<UnitCP>& allUnits) const;

    UnitSystemP _LookupUnitSystemP(Utf8CP name) override {auto val_iter = m_systems.find(name); return val_iter == m_systems.end() ? nullptr : (*val_iter).second;}
    void _AllSystems(bvector<UnitSystemCP>& allUnitSystems) const;

    bool NamedItemExists(Utf8CP name) {return HasUnit(name) || HasPhenomenon(name) || HasSystem(name);}

    UnitCP CreateDummyUnit(Utf8CP unitName);
public:
    //! Returns a pointer to the singleton instance of the UnitRegistry
    //!
    //! If the singleton has not yet been instantiated the UnitRegistry will be initialized with 
    //! a set of base Units, Phenomena, and Unit Systems. For more details see the description of the class.
    UNITS_EXPORT static UnitRegistry& Get();

    //! Clears the singleton instance of all definitions.
    UNITS_EXPORT static void Clear(); // TODO: Remove or hide so cannot be called from public API, only needed for performance testing

    //! Populates the provided vector with the name of all the Units in the registry. If includeSynonyms is true, all Unit synonym names 
    //! will be included.
    //! @param[out] allUnitNames     The vector to populate with the unit names
    //! @param[in] includeSynonyms  If true, will include all units synonyms
    UNITS_EXPORT void AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const;

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
