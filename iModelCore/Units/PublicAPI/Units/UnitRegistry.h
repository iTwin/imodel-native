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
    void AddSystem(Utf8CP name);
    void AddSystem(UnitSystemR unitSystem);
    void AddPhenomenon(Utf8CP phenomenaName, Utf8CP definition);
    void AddBasePhenomenon(Utf8Char baseSymbol);
    UnitCP AddUnitForBasePhenomenon(Utf8CP unitName, Utf8Char baseSymbol);

    UnitP AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant);

    PhenomenonP LookupPhenomenonP(Utf8CP name) const;
    UnitP LookupUnitP(Utf8CP name) const;
    UnitSystemP LookupUnitSystemP(Utf8CP name) const;

    UnitP AddUnitP(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0);

    bool NameConflicts(Utf8CP name);

    bool TryGetConversion(uint64_t index, Conversion& conversion);
    void AddConversion(uint64_t index, Conversion& conversion) {m_conversions.Insert(index, conversion);}

    bool HasConstant(Utf8CP constantName) const {return nullptr != LookupConstant(constantName);}

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
    //! @param[in] allUnitNames     The vector to populate with the unit names
    //! @param[in] includeSynonyms  If true, will include all units synonyms
    UNITS_EXPORT void AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const;

    //! Populates the provided vector with all Phenomena in the registry
    //! @param[in] allPhenomena The vector to populate with the phenomena
    UNITS_EXPORT void AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const;

    //! Populates the provided vector with all UnitSystems in the registry
    //! @param[in] allUnitSystems The vector to populate with the unit systems
    UNITS_EXPORT void AllSystems(bvector<UnitSystemCP>& allUnitSystems) const;

    // Register methods.
    UNITS_EXPORT UnitCP AddDummyUnit(Utf8CP unitName);
    UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0) {return AddUnitP(phenomName, systemName, unitName, definition, factor, offset);}
    UnitCP AddInvertingUnit(Utf8CP parentUnitName, Utf8CP unitName);
    UNITS_EXPORT UnitCP AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor);
    UNITS_EXPORT BentleyStatus AddSynonym(UnitCP unit, Utf8CP synonymName);
    UNITS_EXPORT BentleyStatus AddSynonym(Utf8CP unitName, Utf8CP synonymName);

    // Lookup methods
    UNITS_EXPORT UnitCP LookupUnit(Utf8CP name) const;
    UNITS_EXPORT UnitCP LookupConstant(Utf8CP name) const;
    UNITS_EXPORT PhenomenonCP LookupPhenomenon(Utf8CP name) const;
    UNITS_EXPORT UnitSystemCP LookupUnitSystem(Utf8CP name) const;

    // bool Exists methods.
    bool HasSystem (Utf8CP systemName) const {return m_systems.end() != m_systems.find(systemName);}
    bool HasPhenomena(Utf8CP phenomenaName) const {return m_phenomena.end() != m_phenomena.find(phenomenaName);}
    bool HasUnit(Utf8CP unitName) const {return m_units.end() != m_units.find(unitName);}

    //Mapping methods
    //Gets the "new" unit name from a legacy unit name
    UNITS_EXPORT static Utf8CP TryGetNewName(Utf8CP oldName);
    //Gets the legacy unit name for a unit name
    UNITS_EXPORT static Utf8CP TryGetOldName(Utf8CP newName);

    //Gets the EC compatible name for a unit name
    UNITS_EXPORT static Utf8CP TryGetECName(Utf8CP name);
    //Gets the mapped unit name for an EC compatible name
    UNITS_EXPORT static Utf8CP TryGetNameFromECName(Utf8CP ecName);

    UNITS_EXPORT UnitCP LookupUnitUsingOldName(Utf8CP oldName) const;
    UnitCP GetPlatformLengthUnit() {return LookupUnit("M");}
    UNITS_EXPORT size_t LoadSynonyms(Json::Value jval) const;
    UNITS_EXPORT PhenomenonCP LoadSynonym(Utf8CP unitName, Utf8CP synonym) const;
    UNITS_EXPORT Json::Value SynonymsToJson() const;
    UNITS_EXPORT UnitCP LookupUnitCI(Utf8CP name) const;
};

END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
