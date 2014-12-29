/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECUnits/Units.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

// Note: This is a barebones implementation targeting Graphite requirements.
// It supports applying UnitSpecifications and DisplayUnitSpecifications to ECProperties.

#include <ECObjects/ECObjects.h>

EC_TYPEDEFS(IECClassLocater);

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
    explicit UnitConverter (bool isSlope = false) : m_type (!isSlope ? UnitConversionType_Identity : UnitConversionType_Slope), m_factor (0.0), m_offset (0.0) { }
    explicit UnitConverter (double factor) : m_type (UnitConversionType_Factor), m_factor (factor), m_offset (0.0) { BeAssert (0.0 != factor); }
    UnitConverter (double factor, double offset) : m_type (UnitConversionType_FactorAndOffset), m_factor (factor), m_offset (offset) { BeAssert (0.0 != factor); }
    UnitConverter (UnitConverterCR other) : m_type(other.m_type), m_factor(other.m_factor), m_offset(other.m_offset) { }

    ECOBJECTS_EXPORT double         ToBase (double valueInThisUnit) const;
    ECOBJECTS_EXPORT double         FromBase (double valueInBaseUnit) const;
    ECOBJECTS_EXPORT bool           IsEquivalent (UnitConverterCR other) const;

    UnitConversionType              GetType() const             { return m_type; }
    double                          GetFactor() const           { return UnitConversionType_Factor == m_type || UnitConversionType_FactorAndOffset == m_type ? m_factor : 1.0; }
    double                          GetOffset() const           { return UnitConversionType_FactorAndOffset == m_type ? m_offset : 0.0; }

    WString                         ToECExpressionString() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitSpec
    {
private:
    WString                 m_baseUnitName;
    UnitConverter           m_converter;
public:
    UnitSpec() { }
    UnitSpec (WCharCP baseUnit, UnitConverterCR converter) : m_baseUnitName (baseUnit), m_converter (converter) { }

    WCharCP                 GetBaseUnitName() const                 { return m_baseUnitName.c_str(); }
    bool                    IsCompatible (UnitSpecCR other) const   { return m_baseUnitName.Equals (other.m_baseUnitName); }
    bool                    IsEquivalent (UnitSpecCR other) const   { return m_converter.IsEquivalent (other.m_converter) && IsCompatible (other); }
    UnitConverterCR         GetConverter() const                    { return m_converter; }
    bool                    IsUnspecified() const                   { return m_baseUnitName.empty(); }
    void                    SetUnspecified()                        { m_baseUnitName.clear(); }

    void                    SetConverter (UnitConverterCR converter)    { m_converter = converter; }

    ECOBJECTS_EXPORT bool   ConvertTo (double& value, UnitSpecCR target) const;

    ECOBJECTS_EXPORT WString    ToECExpressionString() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Unit : UnitSpec
    {
private:
    WString                 m_shortLabel;
    WString                 m_unitName;     // This is required only because DgnPlatform needs it in order to look up unit label customizations...
public:
    Unit() : UnitSpec() { }
    Unit (WCharCP unitName, WCharCP shortLabel, UnitConverterCR converter, WCharCP baseUnitName) : UnitSpec (baseUnitName, converter), m_shortLabel (shortLabel), m_unitName (unitName) { }

    WCharCP                 GetName() const                     { return m_unitName.c_str(); }
    WCharCP                 GetShortLabel() const               { return m_shortLabel.c_str(); }
    void                    SetShortLabel (WCharCP label)       { m_shortLabel = label ? label : L""; }

    ECOBJECTS_EXPORT static bool        GetUnitForECProperty (UnitR unit, ECPropertyCR ecprop);
    ECOBJECTS_EXPORT static bool        GetUnitForECProperty (UnitR unit, ECPropertyCR ecprop, IECClassLocaterR unitsECClassLocater);
    ECOBJECTS_EXPORT static bool        GetDisplayUnitAndFormatForECProperty(UnitR unit, WStringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop);
    ECOBJECTS_EXPORT static bool        GetDisplayUnitAndFormatForECProperty (UnitR displayUnit, WStringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop, IECClassLocaterR unitsECClassLocater);
    //ECOBJECTS_EXPORT static bool        GetUnitByName (UnitR unit, WCharCP unitName, bool createIfNotFound);

    // Formats the ECValue according to UnitSpecification custom attribute on the ECProperty.
    // If instance is non-null and an IECTypeAdapter can be located to perform the formatting, the IECTypeAdapter::ConvertToString() method will be used
    // The numeric value will be formatted according to the DisplayFormatString property of any DisplayUnitSpecification custom attribute on the ECProperty
    // Returns false if no UnitSpecification present or if an error occurs
    ECOBJECTS_EXPORT static bool        FormatValue (WStringR formatted, ECValueCR v, ECPropertyCR ecprop, IECInstanceCP instance, IECClassLocaterR unitsECClassLocater);
    ECOBJECTS_EXPORT static bool        FormatValue (WStringR formatted, ECValueCR v, ECPropertyCR ecprop, IECInstanceCP instance);

    };

END_BENTLEY_ECOBJECT_NAMESPACE




