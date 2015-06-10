/*---------------------------------------------------------------------+
|
|   $Source: DgnCore/UnitManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//*** WIP_FOREIGN_FORMAT
BENTLEY_API_TYPEDEFS (BeTextFile)

/*---------------------------------------------------------------------------------**//**
* Standard Unit table
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnPlatform::StandardUnitTableEntry
{
UnitBase        m_base;
UnitSystem      m_system;
double          m_numerator;
double          m_denominator;
StandardUnit    m_standardNumber;
L10N::StringId  m_labelKey;
L10N::StringId m_singularNameKey;
L10N::StringId m_pluralNameKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WString GetLabel () const
    {
    return DgnCoreL10N::GetStringW(m_labelKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WString GetName (bool bSingular) const
    {
    if (bSingular)
        return DgnCoreL10N::GetStringW(m_singularNameKey);

    return DgnCoreL10N::GetStringW(m_pluralNameKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  ToUnitDefinition () const
    {
    return UnitDefinition (m_base, m_system, m_numerator, m_denominator, GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ScaleFactorMatches (UnitDefinitionCR unitDef) const
    {
    if (m_base != unitDef.GetBase())
        return false;

    return (0 == UnitDefinition::CompareRatios (m_numerator, m_denominator, unitDef.GetNumerator(), unitDef.GetDenominator()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
int     Compare (UnitDefinition const& unitDef) const
    {
    if (m_base != unitDef.GetBase())
        return -1;

    // returns -1 if this unit is larger than the "info"
    return UnitDefinition::CompareRatios (m_numerator, m_denominator, unitDef.GetNumerator(), unitDef.GetDenominator());
    }

};

typedef struct StandardUnitTableEntry const*       StandardUnitTableEntryCP;

// The entries in this table should be in ascending size within a particular system.
static StandardUnitTableEntry   s_standardUnits[] =
    {
    // Meter based
    { UnitBase::Meter,     UnitSystem::Metric,     1000000000000000.0,       1.0,                     StandardUnit::MetricFemtometers,   DgnCoreL10N::UNIT_LABEL_Femtometers(),  DgnCoreL10N::UNIT_SINGULAR_NAME_Femtometers(),  DgnCoreL10N::UNIT_PLURAL_NAME_Femtometers(), },
    { UnitBase::Meter,     UnitSystem::Metric,     1000000000000.0,          1.0,                     StandardUnit::MetricPicometers,    DgnCoreL10N::UNIT_LABEL_Picometers(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Picometers(),   DgnCoreL10N::UNIT_PLURAL_NAME_Picometers(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1000000000.0,             1.0,                     StandardUnit::MetricNanometers,    DgnCoreL10N::UNIT_LABEL_Nanometers(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Nanometers(),   DgnCoreL10N::UNIT_PLURAL_NAME_Nanometers(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1000000.0,                1.0,                     StandardUnit::MetricMicrometers,   DgnCoreL10N::UNIT_LABEL_Micrometers(),  DgnCoreL10N::UNIT_SINGULAR_NAME_Micrometers(),  DgnCoreL10N::UNIT_PLURAL_NAME_Micrometers(), },
    { UnitBase::Meter,     UnitSystem::Metric,     1000.0,                   1.0,                     StandardUnit::MetricMillimeters,   DgnCoreL10N::UNIT_LABEL_Millimeters(),  DgnCoreL10N::UNIT_SINGULAR_NAME_Millimeters(),  DgnCoreL10N::UNIT_PLURAL_NAME_Millimeters(), },
    { UnitBase::Meter,     UnitSystem::Metric,     100.0,                    1.0,                     StandardUnit::MetricCentimeters,   DgnCoreL10N::UNIT_LABEL_Centimeters(),  DgnCoreL10N::UNIT_SINGULAR_NAME_Centimeters(),  DgnCoreL10N::UNIT_PLURAL_NAME_Centimeters(), },
    { UnitBase::Meter,     UnitSystem::Metric,     10.0,                     1.0,                     StandardUnit::MetricDecimeters,    DgnCoreL10N::UNIT_LABEL_Decimeters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Decimeters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Decimeters(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1.0,                     StandardUnit::MetricMeters,        DgnCoreL10N::UNIT_LABEL_Meters(),       DgnCoreL10N::UNIT_SINGULAR_NAME_Meters(),       DgnCoreL10N::UNIT_PLURAL_NAME_Meters(),      },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      10.0,                    StandardUnit::MetricDekameters,    DgnCoreL10N::UNIT_LABEL_Dekameters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Dekameters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Dekameters(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      100.0,                   StandardUnit::MetricHectometers,   DgnCoreL10N::UNIT_LABEL_Hectometers(),  DgnCoreL10N::UNIT_SINGULAR_NAME_Hectometers(),  DgnCoreL10N::UNIT_PLURAL_NAME_Hectometers(), },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1000.0,                  StandardUnit::MetricKilometers,    DgnCoreL10N::UNIT_LABEL_Kilometers(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Kilometers(),   DgnCoreL10N::UNIT_PLURAL_NAME_Kilometers(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1000000.0,               StandardUnit::MetricMegameters,    DgnCoreL10N::UNIT_LABEL_Megameters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Megameters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Megameters(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1000000000.0,            StandardUnit::MetricGigameters,    DgnCoreL10N::UNIT_LABEL_Gigameters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Gigameters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Gigameters(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1000000000000.0,         StandardUnit::MetricTerameters,    DgnCoreL10N::UNIT_LABEL_Terameters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Terameters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Terameters(),  },
    { UnitBase::Meter,     UnitSystem::Metric,     1.0,                      1000000000000000.0,      StandardUnit::MetricPetameters,    DgnCoreL10N::UNIT_LABEL_Petameters(),   DgnCoreL10N::UNIT_SINGULAR_NAME_Petameters(),   DgnCoreL10N::UNIT_PLURAL_NAME_Petameters(),  },

    // International foot based.
    { UnitBase::Meter,     UnitSystem::English,    10000000000.0,            254.0,                   StandardUnit::EnglishMicroInches,  DgnCoreL10N::UNIT_LABEL_MicroInches(),  DgnCoreL10N::UNIT_SINGULAR_NAME_MicroInches(),  DgnCoreL10N::UNIT_PLURAL_NAME_MicroInches(), },
    { UnitBase::Meter,     UnitSystem::English,    10000000.0,               254.0,                   StandardUnit::EnglishMils,         DgnCoreL10N::UNIT_LABEL_Mils(),         DgnCoreL10N::UNIT_SINGULAR_NAME_Mils(),         DgnCoreL10N::UNIT_PLURAL_NAME_Mils(),        },
    { UnitBase::Meter,     UnitSystem::English,    10000.0 * 72.0,           254.0,                   StandardUnit::EnglishPoints,       DgnCoreL10N::UNIT_LABEL_Points(),       DgnCoreL10N::UNIT_SINGULAR_NAME_Points(),       DgnCoreL10N::UNIT_PLURAL_NAME_Points(),      },
    { UnitBase::Meter,     UnitSystem::English,    10000.0 * 6.0,            254.0,                   StandardUnit::EnglishPicas,        DgnCoreL10N::UNIT_LABEL_Picas(),        DgnCoreL10N::UNIT_SINGULAR_NAME_Picas(),        DgnCoreL10N::UNIT_PLURAL_NAME_Picas(),       },
    { UnitBase::Meter,     UnitSystem::English,    10000.0,                  254.0,                   StandardUnit::EnglishInches,       DgnCoreL10N::UNIT_LABEL_Inches(),       DgnCoreL10N::UNIT_SINGULAR_NAME_Inches(),       DgnCoreL10N::UNIT_PLURAL_NAME_Inches(),      },
    { UnitBase::Meter,     UnitSystem::English,    10000.0,                  254.0 * 12.0,            StandardUnit::EnglishFeet,         DgnCoreL10N::UNIT_LABEL_Feet(),         DgnCoreL10N::UNIT_SINGULAR_NAME_Feet(),         DgnCoreL10N::UNIT_PLURAL_NAME_Feet(),        },
    { UnitBase::Meter,     UnitSystem::English,    10000.0,                  254.0 * 12.0 * 3.0,      StandardUnit::EnglishYards,        DgnCoreL10N::UNIT_LABEL_Yards(),        DgnCoreL10N::UNIT_SINGULAR_NAME_Yards(),        DgnCoreL10N::UNIT_PLURAL_NAME_Yards(),       },
    { UnitBase::Meter,     UnitSystem::English,    10000.0,                  254.0 * 12.0 * 5280.0,   StandardUnit::EnglishMiles,        DgnCoreL10N::UNIT_LABEL_Miles(),        DgnCoreL10N::UNIT_SINGULAR_NAME_Miles(),        DgnCoreL10N::UNIT_PLURAL_NAME_Miles(),       },

    // US Survey foot based.
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  1000.0,                  StandardUnit::EnglishSurveyInches, DgnCoreL10N::UNIT_LABEL_SurveyInches(), DgnCoreL10N::UNIT_SINGULAR_NAME_SurveyInches(), DgnCoreL10N::UNIT_PLURAL_NAME_SurveyInches(),},
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0,                 StandardUnit::EnglishSurveyFeet,   DgnCoreL10N::UNIT_LABEL_SurveyFeet(),   DgnCoreL10N::UNIT_SINGULAR_NAME_SurveyFeet(),   DgnCoreL10N::UNIT_PLURAL_NAME_SurveyFeet(),  },
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0 * 6.0,           StandardUnit::EnglishFathoms,      DgnCoreL10N::UNIT_LABEL_Fathoms(),      DgnCoreL10N::UNIT_SINGULAR_NAME_Fathoms(),      DgnCoreL10N::UNIT_PLURAL_NAME_Fathoms(),     },
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0 * 16.5,          StandardUnit::EnglishRods,         DgnCoreL10N::UNIT_LABEL_Rods(),         DgnCoreL10N::UNIT_SINGULAR_NAME_Rods(),         DgnCoreL10N::UNIT_PLURAL_NAME_Rods(),        },
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0 * 66.0,          StandardUnit::EnglishChains,       DgnCoreL10N::UNIT_LABEL_Chains(),       DgnCoreL10N::UNIT_SINGULAR_NAME_Chains(),       DgnCoreL10N::UNIT_PLURAL_NAME_Chains(),      },
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0 * 660.0,         StandardUnit::EnglishFurlongs,     DgnCoreL10N::UNIT_LABEL_Furlongs(),     DgnCoreL10N::UNIT_SINGULAR_NAME_Furlongs(),     DgnCoreL10N::UNIT_PLURAL_NAME_Furlongs(),    },
    { UnitBase::Meter,     UnitSystem::USSurvey,   39370.0,                  12000.0 * 5280.0,        StandardUnit::EnglishSurveyMiles,  DgnCoreL10N::UNIT_LABEL_SurveyMiles(),  DgnCoreL10N::UNIT_SINGULAR_NAME_SurveyMiles(),  DgnCoreL10N::UNIT_PLURAL_NAME_SurveyMiles(), },

    // No System
    { UnitBase::Meter,     UnitSystem::Undefined,  10000000000.0,            1.0,                     StandardUnit::NoSystemAngstroms,         DgnCoreL10N::UNIT_LABEL_Angstroms(),         DgnCoreL10N::UNIT_SINGULAR_NAME_Angstroms(),         DgnCoreL10N::UNIT_PLURAL_NAME_Angstroms(),        },
    { UnitBase::Meter,     UnitSystem::Undefined,  1.0,                      1852.0,                  StandardUnit::NoSystemNauticalMiles,     DgnCoreL10N::UNIT_LABEL_NauticalMiles(),     DgnCoreL10N::UNIT_SINGULAR_NAME_NauticalMiles(),     DgnCoreL10N::UNIT_PLURAL_NAME_NauticalMiles(),    },
    { UnitBase::Meter,     UnitSystem::Undefined,  1.0,                      149597900000.0,          StandardUnit::NoSystemAstronomicalUnits, DgnCoreL10N::UNIT_LABEL_AstronomicalUnits(), DgnCoreL10N::UNIT_SINGULAR_NAME_AstronomicalUnits(), DgnCoreL10N::UNIT_PLURAL_NAME_AstronomicalUnits(),},
    { UnitBase::Meter,     UnitSystem::Undefined,  1.0,                      9460730000000000.0,      StandardUnit::NoSystemLightYears,        DgnCoreL10N::UNIT_LABEL_LightYears(),        DgnCoreL10N::UNIT_SINGULAR_NAME_LightYears(),        DgnCoreL10N::UNIT_PLURAL_NAME_LightYears(),       },
    { UnitBase::Meter,     UnitSystem::Undefined,  1.0,                      30856780000000000.0,     StandardUnit::NoSystemParsecs,           DgnCoreL10N::UNIT_LABEL_Parsecs(),           DgnCoreL10N::UNIT_SINGULAR_NAME_Parsecs(),           DgnCoreL10N::UNIT_PLURAL_NAME_Parsecs(),          },

    // Angular
    { UnitBase::Degree,    UnitSystem::Undefined,  3600.0,                   1.0,                     StandardUnit::AngleSeconds,        DgnCoreL10N::UNIT_LABEL_Seconds(),      DgnCoreL10N::UNIT_SINGULAR_NAME_Seconds(),       DgnCoreL10N::UNIT_PLURAL_NAME_Seconds(), },
    { UnitBase::Degree,    UnitSystem::Undefined,  60.0,                     1.0,                     StandardUnit::AngleMinutes,        DgnCoreL10N::UNIT_LABEL_Minutes(),      DgnCoreL10N::UNIT_SINGULAR_NAME_Minutes(),       DgnCoreL10N::UNIT_PLURAL_NAME_Minutes(), },
    { UnitBase::Degree,    UnitSystem::Undefined,  10.0,                     9.0,                     StandardUnit::AngleGrads,          DgnCoreL10N::UNIT_LABEL_Grads(),        DgnCoreL10N::UNIT_SINGULAR_NAME_Grads(),         DgnCoreL10N::UNIT_PLURAL_NAME_Grads(),   },
    { UnitBase::Degree,    UnitSystem::Undefined,  1.0,                      1.0,                     StandardUnit::AngleDegrees,        DgnCoreL10N::UNIT_LABEL_Degrees(),      DgnCoreL10N::UNIT_SINGULAR_NAME_Degrees(),       DgnCoreL10N::UNIT_PLURAL_NAME_Degrees(), },
    { UnitBase::Degree,    UnitSystem::Undefined,  3.1415926535897932846,    180.0,                   StandardUnit::AngleRadians,        DgnCoreL10N::UNIT_LABEL_Radians(),      DgnCoreL10N::UNIT_SINGULAR_NAME_Radians(),       DgnCoreL10N::UNIT_PLURAL_NAME_Radians(), },

    // No Base
    { UnitBase::None,      UnitSystem::Undefined,  1.0,                      1.0,                     StandardUnit::UnitlessWhole,       DgnCoreL10N::UNIT_LABEL_UnitlessWhole(), DgnCoreL10N::UNIT_SINGULAR_NAME_UnitlessWhole(), DgnCoreL10N::UNIT_PLURAL_NAME_UnitlessWhole(),},
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct StandardUnitTable
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetFirst ()
    {
    return s_standardUnits;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetLast ()
    {
    return s_standardUnits + _countof (s_standardUnits) - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetNext (StandardUnitTableEntryCP standardUnit)
    {
    if (NULL == standardUnit)
        return NULL;

    if (standardUnit >= s_standardUnits + _countof (s_standardUnits) - 1)
        return NULL;

    return ++standardUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    GetPrevious (StandardUnitTableEntryCP standardUnit)
    {
    if (NULL == standardUnit)
        return NULL;

    if (standardUnit <= s_standardUnits)
        return NULL;

    return --standardUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindByNumber (StandardUnit unitNum)
    {
    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof (s_standardUnits); iUnit++, standardUnit++)
        {
        if (standardUnit->m_standardNumber == unitNum)
            return standardUnit;
        }

    return NULL;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindMatchingScaleFactor (UnitDefinitionCR unitDef)
    {
    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof (s_standardUnits); iUnit++, standardUnit++)
        {
        if (standardUnit->ScaleFactorMatches (unitDef))
            return standardUnit;
        }

    return NULL;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StandardUnitTableEntryCP    FindByName (WCharCP name)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }
    
    int                         iUnit;
    StandardUnitTableEntryCP    standardUnit;

    for (iUnit = 0, standardUnit = s_standardUnits; iUnit < _countof (s_standardUnits); iUnit++, standardUnit++)
        {
        // Try to match either the singular or plural name
        if (0 == BeStringUtilities::Wcsicmp (standardUnit->GetName(true).c_str(),  name) || 
            0 == BeStringUtilities::Wcsicmp (standardUnit->GetName(false).c_str(), name) )
            {
            return standardUnit;
            }
        }

    return NULL;    
    }

}; //StandardUnitTable

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  UnitDefinition::GetStandardUnit (StandardUnit iUnit)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber (iUnit);

    if (NULL == unitEntry)
        return UnitDefinition();

    return unitEntry->ToUnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  UnitDefinition::GetStandardUnitByName (WCharCP name)
    {
    if (NULL == name)
        { BeAssert (false); return UnitDefinition (); }
    
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByName (name);

    if (NULL == unitEntry)
        return UnitDefinition();

    return unitEntry->ToUnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString         UnitDefinition::GetStandardLabel (StandardUnit iUnit)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber (iUnit);

    if (NULL == unitEntry)
        return L"";

    return unitEntry->GetLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString         UnitDefinition::GetStandardName (StandardUnit iUnit, bool singular)
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindByNumber (iUnit);

    if (NULL == unitEntry)
        return L"";

    return unitEntry->GetName (singular);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit    UnitDefinition::IsStandardUnit () const
    {
    StandardUnitTableEntryCP    unitEntry = StandardUnitTable::FindMatchingScaleFactor (*this);

    if (NULL == unitEntry)
        return StandardUnit::None;

    return unitEntry->m_standardNumber;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitIteratorOptions::UnitIteratorOptions ()
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
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            UnitIteratorOptions::IsBaseAllowed (UnitBase base) const
    {
    switch (base)
        {
        case UnitBase::None:   return m_allowBaseNone;
        case UnitBase::Meter:  return m_allowBaseMeter;
        case UnitBase::Degree: return m_allowBaseDegree;
        }

    BeAssert (0);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            UnitIteratorOptions::IsSystemAllowed (UnitSystem system) const
    {
    switch (system)
        {
        case UnitSystem::Undefined:   return m_allowSystemNone;
        case UnitSystem::English:     return m_allowSystemMetric;
        case UnitSystem::Metric:      return m_allowSystemEnglish;
        case UnitSystem::USSurvey:    return m_allowSystemUSSurvey;
        }

    BeAssert (0);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetOrderAscending ()  { m_orderAscending = true;  }
void            UnitIteratorOptions::SetOrderDescending () { m_orderAscending = false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::DisallowAllBases ()
    {
    m_allowBaseNone   = false;
    m_allowBaseMeter  = false;
    m_allowBaseDegree = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowAdditionalBase (UnitBase base)
    {
    switch (base)
        {
        case UnitBase::None:   m_allowBaseNone   = true;    return;
        case UnitBase::Meter:  m_allowBaseMeter  = true;    return;
        case UnitBase::Degree: m_allowBaseDegree = true;    return;
        }

    BeAssert (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowSingleBase (UnitBase base)
    {
    DisallowAllBases ();
    SetAllowAdditionalBase (base);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::DisallowAllSystems ()
    {
    m_allowSystemNone       = false;
    m_allowSystemMetric     = false;
    m_allowSystemEnglish    = false;
    m_allowSystemUSSurvey   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowAdditionalSystem (UnitSystem system)
    {
    switch (system)
        {
        case UnitSystem::Undefined:     m_allowSystemNone     = true;   return;
        case UnitSystem::English:       m_allowSystemMetric   = true;   return;
        case UnitSystem::Metric:        m_allowSystemEnglish  = true;   return;
        case UnitSystem::USSurvey:      m_allowSystemUSSurvey = true;   return;
        }

    BeAssert (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetAllowSingleSystem (UnitSystem system)
    {
    DisallowAllSystems ();
    SetAllowAdditionalSystem (system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitIteratorOptions::SetSizeCriteria (UnitDefinitionCR unitDef, UnitCompareMethod method)
    {
    m_compareUnit = unitDef;
    SetAllowSingleBase (unitDef.GetBase());

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

UnitDefinition   StandardUnitCollection::Entry::GetUnitDef()    const { return m_tableP ? m_tableP->ToUnitDefinition() : UnitDefinition(); }
StandardUnit     StandardUnitCollection::Entry::GetNumber()     const { return m_tableP ? m_tableP->m_standardNumber   : StandardUnit::None; }
WString          StandardUnitCollection::Entry::GetName(bool singular) const { return m_tableP ? m_tableP->GetName (singular) : L""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator::const_iterator (UnitIteratorOptionsCR options)
    :
    m_options (&options)
    {
    if (m_options->IsOrderAscending())
        m_entry.m_tableP = StandardUnitTable::GetFirst();
    else
        m_entry.m_tableP = StandardUnitTable::GetLast();

    // Find the first valid unit
    if ( ! UnitValidForIterator (m_entry.m_tableP))
        ++(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator::const_iterator ()
    :
    m_options (NULL)
    {
    m_entry.m_tableP = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    StandardUnitCollection::const_iterator::UnitValidForIterator (StandardUnitTableEntryCP tableEntry)
    {
    if ( ! m_options->IsBaseAllowed (tableEntry->m_base))
        return false;

    if ( ! m_options->IsSystemAllowed (tableEntry->m_system))
        return false;

    UnitDefinitionCR compareUnit = m_options->GetCompareUnit();

    if (compareUnit.IsValid())
        {
        int iComparison = tableEntry->Compare (compareUnit);

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
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator& StandardUnitCollection::const_iterator::operator ++ ()
    {
    while (NULL != m_entry.m_tableP)
        {
        if (m_options->IsOrderAscending())
             m_entry.m_tableP = StandardUnitTable::GetNext ( m_entry.m_tableP);
        else
             m_entry.m_tableP = StandardUnitTable::GetPrevious ( m_entry.m_tableP);

        if (NULL ==  m_entry.m_tableP || UnitValidForIterator ( m_entry.m_tableP))
            break;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandardUnitCollection::const_iterator::operator != (StandardUnitCollection::const_iterator const& rhs) const
    {
    return m_entry.m_tableP != rhs.m_entry.m_tableP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::Entry const&  StandardUnitCollection::const_iterator::operator * () const
    {
    return m_entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      StandardUnitCollection::StandardUnitCollection (UnitIteratorOptionsCR options)
    :
    m_options (options)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator    StandardUnitCollection::begin () const
    {
    return const_iterator (m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnitCollection::const_iterator    StandardUnitCollection::end () const
    {
    return const_iterator();
    }


// -=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//                              User Units
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


#if defined (WIP_CFGVAR)
static const WCharCP    MS_UNITS_SHOWALL            = L"MS_UNITS_SHOWALL";
#endif

GLOBAL_TYPEDEF (UserUnitTable, UserUnitTable)
DGNPLATFORM_TYPEDEFS (UserUnitTableEntry)
typedef std::vector <UserUnitTableEntry>    UnitEntryVector;

/*----------------------------------------------------------------------+
| Structure used for entries in the table of available units.           |
+----------------------------------------------------------------------*/
struct DgnPlatform::UserUnitTableEntry
    {
    int             m_number;
    UnitDefinition  m_unitDef;
    WString         m_singularName;
    WString         m_pluralName;
    WString         m_labels;               // comma-separated list of labels

    UserUnitTableEntry (UnitDefinitionCR unitDef, int unitNum, WCharCP singularName, WCharCP pluralName, WString labels)
        : m_unitDef (unitDef), m_number (unitNum), m_singularName (singularName), m_pluralName (pluralName), m_labels (labels) { }
    };

/*---------------------------------------------------------------------------------**//**
* units.def is parsed once and its contents cached here.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserUnitTable : DgnHost::HostObjectBase
    {
private:
    UnitEntryVector         m_unitVector;
    bool                    m_containsStandardUnits;

    UserUnitTable() : m_containsStandardUnits (false)   { }

    bool                    ParseLine (WStringR labels, WStringR singularName, WStringR pluralName, double& numerator, double& denominator, uint32_t& base, uint32_t& system, WCharCP input) const;
    StatusInt               AddEntry (UserUnitTableEntryCR newEntry);
    void                    PopulateFromFile();
    void                    PopulateFromStandardUnits();

    static int              GetBasePriority(UnitDefinitionCR unitDef);
    static DgnHost::Key&    GetHostKey()                                { static DgnHost::Key key; return key; }
public:
    bool                    ContainsStandardUnits() const   { return m_containsStandardUnits; }
    size_t                  GetCount() const            { return m_unitVector.size(); }
    void                    Dump() const;

    UserUnitTableEntryCP    GetEntry (size_t index) const { return (index < GetCount()) ? &m_unitVector[index] : NULL; }
    UserUnitTableEntryCP    FindByName (WCharCP name) const;
    UserUnitTableEntryCP    FindByScale (UnitDefinitionCR unitDef) const;
    UserUnitTableEntryCP    FindByNumber (int unitNumber) const;        
    UserUnitTableEntryCP    FindByLabel (WCharCP label) const;

    static UserUnitTableR   GetTable();

    template <class V>
    UserUnitTableEntryCP    VisitUnits (V& v) const
        {
        FOR_EACH (UserUnitTableEntryCR entry, m_unitVector)
            if (v.Accept (entry))
                return &entry;

        return NULL;
        }

    static void             ResetForTest();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeStrippedString (WStringR str, WCharCP start, WCharCP end /*inclusive*/)
    {
    while (*start && iswspace (*start))
        ++start;
    while (*end && end > start && iswspace (*end))
        --end;

    str = WString (start, end+1);
    }

namespace UnitVisitors
    {
    struct ByNameFinder
        {
        WCharCP         m_name;
        bool            Accept (UserUnitTableEntryCR entry)     { return 0 == BeStringUtilities::Wcsicmp (entry.m_singularName.c_str(), m_name) || 0 == BeStringUtilities::Wcsicmp (entry.m_pluralName.c_str(), m_name); }
        };

    struct ByLabelFinder
        {
        WCharCP         m_label;
        bool            Accept (UserUnitTableEntryCR entry)
            {
            WCharCP     labelList = entry.m_labels.c_str(), endOfList = labelList + wcslen (labelList), cur = labelList, next;
            WString thisLabel;
            while (cur < endOfList)
                {
                next = ::wcschr (cur, ',');
                if (NULL == next)
                    next = endOfList;
                
                makeStrippedString (thisLabel, cur, next-1);
                if (0 == BeStringUtilities::Wcsicmp (thisLabel.c_str(), m_label))
                    return true;

                cur = next+1;
                }

            return false;
            }
        };

    struct ByScaleFinder
        {
        UnitDefinitionCR    m_unitDef;
        bool                Accept (UserUnitTableEntryCR entry) { return 0 == m_unitDef.CompareByScale (entry.m_unitDef); }
        };

    struct ByNumberFinder
        {
        int num;
        bool Accept (UserUnitTableEntryCR entry)    { return entry.m_number == num; }
        };

    struct ClosestFinder
        {
        UnitDefinitionCR unit;
        bool             ascending;

        bool Accept (UserUnitTableEntryCR entry)
            { return unit.GetBase() == entry.m_unitDef.GetBase() && unit.GetSystem() == entry.m_unitDef.GetSystem() && 0 < unit.CompareByScale (entry.m_unitDef); }

        ClosestFinder (UnitDefinitionCR u, bool a) : unit(u), ascending(a) { }
        };

    struct Dumper
        {
        int iEntry;
        Dumper() : iEntry(0) { }

        bool Accept (UserUnitTableEntryCR unitEntry)
            {
            printf ("  %d:\t%d %S (%f / %f)\n", iEntry, unitEntry.m_number, unitEntry.m_pluralName.c_str(), unitEntry.m_unitDef.GetNumerator(), unitEntry.m_unitDef.GetDenominator());
            iEntry++;
            return false;
            }
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByName (WCharCP name) const
    {
    UnitVisitors::ByNameFinder v = { name };
    return VisitUnits (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByLabel (WCharCP label) const
    {
    UnitVisitors::ByLabelFinder v = { label };
    return VisitUnits (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByScale (UnitDefinitionCR unitDef) const
    {
    UnitVisitors::ByScaleFinder v = { unitDef };
    return VisitUnits (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableEntryCP UserUnitTable::FindByNumber (int unitNumber) const
    {
    UnitVisitors::ByNumberFinder v = { unitNumber };
    return VisitUnits (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    UserUnitTable::Dump () const
    {
    printf ("Dumping units table, %zu entries:\n", m_unitVector.size());
    UnitVisitors::Dumper v;
    VisitUnits (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/09
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
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       UserUnitTable::AddEntry
(
UserUnitTableEntryCR    newEntry        /* => add this to the table */
)
    {
    bool                        found = false;
    UnitEntryVector::iterator   iter;
    UnitDefinitionCR            newUnit = newEntry.m_unitDef;
    int                         newPriority = GetBasePriority (newUnit);

    for (iter = m_unitVector.begin(); iter != m_unitVector.end(); ++iter)
        {
        UnitDefinitionCR    existingUnit = iter->m_unitDef;
        int                 basePriority = GetBasePriority (existingUnit);

        // Keep the units together by base in order: Meter, Degree, Unitless
        if (basePriority != newPriority)
            {
            if (basePriority > newPriority)
                continue;
            else
                break;
            }

        int iCompare = newUnit.CompareByScale (existingUnit);

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

    m_unitVector.insert (iter, newEntry);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitTableR UserUnitTable::GetTable()
    {
    DgnHost::Key& key = GetHostKey();
    UserUnitTableP table = dynamic_cast<UserUnitTableP> (T_HOST.GetHostObject (key));
    if (NULL == table)
        {
        table = new UserUnitTable();
        table->PopulateFromFile();

#if defined (WIP_CFGVAR) // MS_UNITS_SHOWALL
        if (ConfigurationManager::IsVariableDefined (MS_UNITS_SHOWALL))
#endif
            table->PopulateFromStandardUnits();

        T_HOST.SetHostObject (key, table);
        }

    BeAssert (NULL != table);
    return *table;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::ResetForTest()
    {
    DgnHost::Key& key = GetHostKey();
    UserUnitTableP table = static_cast<UserUnitTableP> (T_HOST.GetHostObject (key));
    if (NULL != table)
        {
        T_HOST.SetHostObject (key, NULL);
        delete table;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::PopulateFromStandardUnits()
    {
    m_containsStandardUnits = true;
    UnitSystem  unitSystem[] = { UnitSystem::Metric, UnitSystem::English, UnitSystem::USSurvey, UnitSystem::Undefined };
    for (int iSystem = 0; iSystem < _countof (unitSystem); iSystem++)
        {
        UnitIteratorOptions options;
        options.SetAllowSingleSystem (unitSystem[iSystem]);

        StandardUnitCollection collection (options);

        FOR_EACH (StandardUnitCollection::Entry const& standardUnit, collection)
            {
            UnitDefinition unitDef = standardUnit.GetUnitDef();
            UserUnitTableEntry  entry (unitDef, (int)standardUnit.GetNumber(),
                                    standardUnit.GetName(true).c_str(), standardUnit.GetName(false).c_str(), unitDef.GetLabelCP());

            AddEntry (entry);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UserUnitTable::PopulateFromFile()
    {
// *** WIP_ForeignFormat - We should move this code into foreignformat.

#if defined (WIP_CFGVAR) // MS_CUSTOMUNITDEF
    WString filename;
    if (SUCCESS != ConfigurationManager::GetVariable (filename, L"MS_CUSTOMUNITDEF"))
        return;

    BeFileStatus fileOpenStatus;
    BeTextFilePtr file = BeTextFile::Open (fileOpenStatus, filename.c_str(), TextFileOpenType::Read, TextFileOptions::None);
    if (file.IsNull() || BeFileStatus::Success != fileOpenStatus)
        return;

    UnitInfo        unitInfo;
    WString         nameSingular, namePlural, labels;
    int             customUnitIndex = 0;
    while (SUCCESS == GetNextUserFileUnitDef (unitInfo, nameSingular, namePlural, labels, *file))
        {
        UnitDefinition unitDef (unitInfo);
        int unitNumber = unitDef.IsStandardUnit();
        if (StandardUnit::None == unitNumber || StandardUnit::Custom == unitNumber)
            unitNumber = StandardUnit::Custom + 1 + customUnitIndex++;

        UserUnitTableEntry entry (unitDef, unitNumber, nameSingular.c_str(), namePlural.c_str(), labels);
        AddEntry (entry);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitTable::ParseLine (WStringR labels, WStringR singularName, WStringR pluralName, double& numerator, double& denominator, uint32_t& base, uint32_t& system, WCharCP input) const
    {
    static const WChar separator = ';';
    WCharCP a = input,
            b = a ? ::wcschr (a, separator)   : NULL,
            c = b ? ::wcschr (b+1, separator) : NULL,
            d = c ? ::wcschr (c+1, separator) : NULL,
            e = d ? ::wcschr (d+1, separator) : NULL,
            f = e ? ::wcschr (e+1, separator) : NULL,
            g = f ? ::wcschr (f+1, separator) : NULL;

    if (NULL == g)
        return false;

    makeStrippedString (labels, a, b-1);
    makeStrippedString (singularName, b+1, c-1);
    makeStrippedString (pluralName, c+1, d-1);

    return  1 == BE_STRING_UTILITIES_SWSCANF (d+1, L"%lf", &numerator) &&
            1 == BE_STRING_UTILITIES_SWSCANF (e+1, L"%lf", &denominator) &&
            1 == BE_STRING_UTILITIES_SWSCANF (f+1, L"%d", &base) &&
            1 == BE_STRING_UTILITIES_SWSCANF (g+1, L"%d", &system);        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UserUnitCollection::Entry::GetUnitDef() const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry (m_index);
    return entry ? entry->m_unitDef : UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit UserUnitCollection::Entry::GetNumber() const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry (m_index);
    return entry ? (StandardUnit)entry->m_number : StandardUnit::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
WString UserUnitCollection::Entry::GetName (bool singular) const
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry (m_index);
    return entry ? (singular ? entry->m_singularName : entry->m_pluralName) : L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator::const_iterator (UnitIteratorOptionsCR options) : m_options (&options)
    {
    UserUnitTableR unitTable = UserUnitTable::GetTable();
    if (m_options->IsOrderAscending())
        m_entry.m_index = 0;
    else
        m_entry.m_index = unitTable.GetCount() - 1;

    // Find the first valid unit
    if (IndexIsValid (m_entry.m_index) && ! UnitValidForIterator (m_entry.m_index))
        ++(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator::const_iterator (bool ascending) : m_options (NULL)
    {
    // Create an invalid iterator
    if (ascending)
        m_entry.m_index = UserUnitTable::GetTable().GetCount();
    else
        m_entry.m_index = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::const_iterator::UnitValidForIterator (size_t tableIndex)
    {
    UserUnitTableEntryCP entry = UserUnitTable::GetTable().GetEntry (tableIndex);
    if (NULL == entry)
        return false;

    UnitDefinitionCR unit = entry->m_unitDef;
    if (!m_options->IsBaseAllowed (unit.GetBase()) || !m_options->IsSystemAllowed (unit.GetSystem()))
        return false;

    UnitDefinitionCR compare = m_options->GetCompareUnit();
    if (compare.IsValid())
        {
        int iComparison = unit.CompareByScale (compare);
        
        // iComp is reversed: positive if candidate is smaller
        if ((0 > iComparison && !m_options->GetCompareAllowLarger()) ||
            (0 == iComparison && !m_options->GetCompareAllowEqual()) ||
            ( 0 < iComparison && !m_options->GetCompareAllowSmaller()))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::IndexIsValid (size_t tableIndex)
    {
    return tableIndex < UserUnitTable::GetTable().GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator& UserUnitCollection::const_iterator::operator ++ ()
    {
    while (IndexIsValid (m_entry.m_index))
        {
        m_entry.m_index += m_options->IsOrderAscending() ? 1 : -1;
        if (!IndexIsValid (m_entry.m_index) || UnitValidForIterator (m_entry.m_index))
            break;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserUnitCollection::const_iterator::operator != (UserUnitCollection::const_iterator const& rhs) const
    {
    return m_entry.m_index != rhs.m_entry.m_index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ UserUnitCollection::UserUnitCollection (UnitIteratorOptionsCR options) : m_options (options)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::Entry UserUnitCollection::const_iterator::operator * () const
    {
    return m_entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator UserUnitCollection::begin() const
    {
    return const_iterator (m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UserUnitCollection::const_iterator UserUnitCollection::end() const
    {
    return const_iterator (m_options.IsOrderAscending());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByNumber (int unitNumber, bool alsoStandard)
    {
    UserUnitTableR unitTable = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = unitTable.FindByNumber (unitNumber);
    if (NULL != entry)
        return entry->m_unitDef;
    else if (alsoStandard && !unitTable.ContainsStandardUnits())
        return UnitDefinition::GetStandardUnit ((StandardUnit)unitNumber);
    else
        return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetNextLarger() const
    {
    UnitVisitors::ClosestFinder v (*this, true);
    UserUnitTableEntryCP found = UserUnitTable::GetTable().VisitUnits (v);
    return found ? found->m_unitDef : *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetNextSmaller () const
    {
    UnitVisitors::ClosestFinder v (*this, false);
    UserUnitTableEntryCP found = UserUnitTable::GetTable().VisitUnits (v);
    return found ? found->m_unitDef : *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
int UnitDefinition::GetNumber (bool alsoStandard) const
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByScale (*this);
    if (NULL != entry)
        return entry->m_number;
    else if (alsoStandard && table.ContainsStandardUnits())
        return (int)this->IsStandardUnit();
    else
        return (int)StandardUnit::Custom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString UnitDefinition::GetName (bool singular, bool alsoStandard) const
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByScale (*this);
    if (NULL != entry)
        return singular ? entry->m_singularName : entry->m_pluralName;

    StandardUnit unitNo = (alsoStandard && table.ContainsStandardUnits()) ? IsStandardUnit() : StandardUnit::Custom;
    return UnitDefinition::GetStandardName (unitNo, singular);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByName (WCharCP unitName, bool alsoStandard)
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByName (unitName);
    if (NULL != entry)
        return entry->m_unitDef;

    if (alsoStandard && table.ContainsStandardUnits())
        return UnitDefinition::GetStandardUnitByName (unitName);

    return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition UnitDefinition::GetByLabel (WCharCP label, bool alsoStandard)
    {
    UserUnitTableR table = UserUnitTable::GetTable();
    UserUnitTableEntryCP entry = table.FindByLabel (label);
    if (NULL != entry)
        return entry->m_unitDef;
    else if (alsoStandard && table.ContainsStandardUnits())
        {
        UnitIteratorOptions opts;
        FOR_EACH (StandardUnitCollection::Entry const& standardUnit, StandardUnitCollection (opts))
            {
            UnitDefinition def = standardUnit.GetUnitDef();
            if (0 == BeStringUtilities::Wcsicmp (label, def.GetLabel().c_str()))
                return def;
            }
        }

    return UnitDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::BackDoor_ResetUserUnitsForTest()
    {
    UserUnitTable::ResetForTest();
    }

