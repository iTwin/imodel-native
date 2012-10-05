/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECUnits/Units.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>

// WIP_UNITS: This is a very rough implementation, to try some things out.
//  -Where do these belong? Are they part of the EC universe?
//  -Names.

#define BEGIN_BENTLEY_EC_UNITS_NAMESPACE namespace Bentley { namespace EC { namespace Units {
#define END_BENTLEY_EC_UNITS_NAMESPACE   } } }

#define UNITS_TYPEDEFS(_name_)  \
        BEGIN_BENTLEY_EC_UNITS_NAMESPACE      \
            struct _name_;      \
            typedef _name_ *         _name_##P;  \
            typedef _name_ &         _name_##R;  \
            typedef _name_ const*    _name_##CP; \
            typedef _name_ const&    _name_##CR; \
        END_BENTLEY_EC_UNITS_NAMESPACE

UNITS_TYPEDEFS (Descriptor);
UNITS_TYPEDEFS (DimensionDescriptor);
UNITS_TYPEDEFS (UnitSystem);
UNITS_TYPEDEFS (Dimension);
UNITS_TYPEDEFS (KindOfQuantity);
UNITS_TYPEDEFS (Unit);
UNITS_TYPEDEFS (ICustomUnitConverter);
UNITS_TYPEDEFS (UnitConverter);

BEGIN_BENTLEY_EC_UNITS_NAMESPACE

enum UnitsStatus
    {
    UnitsStatus_Success,
    UnitsStatus_DuplicateName,
    UnitsStatus_InvalidName,
    UnitsStatus_NameNotRegistered,
    //...
    UnitsStatus_Error
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Descriptor
    {
private:
    WString         m_name;
    WString         m_shortLabel;
    WString         m_longLabel;
public:
    Descriptor (WCharCP name, WCharCP shortLabel, WCharCP longLabel);

    bool        operator< (DescriptorCR rhs) const     { return m_name < rhs.m_name; }
    bool        operator== (DescriptorCR rhs) const    { return m_name.Equals (rhs.m_name); }

    WCharCP         GetName() const                     { return m_name.c_str(); }
    WCharCP         GetLabel (bool getShort)            { return getShort ? m_shortLabel.c_str() : m_longLabel.c_str(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionDescriptor : Descriptor
    {
private:
    WString         m_description;
    WString         m_derivation;
public:
    DimensionDescriptor (WCharCP name, WCharCP shortLabel, WCharCP longLabel, WCharCP description, WCharCP derivation);
        
    WCharCP         GetDescription() const      { return m_description.c_str(); }
    WCharCP         GetDerivation() const       { return m_derivation.c_str(); }
    };

/*---------------------------------------------------------------------------------**//**
* Base class for a component of the units framework, such as Unit, UnitSystem, and Dimension.
* Components registered into the system have a unique name and ID, and a pair of labels.
* They can be standard components or user-supplied.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_ID, T_ID STANDARD_MAX, typename T_Descriptor = Descriptor> struct Component
    {
protected:
    T_Descriptor const*     m_descriptor;
    T_ID                    m_id;

    Component() : m_descriptor (NULL), m_id ((T_ID)-1) { }

    friend struct UnitsManager;
    void        Register (T_ID id, T_Descriptor const* desc) { m_id = id; m_descriptor = desc; }
public:
    WCharCP                         GetName() const             { return m_descriptor ? m_descriptor->GetName() : NULL; }
    WCharCP                         GetShortLabel() const       { return m_descriptor ? m_descriptor->GetLabel (true) : NULL; }
    WCharCP                         GetLongLabel() const        { return m_descriptor ? m_descriptor->GetLabel (false) : NULL; }
    bool                            IsStandard() const          { return this->m_id <= STANDARD_MAX; }
    bool                            IsRegistered() const        { return this->m_id != (T_ID)-1; }
    T_ID                            GetId() const               { return m_id; }
    };

// WIP_UNITS: limits...
typedef UInt8           UnitSystemId;
typedef UInt8           DimensionId;
typedef UInt16          KindOfQuantityId;
typedef UInt16          UnitId;

#define UNREGISTERED_UNITID ((UnitId)-1)
#define UNREGISTERED_UNITSYSTEMID ((UnitSystemId)-1)
#define UNREGISTERED_DIMENSIONID ((DimensionId)-1)
#define UNREGISTERED_KINDOFQUANTITYID ((KindOfQuantityId)-1)

enum StandardUnitSystem
    {
    StandardUnitSystem_None,
    StandardUnitSystem_SI,
    StandardUnitSystem_USCustomary,
    StandardUnitSystem_Both,

    StandardUnitSystem_MAX = StandardUnitSystem_Both
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
    double                          m_factor;
    union
        {
        double                      m_offset;
        ICustomUnitConverterP       m_customConverter;
        };
public:
    UnitConverter (bool isSlope = false) : m_type (isSlope ? UnitConversionType_Identity : UnitConversionType_Slope), m_factor (0.0), m_offset (0.0) { }
    UnitConverter (double factor) : m_type (UnitConversionType_Factor), m_factor (factor), m_offset (0.0) { BeAssert (0.0 != factor); }
    UnitConverter (double factor, double offset) : m_type (UnitConversionType_FactorAndOffset), m_factor (factor), m_offset (offset) { BeAssert (0.0 != factor); }
    UnitConverter (ICustomUnitConverterR customConverter);
    UnitConverter (UnitConverterCR other);
    UnitConverter& operator= (UnitConverterCR other);
    ~UnitConverter();

    double          ToBase (double valueInThisUnit) const;
    double          FromBase (double valueInBaseUnit) const;
    };

/*---------------------------------------------------------------------------------**//**
* UnitSystem is simply a way to categorize Units, for example SI and English.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitSystem : Component <UnitSystemId, StandardUnitSystem_MAX>
    {
public:
    static UnitsStatus          GetByName (UnitSystem& unitSystem, WCharCP name);
    static UnitsStatus          GetById (UnitSystem& unitSystem, UnitSystemId id);
    static UnitsStatus          Create (UnitSystem& unitSystem, DescriptorCR descriptor);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Dimension : Component <DimensionId, StandardDimension_MAX, DimensionDescriptor>
    {
public:
    UnitsStatus                 CreateUnit (Unit& unit, UnitConverterCR converter, UnitSystemId systemId, DescriptorCR descriptor);
    UnitsStatus                 CreateKindOfQuantity (KindOfQuantity& koq, KindOfQuantityId baseKoqId, KindOfQuantityId parentKoqId, DescriptorCR& descriptor);

    static UnitsStatus          GetByName (Dimension& dimension, WCharCP name);
    static UnitsStatus          GetById (Dimension& dimension, DimensionId id);
    static UnitsStatus          Create (Dimension& dimension, DimensionDescriptorCR descriptor);
    };

/*---------------------------------------------------------------------------------**//**
* Every KindOfQuantity belongs to a Dimension. Use Dimension::CreateKindOfQuantity()
* to create a new KindOfQuantity.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct KindOfQuantity : Component <KindOfQuantityId, StandardKindOfQuantity_MAX>
    {
private:
    DimensionId                 m_dimensionId;
    KindOfQuantityId            m_parentKoqId;
    KindOfQuantityId            m_baseKoqId;
public:
    static UnitsStatus          GetByName (KindOfQuantity& koq, WCharCP name);
    static UnitsStatus          GetById (KindOfQuantity& koq, KindOfQuantityId id);
    };

/*---------------------------------------------------------------------------------**//**
* Every Unit belongs to a Dimension and a UnitSystem. Use Dimension::CreateUnit() to
* create a Unit.
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Unit : Component <UnitId, StandardUnit_MAX>
    {
private:
    UnitId          m_baseUnitId;
    DimensionId     m_dimensionId;
    UnitSystemId    m_unitSystemId;
    UnitConverter   m_unitConverter;
public:
    UnitConverterCR         GetConverter() const { return m_unitConverter; }
    bool                    IsConvertible (UnitCR other) const
        {
        return m_baseUnitId == other.m_baseUnitId && m_dimensionId == other.m_dimensionId;
        }

    static UnitsStatus      GetByName (Unit& unit, WCharCP name);
    static UnitsStatus      GetById (Unit& unit, UnitId id);
    };

END_BENTLEY_EC_UNITS_NAMESPACE



