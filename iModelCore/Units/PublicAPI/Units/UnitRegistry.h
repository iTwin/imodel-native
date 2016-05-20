/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitRegistry.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    Utf8Vector m_systems;
    bmap<Utf8String, PhenomenonP> m_phenomena;
    bmap<Utf8String, UnitP> m_units;
    bmap<uint64_t, Conversion> m_conversions;
    uint32_t m_nextId = 0;
    bmap<Utf8String, Utf8String> m_oldNameNewNameMapping;
    bmap<Utf8String, Utf8String> m_newNameOldNameMapping;

    UnitRegistry();
    UnitRegistry(const UnitRegistry& rhs) = delete;
    UnitRegistry & operator= (const UnitRegistry& rhs) = delete;

    void AddDefaultSystems ();
    void AddDefaultPhenomena ();
    void AddDefaultUnits ();
    void AddDefaultConstants();
    void AddDefaultMappings();

    void InsertUnique (Utf8Vector &vec, Utf8String &str);
    void AddSystem(Utf8CP systemName);
    void AddPhenomena(Utf8CP phenomenaName, Utf8CP definition);
    void AddBasePhenomena(Utf8Char baseSymbol);
    UnitCP AddUnitForBasePhenomenon(Utf8CP unitName, Utf8Char baseSymbol);

    UnitP AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant);

    PhenomenonP LookupPhenomenonP(Utf8CP name) const;
    UnitP LookupUnitP(Utf8CP name) const;
    UnitP AddUnitP(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0);

    bool NameConflicts(Utf8CP name);

    bool TryGetConversion(uint64_t index, Conversion& conversion);
    void AddConversion(uint64_t index, Conversion& conversion) { m_conversions.Insert(index, conversion); }

    void AddMapping(Utf8CP oldName, Utf8CP newName);
    bool HasConstant (Utf8CP constantName) const;

public:
    UNITS_EXPORT static UnitRegistry & Instance();
    UNITS_EXPORT static void Clear(); // TODO: Remove or hide so cannot be called from public API, only needed for performance testing

    UNITS_EXPORT void AllUnits(bvector<UnitCP>& allUnits) const;
    UNITS_EXPORT void AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const;
    UNITS_EXPORT void AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const;
    
    // Register methods.
    UNITS_EXPORT UnitCP AddDummyUnit(Utf8CP unitName);
    UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0);
    UnitCP AddInvertingUnit(Utf8CP parentUnitName, Utf8CP unitName);
    UnitCP AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor);
    BentleyStatus AddSynonym(UnitCP unit, Utf8CP synonymName);
    BentleyStatus AddSynonym(Utf8CP unitName, Utf8CP synonymName);
    
    // Lookup methods
    UNITS_EXPORT UnitCP LookupUnit(Utf8CP name) const;
    UNITS_EXPORT UnitCP LookupConstant(Utf8CP name) const;
    UNITS_EXPORT PhenomenonCP LookupPhenomenon(Utf8CP name) const { return LookupPhenomenonP(name); }
        
    // bool Exists methods.
    UNITS_EXPORT bool HasSystem (Utf8CP systemName) const;
    UNITS_EXPORT bool HasPhenomena (Utf8CP phenomenaName) const;
    UNITS_EXPORT bool HasUnit (Utf8CP unitName) const;

    //Mapping methods
    UNITS_EXPORT bool TryGetNewName(Utf8CP oldName, Utf8StringR newName) const;
    UNITS_EXPORT bool TryGetOldName(Utf8CP newName, Utf8StringR oldName) const;
    UNITS_EXPORT UnitCP LookupUnitUsingOldName(Utf8CP oldName) const;
    };

END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
