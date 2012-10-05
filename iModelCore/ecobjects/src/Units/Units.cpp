/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Units/Units.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECUnits/Units.h>
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_EC_UNITS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
Descriptor::Descriptor (WCharCP name, WCharCP shortLabel, WCharCP longLabel)
    : m_name (name ? name : L""), m_shortLabel (shortLabel ? shortLabel : longLabel ? longLabel : L""), m_longLabel (longLabel ? longLabel : shortLabel ? shortLabel : L"")
    {
    BeAssert (NULL != name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDescriptor::DimensionDescriptor (WCharCP name, WCharCP shortLabel, WCharCP longLabel, WCharCP description, WCharCP derivation)
    : Descriptor (name, shortLabel, longLabel), m_description (description ? description : L""), m_derivation (derivation ? derivation : L"")
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitConverter::UnitConverter (ICustomUnitConverter& customConverter)
    : m_type (UnitConversionType_Custom), m_factor (0.0), m_customConverter (&customConverter)
    {
    customConverter.AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitConverter::UnitConverter (UnitConverter const& other)
    : m_type(other.m_type), m_factor(other.m_factor), m_offset(other.m_offset)
    {
    if (UnitConversionType_Custom == m_type)
        m_customConverter->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitConverter& UnitConverter::operator= (UnitConverter const& other)
    {
    if (this != &other)
        {
        if (UnitConversionType_Custom == other.m_type)
            other.m_customConverter->AddRef();

        if (UnitConversionType_Custom == m_type)
            m_customConverter->Release();

        m_type = other.m_type;
        m_factor = other.m_factor;
        m_offset = other.m_offset;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitConverter::~UnitConverter()
    {
    if (UnitConversionType_Custom == m_type)
        m_customConverter->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
double UnitConverter::ToBase (double val) const
    {
    switch (m_type)
        {
    case UnitConversionType_Identity:           return val;
    case UnitConversionType_Custom:             return m_customConverter->_ToBase (val);
    case UnitConversionType_Factor:             return val / m_factor;
    case UnitConversionType_Slope:              return 0.0 == val ? 0.0 : 1.0 / val; 
    case UnitConversionType_FactorAndOffset:    return (val - m_offset) / m_factor;
    default:                                    BeAssert (false); return 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
double UnitConverter::FromBase (double val) const
    {
    switch (m_type)
        {
    case UnitConversionType_Identity:           return val;
    case UnitConversionType_Custom:             return m_customConverter->_FromBase (val);
    case UnitConversionType_Factor:             return val * m_factor;
    case UnitConversionType_Slope:              return 0.0 == val ? 0.0 : 1.0 / val;
    case UnitConversionType_FactorAndOffset:    return (val * m_factor) + m_offset;
    default:                                    BeAssert (false); return 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitsManager
    {
private:
    typedef bvector<Unit>                           UnitList;
    typedef bvector<KindOfQuantity>                 KOQList;
    typedef bvector<Dimension>                      DimensionList;
    typedef bvector<UnitSystem>                     UnitSystemList;

    typedef bmap<Descriptor, UnitId>                UnitMap;
    typedef bmap<Descriptor, KindOfQuantityId>      KOQMap;
    typedef bmap<DimensionDescriptor, DimensionId>  DimensionMap;
    typedef bmap<Descriptor, UnitSystemId>          UnitSystemMap;

    UnitList        m_unitsById;
    KOQList         m_koqsById;
    DimensionList   m_dimensionsById;
    UnitSystemList  m_systemsById;
    UnitMap         m_unitsByName;
    KOQMap          m_koqsByName;
    DimensionMap    m_dimensionsByName;
    UnitSystemMap   m_systemsByName;
public:
    Unit const*             GetUnitById (UnitId id) const           { return id < m_unitsById.size() ? &m_unitsById[id] : NULL; }
    KindOfQuantity const*   GetKOQById (KindOfQuantityId id) const  { return id < m_koqsById.size() ? &m_koqsById[id] : NULL; }
    Dimension const*        GetDimensionById (DimensionId id) const { return id < m_dimensionsById.size() ? &m_dimensionsById[id] : NULL; }
    UnitSystem const*       GetSystemById (UnitSystemId id) const   { return id < m_systemsById.size() ? &m_systemsById[id] : NULL; }

    Unit const*             GetUnitByName (WCharCP name) const
        {
        UnitMap::const_iterator iter = m_unitsByName.find (Descriptor (name, NULL, NULL));
        return iter != m_unitsByName.end() ? GetUnitById (iter->second) : NULL;
        }

    KindOfQuantity const*   GetKOQByName (WCharCP name) const
        {
        KOQMap::const_iterator iter = m_koqsByName.find (Descriptor (name, NULL, NULL));
        return iter != m_koqsByName.end() ? GetKOQById (iter->second) : NULL;
        }

    Dimension const*        GetDimensionByName (WCharCP name) const
        {
        DimensionMap::const_iterator iter = m_dimensionsByName.find (DimensionDescriptor (name, NULL, NULL, NULL, NULL));
        return iter != m_dimensionsByName.end() ? GetDimensionById (iter->second) : NULL;
        }

    UnitSystem const*       GetSystemByName (WCharCP name) const
        {
        UnitSystemMap::const_iterator iter = m_systemsByName.find (Descriptor (name, NULL, NULL));
        return iter != m_systemsByName.end() ? GetSystemById (iter->second) : NULL;
        }

    static UnitsManager&    GetManager()
        {
        // WIP_UNITS: this ought to be thread-local
        static UnitsManager mgr;
        return mgr;
        }
    };

END_BENTLEY_EC_UNITS_NAMESPACE

