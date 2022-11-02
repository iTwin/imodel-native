/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//*** WIP_FOREIGN_FORMAT
BENTLEY_NAMESPACE_TYPEDEFS (BeTextFile)

/*---------------------------------------------------------------------------------**//**
* Standard Unit table
+---------------+---------------+---------------+---------------+---------------+------*/
struct Dgn::StandardUnitTableEntry
{
UnitBase        m_base;
UnitSystem      m_system;
double          m_numerator;
double          m_denominator;
StandardUnit    m_standardNumber;
Utf8String      m_label;
Utf8String      m_singularName;
Utf8String      m_pluralName;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetLabel() const
    {
    return m_label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetName(bool bSingular) const
    {
    if (bSingular)
        return m_singularName;

    return m_pluralName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  ToUnitDefinition() const
    {
    return UnitDefinition(m_base, m_system, m_numerator, m_denominator, GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ScaleFactorMatches(UnitDefinitionCR unitDef) const
    {
    if (m_base != unitDef.GetBase())
        return false;

    return (0 == UnitDefinition::CompareRatios(m_numerator, m_denominator, unitDef.GetNumerator(), unitDef.GetDenominator()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int     Compare(UnitDefinition const& unitDef) const
    {
    if (m_base != unitDef.GetBase())
        return -1;

    // returns -1 if this unit is larger than the "info"
    return UnitDefinition::CompareRatios(m_numerator, m_denominator, unitDef.GetNumerator(), unitDef.GetDenominator());
    }

};

typedef struct StandardUnitTableEntry const*       StandardUnitTableEntryCP;

// The entries in this table should be in ascending size within a particular system.
static StandardUnitTableEntry   s_standardUnits[] =
    {
    // Meter based
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1000000000000000.0,       1.0,                     StandardUnit::MetricFemtometers,   "fm",      "Femtometer",   "Femtometers", },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1000000000000.0,          1.0,                     StandardUnit::MetricPicometers,    "pm",      "Picometer",    "Picometers",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1000000000.0,             1.0,                     StandardUnit::MetricNanometers,    "nm",      "Nanometer",    "Nanometers",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1000000.0,                1.0,                     StandardUnit::MetricMicrometers,   "um",      "Micrometer",   "Micrometers", },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1000.0,                   1.0,                     StandardUnit::MetricMillimeters,   "mm",      "Millimeter",   "Millimeters", },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     100.0,                    1.0,                     StandardUnit::MetricCentimeters,   "cm",      "Centimeter",   "Centimeters", },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     10.0,                     1.0,                     StandardUnit::MetricDecimeters,    "dm",      "Decimeter",    "Decimeters",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1.0,                     StandardUnit::MetricMeters,        "m",       "Meter",        "Meters",      },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      10.0,                    StandardUnit::MetricDekameters,    "dam",     "Dekameter",    "Dekameters",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      100.0,                   StandardUnit::MetricHectometers,   "hm",      "Hectometer",   "Hectometers", },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1000.0,                  StandardUnit::MetricKilometers,    "km",      "Kilometer",    "Kilometers",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1000000.0,               StandardUnit::MetricMegameters,    "Mm",      "Megameter",    "Megameters",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1000000000.0,            StandardUnit::MetricGigameters,    "Gm",      "Gigameter",    "Gigameters",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1000000000000.0,         StandardUnit::MetricTerameters,    "Tm",      "Terameter",    "Terameters",  },
    {UnitBase::Meter,     Dgn::UnitSystem::Metric,     1.0,                      1000000000000000.0,      StandardUnit::MetricPetameters,    "Pm",      "Petameter",    "Petameters",  },

    // International foot based.
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000000000.0,            254.0,                   StandardUnit::EnglishMicroInches,  "ui",      "MicroInch",    "MicroInches", },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000000.0,               254.0,                   StandardUnit::EnglishMils,         "mil",     "Mil",          "Mils",        },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0 * 72.0,           254.0,                   StandardUnit::EnglishPoints,       "pt",      "Point",        "Points",      },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0 * 6.0,            254.0,                   StandardUnit::EnglishPicas,        "pica",    "Pica",         "Picas",       },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0,                  254.0,                   StandardUnit::EnglishInches,       "\"",      "Inch",         "Inches",      },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0,                  254.0 * 12.0,            StandardUnit::EnglishFeet,         "'",       "Foot",         "Feet",        },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0,                  254.0 * 12.0 * 3.0,      StandardUnit::EnglishYards,        "yd",      "Yard",         "Yards",       },
    {UnitBase::Meter,     Dgn::UnitSystem::English,    10000.0,                  254.0 * 12.0 * 5280.0,   StandardUnit::EnglishMiles,        "mi",      "Mile",         "Miles",       },

    // US Survey foot based.
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  1000.0,                  StandardUnit::EnglishSurveyInches, "si",      "US Survey Inch",   "US Survey Inches",},
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0,                 StandardUnit::EnglishSurveyFeet,   "sf",      "US Survey Foot",   "US Survey Feet",  },
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0 * 6.0,           StandardUnit::EnglishFathoms,      "fat",     "Fathom",           "Fathoms",     },
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0 * 16.5,          StandardUnit::EnglishRods,         "rd",      "Rod",              "Rods",        },
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0 * 66.0,          StandardUnit::EnglishChains,       "ch",      "Chain",            "Chains",      },
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0 * 660.0,         StandardUnit::EnglishFurlongs,     "fur",     "Furlong",          "Furlongs",    },
    {UnitBase::Meter,     Dgn::UnitSystem::USSurvey,   39370.0,                  12000.0 * 5280.0,        StandardUnit::EnglishSurveyMiles,  "sm",      "US Survey Mile",   "US Survey Miles", },

    // No System
    {UnitBase::Meter,     Dgn::UnitSystem::Undefined,  10000000000.0,            1.0,                     StandardUnit::NoSystemAngstroms,         "A",         "Angstrom",             "Angstroms",        },
    {UnitBase::Meter,     Dgn::UnitSystem::Undefined,  1.0,                      1852.0,                  StandardUnit::NoSystemNauticalMiles,     "nm",        "Nautical Mile",        "Nautical Miles",    },
    {UnitBase::Meter,     Dgn::UnitSystem::Undefined,  1.0,                      149597900000.0,          StandardUnit::NoSystemAstronomicalUnits, "AU",        "Astronomical Unit",    "Astronomical Units",},
    {UnitBase::Meter,     Dgn::UnitSystem::Undefined,  1.0,                      9460730000000000.0,      StandardUnit::NoSystemLightYears,        "l.y.",      "Light Year",           "Light Years",       },
    {UnitBase::Meter,     Dgn::UnitSystem::Undefined,  1.0,                      30856780000000000.0,     StandardUnit::NoSystemParsecs,           "pc",        "Parsec",               "Parsecs",          },

    // Angular
    {UnitBase::Degree,    Dgn::UnitSystem::Undefined,  3600.0,                   1.0,                     StandardUnit::AngleSeconds,        "sec",      "Second",       "Seconds", },
    {UnitBase::Degree,    Dgn::UnitSystem::Undefined,  60.0,                     1.0,                     StandardUnit::AngleMinutes,        "min",      "Minute",       "Minutes", },
    {UnitBase::Degree,    Dgn::UnitSystem::Undefined,  10.0,                     9.0,                     StandardUnit::AngleGrads,          "grad",     "Grad",         "Grads",   },
    {UnitBase::Degree,    Dgn::UnitSystem::Undefined,  1.0,                      1.0,                     StandardUnit::AngleDegrees,        "deg",      "Degree",       "Degrees", },
    {UnitBase::Degree,    Dgn::UnitSystem::Undefined,  3.1415926535897932846,    180.0,                   StandardUnit::AngleRadians,        "rad",      "Radian",       "Radians", },

    // No Base
     {UnitBase::None,      Dgn::UnitSystem::Undefined,  1.0,                      1.0,                     StandardUnit::UnitlessWhole,       "uu", "Unitless Unit", "Unitless Units",},
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct StandardUnitTable
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetFirst()
    {
    return s_standardUnits;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetLast()
    {
    return s_standardUnits + _countof(s_standardUnits) - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetNext(StandardUnitTableEntryCP standardUnit)
    {
    if (NULL == standardUnit)
        return NULL;

    if (standardUnit >= s_standardUnits + _countof(s_standardUnits) - 1)
        return NULL;

    return ++standardUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetPrevious(StandardUnitTableEntryCP standardUnit)
    {
    if (NULL == standardUnit)
        return NULL;

    if (standardUnit <= s_standardUnits)
        return NULL;

    return --standardUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindByNumber(StandardUnit unitNum)
    {
    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof(s_standardUnits); iUnit++, standardUnit++)
        {
        if (standardUnit->m_standardNumber == unitNum)
            return standardUnit;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindMatchingScaleFactor(UnitDefinitionCR unitDef)
    {
    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof(s_standardUnits); iUnit++, standardUnit++)
        {
        if (standardUnit->ScaleFactorMatches(unitDef))
            return standardUnit;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindByName(Utf8CP name)
    {
    if (NULL == name)
        {BeAssert(false); return NULL;}

    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof(s_standardUnits); iUnit++, standardUnit++)
        {
        // Try to match either the singular or plural name
        if (standardUnit->GetName(true).EqualsI (name) ||
            standardUnit->GetName(false).EqualsI (name) )
            {
            return standardUnit;
            }
        }

    return NULL;
    }

}; //StandardUnitTable

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  UnitDefinition::GetStandardUnit(StandardUnit iUnit)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber(iUnit);

    if (NULL == unitEntry)
        return UnitDefinition();

    return unitEntry->ToUnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  UnitDefinition::GetStandardUnitByName(Utf8CP name)
    {
    if (NULL == name)
        {BeAssert(false); return UnitDefinition();}

    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByName(name);

    if (NULL == unitEntry)
        return UnitDefinition();

    return unitEntry->ToUnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      UnitDefinition::GetStandardLabel(StandardUnit iUnit)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber(iUnit);

    if (NULL == unitEntry)
        return "";

    return unitEntry->GetLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      UnitDefinition::GetStandardName(StandardUnit iUnit, bool singular)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber(iUnit);

    if (NULL == unitEntry)
        return "";

    return unitEntry->GetName(singular);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit    UnitDefinition::IsStandardUnit() const
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindMatchingScaleFactor(*this);

    if (NULL == unitEntry)
        return StandardUnit::None;

    return unitEntry->m_standardNumber;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitIteratorOptions::UnitIteratorOptions()
    {
    m_orderAscending        = true;
    m_allowBaseNone         = true;
    m_allowBaseMeter        = true;
    m_allowBaseDegree       = true;

    m_allowSystemNone       = true;
    m_allowSystemMetric     = true;
    m_allowSystemEnglish    = true;
    m_allowSystemUSSurvey   = true;

    m_allowLarger           = true;
    m_allowSmaller          = true;
    m_allowEqual            = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            UnitIteratorOptions::IsBaseAllowed(UnitBase base) const
    {
    switch (base)
        {
        case UnitBase::None:   return m_allowBaseNone;
        case UnitBase::Meter:  return m_allowBaseMeter;
        case UnitBase::Degree: return m_allowBaseDegree;
        }

    BeAssert(0);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            UnitIteratorOptions::IsSystemAllowed(Dgn::UnitSystem system) const
    {
    switch (system)
        {
        case Dgn::UnitSystem::Undefined:   return m_allowSystemNone;
        case Dgn::UnitSystem::English:     return m_allowSystemMetric;
        case Dgn::UnitSystem::Metric:      return m_allowSystemEnglish;
        case Dgn::UnitSystem::USSurvey:    return m_allowSystemUSSurvey;
        }

    BeAssert(0);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetOrderAscending()  {m_orderAscending = true;  }
void            UnitIteratorOptions::SetOrderDescending() {m_orderAscending = false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::DisallowAllBases()
    {
    m_allowBaseNone   = false;
    m_allowBaseMeter  = false;
    m_allowBaseDegree = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowAdditionalBase(UnitBase base)
    {
    switch (base)
        {
        case UnitBase::None:   m_allowBaseNone   = true;    return;
        case UnitBase::Meter:  m_allowBaseMeter  = true;    return;
        case UnitBase::Degree: m_allowBaseDegree = true;    return;
        }

    BeAssert(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowSingleBase(UnitBase base)
    {
    DisallowAllBases();
    SetAllowAdditionalBase(base);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::DisallowAllSystems()
    {
    m_allowSystemNone       = false;
    m_allowSystemMetric     = false;
    m_allowSystemEnglish    = false;
    m_allowSystemUSSurvey   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowAdditionalSystem(Dgn::UnitSystem system)
    {
    switch (system)
        {
        case Dgn::UnitSystem::Undefined:     m_allowSystemNone     = true;   return;
        case Dgn::UnitSystem::English:       m_allowSystemMetric   = true;   return;
        case Dgn::UnitSystem::Metric:        m_allowSystemEnglish  = true;   return;
        case Dgn::UnitSystem::USSurvey:      m_allowSystemUSSurvey = true;   return;
        }

    BeAssert(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowSingleSystem(Dgn::UnitSystem system)
    {
    DisallowAllSystems();
    SetAllowAdditionalSystem(system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetSizeCriteria(UnitDefinitionCR unitDef, UnitCompareMethod method)
    {
    m_compareUnit = unitDef;
    SetAllowSingleBase(unitDef.GetBase());

    m_allowLarger  = false;
    m_allowSmaller = false;
    m_allowEqual   = false;

    switch (method)
        {
        case SIZECOMPARE_AllowSmallerOrEqual:
            {
            m_allowEqual = true;
            // FALLTHRU
            }
        case SIZECOMPARE_AllowSmaller:
            {
            m_allowSmaller = true;
            break;
            }
        case SIZECOMPARE_AllowLargerOrEqual:
            {
            m_allowEqual = true;
            // FALLTHRU
            }
        case SIZECOMPARE_AllowLarger:
            {
            m_allowLarger = true;
            break;
            }
        }
    }

UnitDefinition   StandardUnitCollection::Entry::GetUnitDef()    const {return m_tableP ? m_tableP->ToUnitDefinition() : UnitDefinition();}
StandardUnit     StandardUnitCollection::Entry::GetNumber()     const {return m_tableP ? m_tableP->m_standardNumber   : StandardUnit::None;}
Utf8String       StandardUnitCollection::Entry::GetName(bool singular) const {return m_tableP ? m_tableP->GetName(singular) : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator::const_iterator(UnitIteratorOptionsCR options)
    :
    m_options(&options)
    {
    if (m_options->IsOrderAscending())
        m_entry.m_tableP = StandardUnitTable::GetFirst();
    else
        m_entry.m_tableP = StandardUnitTable::GetLast();

    // Find the first valid unit
    if ( ! UnitValidForIterator(m_entry.m_tableP))
        ++(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator::const_iterator()
    :
    m_options(NULL)
    {
    m_entry.m_tableP = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    StandardUnitCollection::const_iterator::UnitValidForIterator(StandardUnitTableEntryCP tableEntry)
    {
    if ( ! m_options->IsBaseAllowed(tableEntry->m_base))
        return false;

    if ( ! m_options->IsSystemAllowed(tableEntry->m_system))
        return false;

    UnitDefinitionCR compareUnit = m_options->GetCompareUnit();

    if (compareUnit.IsValid())
        {
        int iComparison = tableEntry->Compare(compareUnit);

        // iComp is reversed: positive if candidate is smaller
        if (0 > iComparison && ! m_options->GetCompareAllowLarger())
            return false;

        if (0 == iComparison && ! m_options->GetCompareAllowEqual())
            return false;

        if (0 < iComparison && ! m_options->GetCompareAllowSmaller())
            return false;
        }

    return true;  // all tests passed, element is acceptable
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator& StandardUnitCollection::const_iterator::operator ++ ()
    {
    while (NULL != m_entry.m_tableP)
        {
        if (m_options->IsOrderAscending())
             m_entry.m_tableP = StandardUnitTable::GetNext( m_entry.m_tableP);
        else
             m_entry.m_tableP = StandardUnitTable::GetPrevious( m_entry.m_tableP);

        if (NULL ==  m_entry.m_tableP || UnitValidForIterator( m_entry.m_tableP))
            break;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandardUnitCollection::const_iterator::operator != (StandardUnitCollection::const_iterator const& rhs) const
    {
    return m_entry.m_tableP != rhs.m_entry.m_tableP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::Entry const&  StandardUnitCollection::const_iterator::operator * () const
    {
    return m_entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      StandardUnitCollection::StandardUnitCollection(UnitIteratorOptionsCR options)
    :
    m_options(options)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator    StandardUnitCollection::begin() const
    {
    return const_iterator(m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator    StandardUnitCollection::end() const
    {
    return const_iterator();
    }


// -=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//                              User Units
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


#if defined (WIP_CFGVAR)
static const Utf8CP    MS_UNITS_SHOWALL            = L"MS_UNITS_SHOWALL";
#endif

DEFINE_POINTER_SUFFIX_TYPEDEFS(UserUnitTable)
DGNPLATFORM_TYPEDEFS (UserUnitTableEntry)
typedef std::vector <UserUnitTableEntry>    UnitEntryVector;

/*----------------------------------------------------------------------+
| Structure used for entries in the table of available units.           |
+----------------------------------------------------------------------*/
struct Dgn::UserUnitTableEntry
    {
    int             m_number;
    UnitDefinition  m_unitDef;
    Utf8String      m_singularName;
    Utf8String      m_pluralName;
    Utf8String      m_labels;               // comma-separated list of labels

    UserUnitTableEntry(UnitDefinitionCR unitDef, int unitNum, Utf8CP singularName, Utf8CP pluralName, Utf8String labels)
        : m_unitDef(unitDef), m_number(unitNum), m_singularName(singularName), m_pluralName(pluralName), m_labels(labels) {}
    };

/*---------------------------------------------------------------------------------**//**
* units.def is parsed once and its contents cached here.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserUnitTable : DgnHost::IHostObject
    {
private:
    UnitEntryVector         m_unitVector;
    bool                    m_containsStandardUnits;

    UserUnitTable() : m_containsStandardUnits(false)   {}

    bool                    ParseLine(Utf8StringR labels, Utf8StringR singularName, Utf8StringR pluralName, double& numerator, double& denominator, uint32_t& base, uint32_t& system, Utf8CP input) const;
    StatusInt               AddEntry(UserUnitTableEntryCR newEntry);
    void                    PopulateFromFile();
    void                    PopulateFromStandardUnits();

    static int              GetBasePriority(UnitDefinitionCR unitDef);
    static DgnHost::Key&    GetHostKey()                                {static DgnHost::Key key; return key;}
public:
    bool                    ContainsStandardUnits() const   {return m_containsStandardUnits;}
    size_t                  GetCount() const            {return m_unitVector.size();}
    void                    Dump() const;

    UserUnitTableEntryCP    GetEntry(size_t index) const {return (index < GetCount()) ? &m_unitVector[index] : NULL;}
    UserUnitTableEntryCP    FindByName(Utf8CP name) const;
    UserUnitTableEntryCP    FindByScale(UnitDefinitionCR unitDef) const;
    UserUnitTableEntryCP    FindByNumber(int unitNumber) const;
    UserUnitTableEntryCP    FindByLabel(Utf8CP label) const;

    static UserUnitTable&   GetTable();

    template <class V>
    UserUnitTableEntryCP    VisitUnits(V& v) const
        {
        FOR_EACH (UserUnitTableEntryCR entry, m_unitVector)
            if (v.Accept(entry))
                return &entry;

        return NULL;
        }

    static void             ResetForTest();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeStrippedString(Utf8StringR str, Utf8CP start, Utf8CP end /*inclusive*/)
    {
    while (*start && isspace(*start))
        ++start;
    while (*end && end > start && isspace(*end))
        --end;

    str = Utf8String(start, end+1);
    }

namespace UnitVisitors
    {
    struct ByNameFinder
        {
        Utf8CP          m_name;
        bool            Accept(UserUnitTableEntryCR entry)     {return entry.m_singularName.EqualsI(m_name) || entry.m_pluralName.EqualsI (m_name);}
        };

    struct ByLabelFinder
        {
        Utf8CP          m_label;
        bool            Accept(UserUnitTableEntryCR entry)
            {
            Utf8CP      labelList = entry.m_labels.c_str(), endOfList = labelList + strlen(labelList), cur = labelList, next;
            Utf8String  thisLabel;
            while (cur < endOfList)
                {
                next = ::strchr(cur, ',');
                if (NULL == next)
                    next = endOfList;

                makeStrippedString(thisLabel, cur, next-1);
                if (thisLabel.EqualsI (m_label))
                    return true;

                cur = next+1;
                }

            return false;
            }
        };

    struct ByScaleFinder
        {
        UnitDefinitionCR    m_unitDef;
        bool                Accept(UserUnitTableEntryCR entry) {return 0 == m_unitDef.CompareByScale(entry.m_unitDef);}
        };

    struct ByNumberFinder
        {
        int num;
        bool Accept(UserUnitTableEntryCR entry)    {return entry.m_number == num;}
        };

    struct ClosestFinder
        {
        UnitDefinitionCR unit;
        bool             ascending;

        bool Accept(UserUnitTableEntryCR entry)
            {return unit.GetBase() == entry.m_unitDef.GetBase() && unit.GetSystem() == entry.m_unitDef.GetSystem() && 0 < unit.CompareByScale(entry.m_unitDef);}

        ClosestFinder(UnitDefinitionCR u, bool a) : unit(u), ascending(a) {}
        };

    struct Dumper
        {
        int iEntry;
        Dumper() : iEntry(0) {}

        bool Accept(UserUnitTableEntryCR unitEntry)
            {
            printf("  %d:\t%d %s (%f / %f)\n", iEntry, unitEntry.m_number, unitEntry.m_pluralName.c_str(), unitEntry.m_unitDef.GetNumerator(), unitEntry.m_unitDef.GetDenominator());
            iEntry++;
            return false;
            }
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByName(Utf8CP name) const
    {
    UnitVisitors::ByNameFinder v = {name };
    return VisitUnits(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByLabel(Utf8CP label) const
    {
    UnitVisitors::ByLabelFinder v = {label };
    return VisitUnits(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByScale(UnitDefinitionCR unitDef) const
    {
    UnitVisitors::ByScaleFinder v = {unitDef };
    return VisitUnits(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByNumber(int unitNumber) const
    {
    UnitVisitors::ByNumberFinder v = {unitNumber };
    return VisitUnits(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    UserUnitTable::Dump() const
    {
    printf("Dumping units table, %zu entries:\n", m_unitVector.size());
    UnitVisitors::Dumper v;
    VisitUnits(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int             UserUnitTable::GetBasePriority
(
UnitDefinitionCR    unitDef
)
    {
    switch (unitDef.GetBase())
        {
        default:
        case UnitBase::None:        return 0;
        case UnitBase::Degree:      return 1;
        case UnitBase::Meter:       return 2;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       UserUnitTable::AddEntry
(
UserUnitTableEntryCR    newEntry        /* => add this to the table */
)
    {
    bool                        found = false;
    UnitEntryVector::iterator   iter;
    UnitDefinitionCR            newUnit = newEntry.m_unitDef;
    int                         newPriority = GetBasePriority(newUnit);

    for (iter = m_unitVector.begin(); iter != m_unitVector.end(); ++iter)
        {
        UnitDefinitionCR    existingUnit = iter->m_unitDef;
        int                 basePriority = GetBasePriority(existingUnit);

        // Keep the units together by base in order: Meter, Degree, Unitless
        if (basePriority != newPriority)
            {
            if (basePriority > newPriority)
                continue;
            else
                break;
            }

        int iCompare = newUnit.CompareByScale(existingUnit);

        // Do not allow multiple equal size units
        if (0 == iCompare)
            {
            found = true;
            break;
            }

        // Within a base, put the smaller units first
        if (0 < iCompare)
            break;
        }

    if (found)
        return ERROR;

    m_unitVector.insert(iter, newEntry);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTable& UserUnitTable::GetTable()
    {
    DgnHost::Key& key = GetHostKey();
    UserUnitTableP table = dynamic_cast<UserUnitTableP> (T_HOST.GetHostObject(key));
    if (NULL == table)
        {
        table = new UserUnitTable();
        table->PopulateFromFile();

#if defined (WIP_CFGVAR) // MS_UNITS_SHOWALL
        if (ConfigurationManager::IsVariableDefined(MS_UNITS_SHOWALL))
#endif
            table->PopulateFromStandardUnits();

        T_HOST.SetHostObject(key, table);
        }

    BeAssert(NULL != table);
    return *table;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::ResetForTest()
    {
    DgnHost::Key& key = GetHostKey();
    UserUnitTableP table = static_cast<UserUnitTableP> (T_HOST.GetHostObject(key));
    if (NULL != table)
        {
        T_HOST.SetHostObject(key, NULL);
        delete table;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::PopulateFromStandardUnits()
    {
    m_containsStandardUnits = true;
    Dgn::UnitSystem  unitSystem[] = {Dgn::UnitSystem::Metric, Dgn::UnitSystem::English, Dgn::UnitSystem::USSurvey, Dgn::UnitSystem::Undefined };
    for (int iSystem = 0; iSystem < _countof(unitSystem); iSystem++)
        {
        UnitIteratorOptions options;
        options.SetAllowSingleSystem(unitSystem[iSystem]);

        StandardUnitCollection collection(options);

        FOR_EACH (StandardUnitCollection::Entry const& standardUnit, collection)
            {
            UnitDefinition unitDef = standardUnit.GetUnitDef();
            UserUnitTableEntry  entry(unitDef, (int)standardUnit.GetNumber(),
                                    standardUnit.GetName(true).c_str(), standardUnit.GetName(false).c_str(), unitDef.GetLabelCP());

            AddEntry(entry);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::PopulateFromFile()
    {
// *** WIP_ForeignFormat - We should move this code into foreignformat.

#if defined (WIP_CFGVAR) // MS_CUSTOMUNITDEF
    WString filename;
    if (SUCCESS != ConfigurationManager::GetVariable(filename, L"MS_CUSTOMUNITDEF"))
        return;

    BeFileStatus fileOpenStatus;
    BeTextFilePtr file = BeTextFile::Open(fileOpenStatus, filename.c_str(), TextFileOpenType::Read, TextFileOptions::None);
    if (file.IsNull() || BeFileStatus::Success != fileOpenStatus)
        return;

    UnitInfo        unitInfo;
    WString         nameSingular, namePlural, labels;
    int             customUnitIndex = 0;
    while (SUCCESS == GetNextUserFileUnitDef(unitInfo, nameSingular, namePlural, labels, *file))
        {
        UnitDefinition unitDef(unitInfo);
        int unitNumber = unitDef.IsStandardUnit();
        if (StandardUnit::None == unitNumber || StandardUnit::Custom == unitNumber)
            unitNumber = StandardUnit::Custom + 1 + customUnitIndex++;

        UserUnitTableEntry entry(unitDef, unitNumber, nameSingular.c_str(), namePlural.c_str(), labels);
        AddEntry(entry);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitTable::ParseLine(Utf8StringR labels, Utf8StringR singularName, Utf8StringR pluralName, double& numerator, double& denominator, uint32_t& base, uint32_t& system, Utf8CP input) const
    {
    static const Utf8Char separator = ';';
    Utf8CP a = input,
            b = a ? ::strchr(a, separator)   : NULL,
            c = b ? ::strchr(b+1, separator) : NULL,
            d = c ? ::strchr(c+1, separator) : NULL,
            e = d ? ::strchr(d+1, separator) : NULL,
            f = e ? ::strchr(e+1, separator) : NULL,
            g = f ? ::strchr(f+1, separator) : NULL;

    if (NULL == g)
        return false;

    makeStrippedString(labels, a, b-1);
    makeStrippedString(singularName, b+1, c-1);
    makeStrippedString(pluralName, c+1, d-1);

    return  1 == Utf8String::Sscanf_safe (d+1, "%lf", &numerator) &&
            1 == Utf8String::Sscanf_safe (e+1, "%lf", &denominator) &&
            1 == Utf8String::Sscanf_safe (f+1, "%d", &base) &&
            1 == Utf8String::Sscanf_safe (g+1, "%d", &system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UserUnitCollection::Entry::GetUnitDef() const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry(m_index);
    return entry ? entry->m_unitDef : UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit UserUnitCollection::Entry::GetNumber() const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry(m_index);
    return entry ? (StandardUnit)entry->m_number : StandardUnit::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserUnitCollection::Entry::GetName(bool singular) const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry(m_index);
    return entry ? (singular ? entry->m_singularName : entry->m_pluralName) : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator::const_iterator(UnitIteratorOptionsCR options) : m_options(&options)
    {
    UserUnitTableR unitTable = UserUnitTable::GetTable();
    if (m_options->IsOrderAscending())
        m_entry.m_index = 0;
    else
        m_entry.m_index = unitTable.GetCount() - 1;

    // Find the first valid unit
    if (IndexIsValid(m_entry.m_index) && ! UnitValidForIterator(m_entry.m_index))
        ++(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator::const_iterator(bool ascending) : m_options(NULL)
    {
    // Create an invalid iterator
    if (ascending)
        m_entry.m_index = UserUnitTable::GetTable().GetCount();
    else
        m_entry.m_index = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::const_iterator::UnitValidForIterator(size_t tableIndex)
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry(tableIndex);
    if (NULL == entry)
        return false;

    UnitDefinitionCR unit = entry->m_unitDef;
    if (!m_options->IsBaseAllowed(unit.GetBase()) || !m_options->IsSystemAllowed(unit.GetSystem()))
        return false;

    UnitDefinitionCR compare = m_options->GetCompareUnit();
    if (compare.IsValid())
        {
        int iComparison = unit.CompareByScale(compare);

        // iComp is reversed: positive if candidate is smaller
        if ((0 > iComparison && !m_options->GetCompareAllowLarger()) ||
            (0 == iComparison && !m_options->GetCompareAllowEqual()) ||
            ( 0 < iComparison && !m_options->GetCompareAllowSmaller()))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::IndexIsValid(size_t tableIndex)
    {
    return tableIndex < UserUnitTable::GetTable().GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator& UserUnitCollection::const_iterator::operator ++ ()
    {
    while (IndexIsValid(m_entry.m_index))
        {
        m_entry.m_index += m_options->IsOrderAscending() ? 1 : -1;
        if (!IndexIsValid(m_entry.m_index) || UnitValidForIterator(m_entry.m_index))
            break;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::const_iterator::operator != (UserUnitCollection::const_iterator const& rhs) const
    {
    return m_entry.m_index != rhs.m_entry.m_index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ UserUnitCollection::UserUnitCollection(UnitIteratorOptionsCR options) : m_options(options)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::Entry UserUnitCollection::const_iterator::operator * () const
    {
    return m_entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator UserUnitCollection::begin() const
    {
    return const_iterator(m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator UserUnitCollection::end() const
    {
    return const_iterator(m_options.IsOrderAscending());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByNumber(int unitNumber, bool alsoStandard)
    {
    UserUnitTableR unitTable = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = unitTable.FindByNumber(unitNumber);
    if (NULL != entry)
        return entry->m_unitDef;
    else if (alsoStandard && !unitTable.ContainsStandardUnits())
        return UnitDefinition::GetStandardUnit((StandardUnit)unitNumber);
    else
        return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetNextLarger() const
    {
    UnitVisitors::ClosestFinder v(*this, true);
    UserUnitTableEntryCP found = UserUnitTable::GetTable().VisitUnits(v);
    return found ? found->m_unitDef : *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetNextSmaller() const
    {
    UnitVisitors::ClosestFinder v(*this, false);
    UserUnitTableEntryCP found = UserUnitTable::GetTable().VisitUnits(v);
    return found ? found->m_unitDef : *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int UnitDefinition::GetNumber(bool alsoStandard) const
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByScale(*this);
    if (NULL != entry)
        return entry->m_number;
    else if (alsoStandard && table.ContainsStandardUnits())
        return (int)this->IsStandardUnit();
    else
        return (int)StandardUnit::Custom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UnitDefinition::GetName(bool singular, bool alsoStandard) const
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByScale(*this);
    if (NULL != entry)
        return singular ? entry->m_singularName : entry->m_pluralName;

    StandardUnit unitNo = (alsoStandard && table.ContainsStandardUnits()) ? IsStandardUnit() : StandardUnit::Custom;
    return UnitDefinition::GetStandardName(unitNo, singular);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByName(Utf8CP unitName, bool alsoStandard)
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByName(unitName);
    if (NULL != entry)
        return entry->m_unitDef;

    if (alsoStandard && table.ContainsStandardUnits())
        return UnitDefinition::GetStandardUnitByName(unitName);

    return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByLabel(Utf8CP label, bool alsoStandard)
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByLabel(label);
    if (NULL != entry)
        return entry->m_unitDef;
    else if (alsoStandard && table.ContainsStandardUnits())
        {
        UnitIteratorOptions opts;
        FOR_EACH (StandardUnitCollection::Entry const& standardUnit, StandardUnitCollection(opts))
            {
            UnitDefinition def = standardUnit.GetUnitDef();
            if (def.GetLabel().EqualsI (label))
                return def;
            }
        }

    return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::BackDoor_ResetUserUnitsForTest()
    {
    UserUnitTable::ResetForTest();
    }

