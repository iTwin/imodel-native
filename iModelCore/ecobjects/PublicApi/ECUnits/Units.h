/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECUnits/Units.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

// Note: This is a barebones implementation targeting Graphite requirements.
// It supports applying UnitSpecifications and DisplayUnitSpecifications to ECProperties.

#include <ECObjects/ECObjects.h>

EC_TYPEDEFS (Unit);
EC_TYPEDEFS (UnitConverter);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

enum UnitConversionType
    {
    UnitConversionType_Identity,
    UnitConversionType_Slope,
    UnitConversionType_Factor,
    UnitConversionType_FactorAndOffset
    };

/*---------------------------------------------------------------------------------**//**
* Every Unit has a UnitConverter capable of converting values to and from its base Unit.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitConverter
    {
private:
    UnitConversionType      m_type;
    double                  m_factor;
    double                  m_offset;
public:
    UnitConverter (bool isSlope = false) : m_type (!isSlope ? UnitConversionType_Identity : UnitConversionType_Slope) { }
    UnitConverter (double factor) : m_type (UnitConversionType_Factor), m_factor (factor), m_offset (0.0) { BeAssert (0.0 != factor); }
    UnitConverter (double factor, double offset) : m_type (UnitConversionType_FactorAndOffset), m_factor (factor), m_offset (offset) { BeAssert (0.0 != factor); }

    ECOBJECTS_EXPORT double          ToBase (double valueInThisUnit) const;
    ECOBJECTS_EXPORT double          FromBase (double valueInBaseUnit) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Unit
    {
private:
    WString                 m_baseUnitName;
    WString                 m_shortLabel;
    UnitConverter           m_converter;
    WString                 m_unitName;     // This is required only because DgnPlatform needs it in order to look up unit label customizations...
public:
    Unit() { }
    Unit (WCharCP unitName, WCharCP shortLabel, UnitConverterCR converter, WCharCP baseUnitName) : m_baseUnitName (baseUnitName), m_shortLabel (shortLabel), m_converter (converter), m_unitName (unitName) { }

    WCharCP                 GetName() const                     { return m_unitName.c_str(); }
    WCharCP                 GetBaseUnitName() const             { return m_baseUnitName.c_str(); }
    WCharCP                 GetShortLabel() const               { return m_shortLabel.c_str(); }
    void                    SetShortLabel (WCharCP label)       { m_shortLabel = label ? label : L""; }
    UnitConverterCR         GetConverter() const                { return m_converter; }
    bool                    IsCompatible (UnitCR other) const   { return m_baseUnitName.Equals (other.m_baseUnitName); }

    ECOBJECTS_EXPORT bool   ConvertTo (double& value, UnitCR target) const;

    ECOBJECTS_EXPORT static bool        GetUnitForECProperty (UnitR unit, ECPropertyCR ecprop);
    ECOBJECTS_EXPORT static bool        GetDisplayUnitForECProperty (UnitR unit, WStringR displayFormat, ECPropertyCR ecprop);
    };

END_BENTLEY_ECOBJECT_NAMESPACE




