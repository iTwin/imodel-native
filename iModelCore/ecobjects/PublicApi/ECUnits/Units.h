/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECUnits/Units.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
// WIP_UNITS: publish api

#include <ECObjects/ECObjects.h>

// WIP_UNITS: This is a very rough implementation, to try some things out.
//  -Where do these belong? Are they part of the EC universe?
//  -Names.

#define BEGIN_BENTLEY_EC_UNITS_NAMESPACE namespace Bentley { namespace ECUnits {
#define END_BENTLEY_EC_UNITS_NAMESPACE   } }

#define UNITS_TYPEDEFS(_name_)  \
        BEGIN_BENTLEY_EC_UNITS_NAMESPACE      \
            struct _name_;      \
            typedef _name_ *         _name_##P;  \
            typedef _name_ &         _name_##R;  \
            typedef _name_ const*    _name_##CP; \
            typedef _name_ const&    _name_##CR; \
        END_BENTLEY_EC_UNITS_NAMESPACE

UNITS_TYPEDEFS (UnitSystem);
UNITS_TYPEDEFS (Dimension);
UNITS_TYPEDEFS (KindOfQuantity);
UNITS_TYPEDEFS (Unit);
UNITS_TYPEDEFS (ICustomUnitConverter);
UNITS_TYPEDEFS (UnitConverter);

BEGIN_BENTLEY_EC_UNITS_NAMESPACE

struct UnitsManager;

enum StandardUnitSystem
    {
    StandardUnitSystem_None,
    StandardUnitSystem_SI,
    StandardUnitSystem_USCustomary,
    StandardUnitSystem_Both,

    StandardUnitSystem_MAX
    };

enum StandardUnit
    {
    //...
    StandardUnit_MAX
    };

enum StandardDimension
    {
    //...
    StandardDimension_MAX
    };

enum StandardKindOfQuantity
    {
    //...
    StandardKindOfQuantity_MAX
    };

enum UnitConversionType
    {
    UnitConversionType_Identity,
    UnitConversionType_Slope,
    UnitConversionType_Factor,
    UnitConversionType_FactorAndOffset,
    UnitConversionType_Custom
    };

/*---------------------------------------------------------------------------------**//**
* User-supplied object that can perform custom conversion between values in some unit
* and its base unit.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICustomUnitConverter : RefCountedBase
    {
    friend struct UnitConverter;
protected:
    ICustomUnitConverter() { }

    virtual double  _ToBase (double valueInThisUnit) const = 0;
    virtual double  _FromBase (double valueInBaseUnit) const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* Every Unit has a UnitConverter capable of converting values to and from its base Unit.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitConverter
    {
private:
    UnitConversionType              m_type;
    union
        {
        struct
            {
            double                  m_factor;
            double                  m_offset;
            };
        ICustomUnitConverterP       m_customConverter;
        };
public:
    UnitConverter (bool isSlope = false) : m_type (isSlope ? UnitConversionType_Identity : UnitConversionType_Slope) { }
    UnitConverter (double factor) : m_type (UnitConversionType_Factor), m_factor (factor), m_offset (0.0) { BeAssert (0.0 != factor); }
    UnitConverter (double factor, double offset) : m_type (UnitConversionType_FactorAndOffset), m_factor (factor), m_offset (offset) { BeAssert (0.0 != factor); }
    ECOBJECTS_EXPORT UnitConverter (ICustomUnitConverterR customConverter);
    ECOBJECTS_EXPORT UnitConverter (UnitConverterCR other);
    ECOBJECTS_EXPORT UnitConverter& operator= (UnitConverterCR other);
    ECOBJECTS_EXPORT ~UnitConverter();

    ECOBJECTS_EXPORT double          ToBase (double valueInThisUnit) const;
    ECOBJECTS_EXPORT double          FromBase (double valueInBaseUnit) const;
    };

/*---------------------------------------------------------------------------------**//**
* For iterating over Units contained in a Dimension, UnitSystem, or other collection.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Collection> struct UnitIterator
    {
private:
    typedef bmap<WString, Unit>     UnitsMap;

    T_Collection const&         m_collection;
    UnitsMap::const_iterator    m_cur;
    UnitsMap::const_iterator    m_end;

    void    MoveNext()
        {
        while (m_cur != m_end && !m_collection.IncludesUnit ((++m_cur)->second))
            ;
        }
public:
    UnitIterator (T_Collection const& coll, UnitsMap const& allUnits, bool isEnd)
        : m_collection(coll), m_cur(isEnd ? allUnits.end() : allUnits.begin()), m_end(allUnits.end())
        {
        if (!m_collection.IncludesUnit (m_cur->second))
            MoveNext();
        }

    bool                        operator==(UnitIterator const& rhs) const   { return m_cur == rhs.m_cur; }
    bool                        operator!=(UnitIterator const& rhs) const   { return !(*this == rhs); }
    UnitCP                      operator*() const                           { return &m_cur->second; }
    UnitCP                      operator->() const                          { return &m_cur->second; }
    UnitIterator<T_Collection>& operator++()                                { MoveNext(); return *this; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitSystem
    {
private:
    WCharCP         m_name;
    WString         m_shortLabel;
    WString         m_longLabel;

    UnitSystem (WCharCP shortLabel, WCharCP longLabel) : m_name(NULL), m_shortLabel(shortLabel), m_longLabel(longLabel) { }
public:
    WCharCP         GetName() const             { return m_name; }
    WCharCP         GetLabel() const            { return m_longLabel.c_str(); }
    WCharCP         GetShortLabel() const       { return m_shortLabel.c_str(); }
    void            SetLabel (WCharCP label)    { m_longLabel = label ? label : L""; }
    void            SetShortLabel (WCharCP l)   { m_shortLabel = l ? l : L""; }


    typedef UnitIterator<UnitSystem>    const_iterator;

    ECOBJECTS_EXPORT const_iterator         begin() const;
    ECOBJECTS_EXPORT const_iterator         end() const;
    ECOBJECTS_EXPORT bool                   IncludesUnit (UnitCR unit) const;

    ECOBJECTS_EXPORT static UnitSystemP     Create (WCharCP name, WCharCP shortLabel, WCharCP longLabel);
    ECOBJECTS_EXPORT static UnitSystemR     GetStandard (StandardUnitSystem id);
    ECOBJECTS_EXPORT static UnitSystemP     GetByName (WCharCP name);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Dimension
    {
private:
    WCharCP             m_name;
    WString             m_displayName;
    WString             m_description;
    WString             m_derivation;
    
    Dimension (WCharCP displayName, WCharCP desc, WCharCP deriv) : m_name(NULL), m_displayName(displayName), m_description(desc), m_derivation(deriv) { }
public:
    WCharCP             GetName() const             { return m_name; }
    WCharCP             GetDisplayName() const      { return m_displayName.c_str(); }
    WCharCP             GetDescription() const      { return m_description.c_str(); }
    WCharCP             GetDerivation() const       { return m_derivation.c_str(); }
    void                SetDisplayName (WCharCP n)  { m_displayName = n ? n : L""; }
    void                SetDescription (WCharCP d)  { m_description = d ? d : L""; }
    void                SetDerivation (WCharCP d)   { m_derivation = d ? d : L""; }


    typedef UnitIterator<Dimension> const_iterator;

    ECOBJECTS_EXPORT UnitP               AddUnit (UnitConverterCR converter, UnitSystemCR system, UnitCP baseUnit, WCharCP name, WCharCP shortLabel, WCharCP longLabel);
    ECOBJECTS_EXPORT KindOfQuantityP     AddKindOfQuantity (WCharCP name, WCharCP description, KindOfQuantityCP parentKOQ = NULL);

    ECOBJECTS_EXPORT bool                IncludesUnit (UnitCR u) const;
    ECOBJECTS_EXPORT const_iterator      begin() const;
    ECOBJECTS_EXPORT const_iterator      end() const;

    ECOBJECTS_EXPORT static DimensionP   Create (WCharCP name, WCharCP displayName, WCharCP description, WCharCP derivation);
    ECOBJECTS_EXPORT static DimensionR   GetStandard (StandardDimension id);
    ECOBJECTS_EXPORT static DimensionP   GetByName (WCharCP name);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct KindOfQuantity
    {
private:
    friend KindOfQuantityP Dimension::AddKindOfQuantity (WCharCP, WCharCP, KindOfQuantityCP);

    WCharCP             m_name;
    WString             m_description;
    DimensionCR         m_dimension;
    KindOfQuantityCP    m_parent;

    KindOfQuantity (WCharCP desc, DimensionCR dimension, KindOfQuantityCP parent)
        : m_name(NULL), m_description(desc), m_dimension(dimension), m_parent(parent) { }
public:
    WCharCP             GetName() const                         { return m_name; }
    WCharCP             GetDescription() const                  { return m_description.c_str(); }
    void                SetDescription (WCharCP d)              { m_description = d ? d : L""; }

    DimensionCR         GetDimension() const                    { return m_dimension; }
    KindOfQuantityCP    GetParent() const                       { return m_parent; }
    KindOfQuantityCR    GetBase() const                         { return m_parent ? m_parent->GetBase() : *this; }

    ECOBJECTS_EXPORT static KindOfQuantityR      GetStandard (StandardKindOfQuantity id);
    ECOBJECTS_EXPORT static KindOfQuantityP      GetByName (WCharCP name);
    };

/*---------------------------------------------------------------------------------**//**
* Every Unit belongs to a Dimension and a UnitSystem. Use Dimension::AddUnit() to
* create a Unit.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Unit
    {
private:
    friend UnitP Dimension::AddUnit (UnitConverterCR, UnitSystemCR, UnitCP, WCharCP, WCharCP, WCharCP);

    WCharCP                 m_name;
    WString                 m_shortLabel;
    WString                 m_longLabel;
    UnitConverter           m_converter;
    UnitSystemCR            m_system;
    DimensionCR             m_dimension;
    UnitCP                  m_baseUnit;

    Unit (WCharCP shortLabel, WCharCP longLabel, UnitConverterCR converter, UnitSystemCR system, DimensionCR dimension, UnitCP base)
        : m_name(NULL), m_shortLabel(shortLabel), m_longLabel(longLabel), m_converter(converter), m_system(system), m_dimension(dimension), m_baseUnit(base) { }
public:
    DimensionCR             GetDimension() const                    { return m_dimension; }
    UnitSystemCR            GetSystem() const                       { return m_system; }
    UnitCR                  GetBase() const                         { return m_baseUnit ? *m_baseUnit : *this; }
    UnitConverterCR         GetConverter() const                    { return m_converter; }
    bool                    IsCompatible (UnitCR other) const       { return &GetBase() == &other.GetBase() && &m_dimension == &other.m_dimension; }
    
    ECOBJECTS_EXPORT bool   ConvertTo (double& value, UnitCR target) const;

    WCharCP                 GetName() const                         { return m_name; }
    WCharCP                 GetLabel() const                        { return m_longLabel.c_str(); }
    WCharCP                 GetShortLabel() const                   { return m_shortLabel.c_str(); }
    void                    SetLabel (WCharCP l)                    { m_longLabel = l ? l : L""; }
    void                    SetShortLabel (WCharCP l)               { m_shortLabel = l ? l : L""; }

    ECOBJECTS_EXPORT static UnitR            GetStandard (StandardUnit id);
    ECOBJECTS_EXPORT static UnitP            GetByName (WCharCP name);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitsSchemaReader
    {
    // WIP_UNITS: error handling.
    // The managed implementation throws exceptions all over the place
    // Some callers may care what went wrong, others may not
    // Caller can specify:
    //  -Whether error messages should be logged or not
    //  -Whether we should halt on first error or try to continue
    // For now I am simply returning false if any error occurs, without logging
    enum Error
        {
        //...
        Error_Unknown
        };
    
    typedef bpair<Error, WString>       ErrorMessage;
    typedef bvector<ErrorMessage>       ErrorLog;
private:
    ErrorLog            m_errorLog;
    UnitsManager&       m_unitsManager;
    bool                m_haltOnError;
    bool                m_recordErrors;
    bool                m_applyDisplayLabels;

    void                Log (Error error, WCharCP msg) { m_errorLog.push_back (ErrorMessage (error, msg ? msg : L"")); }

    bool                ReadDimensions (EC::ECSchemaCR schema);
    bool                ReadDimension (EC::ECClassCR ecClass, EC::IECInstanceCR attr);
    bool                ReadUnitsAndKOQs (EC::ECSchemaCR schema);
    bool                ReadUnit (EC::ECClassCR ecClass, EC::IECInstanceCR attr);
    bool                ReadKindOfQuantity (EC::ECClassCR ecClass, EC::IECInstanceCR attr);
public:
    ECOBJECTS_EXPORT UnitsSchemaReader (bool recordErrors=false, bool haltOnError=false, bool applyDisplayLabels = true);

    // Process the schema for any Units, Dimensions, and KindOfQuantities or customizations and register them into the system
    ECOBJECTS_EXPORT bool                ReadUnitsInfo (EC::ECSchemaCR schema);
    // Returns a list of any logged errors in order of occurrence
    ErrorLog const&     GetErrorLog() const     { return m_errorLog; }
    void                ClearErrorLog()         { m_errorLog.clear(); }
    };

END_BENTLEY_EC_UNITS_NAMESPACE



