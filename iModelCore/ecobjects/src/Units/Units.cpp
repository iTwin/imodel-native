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

using namespace Bentley::EC;

static WCharCP const  KOQSchemaName                     = L"KindOfQuantity_Schema";
static WCharCP const  DimensionSchemaName               = L"Dimension_Schema";
static WCharCP const  UnitSchemaName                    = L"Units_Schema";
static WCharCP const  FACTOR_CONVERTER                  = L"Factor Converter";
static WCharCP const  FACTOR_OFFSET_CONVERTER           = L"Factor Offset Converter";
static WCharCP const  BASEUNIT_CONVERTER                = L"BaseUnit Converter";
static WCharCP const  NOOP_CONVERTER                    = L"NoOp Converter";
static WCharCP const  SLOPE_CONVERTER                   = L"Slope Converter";
static WCharCP const  DIMENSION_CUSTOMATTRIBUTE         = L"Dimension_Attributes";
static WCharCP const  KOQ_CUSTOMATTRIBUTE               = L"KindOfQuantity_Attributes";
static WCharCP const  UNIT_CUSTOMATTRIBUTE              = L"Unit_Attributes";
static WCharCP const  DIMENSION_PROPNAME                = L"Dimension";
static WCharCP const  BASEUNIT_PROPNAME                 = L"BaseUnit";
static WCharCP const  SHORTLABEL_PROPNAME               = L"ShortLabel";
static WCharCP const  LABEL_PROPNAME                    = L"Label";
static WCharCP const  DIMENSION_DISPLAYLABEL_PROPNAME   = L"DisplayName";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitConverter::UnitConverter (ICustomUnitConverter& customConverter)
    : m_type (UnitConversionType_Custom), m_customConverter (&customConverter)
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
    typedef bvector<UnitP>                          UnitList;
    typedef bvector<KindOfQuantityP>                KOQList;
    typedef bvector<DimensionP>                     DimensionList;
    typedef bvector<UnitSystemP>                    UnitSystemList;

    // Note that Unit, KOQ, Dimension, and UnitSystem store their names as a pointer into the WString key in one of the maps below, to avoid storing each name twice.
    typedef bmap<WString, Unit>                     UnitMap;
    typedef bmap<WString, KindOfQuantity>           KOQMap;
    typedef bmap<WString, Dimension>                DimensionMap;
    typedef bmap<WString, UnitSystem>               UnitSystemMap;

    UnitList        m_unitsById;
    KOQList         m_koqsById;
    DimensionList   m_dimensionsById;
    UnitSystemList  m_systemsById;
    UnitMap         m_unitsByName;
    KOQMap          m_koqsByName;
    DimensionMap    m_dimensionsByName;
    UnitSystemMap   m_systemsByName;

    UnitsManager();

    void                InitializeStandardUnitSystems();
public:
    UnitR               GetStandardUnit (StandardUnit id) const             { return *m_unitsById[id]; }
    KindOfQuantityR     GetStandardKOQ (StandardKindOfQuantity id) const    { return *m_koqsById[id]; }
    DimensionR          GetStandardDimension (StandardDimension id) const   { return *m_dimensionsById[id]; }
    UnitSystemR         GetStandardSystem (StandardUnitSystem id) const     { return *m_systemsById[id]; }

    UnitP             GetUnitByName (WCharCP name)
        {
        UnitMap::iterator iter = m_unitsByName.find (name);
        return iter != m_unitsByName.end() ? &iter->second : NULL;
        }

    KindOfQuantityP   GetKOQByName (WCharCP name)
        {
        KOQMap::iterator iter = m_koqsByName.find (name);
        return iter != m_koqsByName.end() ? &iter->second : NULL;
        }

    DimensionP        GetDimensionByName (WCharCP name)
        {
        DimensionMap::iterator iter = m_dimensionsByName.find (name);
        return iter != m_dimensionsByName.end() ? &iter->second : NULL;
        }

    UnitSystemP       GetSystemByName (WCharCP name)
        {
        // Note: the managed impl ignored case, and schemas are written with inconsistent case, so we have to do the same...
        WString lName (name);
        lName.ToLower();
        UnitSystemMap::iterator iter = m_systemsByName.find (lName);
        return iter != m_systemsByName.end() ? &iter->second : NULL;
        }

    UnitSystemP       AddSystem (WCharCP& name, UnitSystemCR system)
        {
        WString lName (name);
        lName.ToLower();
        UnitSystemMap::iterator pos = m_systemsByName.lower_bound (lName.c_str());
        if (pos == m_systemsByName.end() || !pos->first.Equals (lName))
            {
            pos = m_systemsByName.insert (pos, UnitSystemMap::value_type (lName, system));
            name = pos->first.c_str();
            return &pos->second;
            }
        else
            return NULL;    // name already exists
        }

    DimensionP          AddDimension (WCharCP& name, DimensionCR dim)
        {
        DimensionMap::iterator pos = m_dimensionsByName.lower_bound (name);
        if (pos == m_dimensionsByName.end() || !pos->first.Equals (name))
            {
            pos = m_dimensionsByName.insert (pos, DimensionMap::value_type (name, dim));
            name = pos->first.c_str();
            return &pos->second;
            }
        else
            return NULL;
        }

    KindOfQuantityP     AddKindOfQuantity (WCharCP& name, KindOfQuantityCR koq)
        {
        KOQMap::iterator pos = m_koqsByName.lower_bound (name);
        if (pos == m_koqsByName.end() || !pos->first.Equals (name))
            {
            pos = m_koqsByName.insert (pos, KOQMap::value_type (name, koq));
            name = pos->first.c_str();
            return &pos->second;
            }
        else
            return NULL;
        }

    UnitP               AddUnit (WCharCP& name, UnitCR unit)
        {
        UnitMap::iterator pos = m_unitsByName.lower_bound (name);
        if (pos == m_unitsByName.end() || !pos->first.Equals (name))
            {
            pos = m_unitsByName.insert (pos, UnitMap::value_type (name, unit));
            name = pos->first.c_str();
            return &pos->second;
            }
        else
            return NULL;
        }

    UnitMap const&          GetAllUnits() const { return m_unitsByName; }

    static UnitsManager&    GetManager()
        {
        // WIP_UNITS: thread-local
        static UnitsManager mgr;
        return mgr;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitsManager::UnitsManager() : m_unitsById(StandardUnit_MAX), m_koqsById(StandardKindOfQuantity_MAX), m_dimensionsById(StandardDimension_MAX), m_systemsById(StandardUnitSystem_MAX)
    {
    // WIP_UNITS: populate standard lists. Maybe the simplest way would be to add a custom attribute containing the standard ID to all the standard definitions
    // Initialize standard UnitSystems. Apparently nobody ever creates new ones via schemas?
    // Read standard schemas.
    InitializeStandardUnitSystems();

    UnitsSchemaReader reader (false, true, false);
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext (NULL, false);
    SchemaKey key (DimensionSchemaName, 1, 0);  // WIP_FUSION: why does LocateSchema() take a non-const SchemaKey...?
    ECSchemaPtr schema = context->LocateSchema (key, SCHEMAMATCHTYPE_LatestCompatible);
    if (schema.IsNull() || !reader.ReadUnitsInfo (*schema))
        { BeAssert (false); return; }

    key.m_schemaName = KOQSchemaName;
    key.m_versionMinor = 1;
    schema = context->LocateSchema (key, SCHEMAMATCHTYPE_LatestCompatible);
    if (schema.IsNull() || !reader.ReadUnitsInfo (*schema))
        { BeAssert (false); return; }

    key.m_schemaName = UnitSchemaName;
    key.m_versionMinor = 0;
    schema = context->LocateSchema (key, SCHEMAMATCHTYPE_LatestCompatible);
    if (schema.IsNull() || !reader.ReadUnitsInfo (*schema))
        { BeAssert (false); return; }

    // WIP_UNITS: Units customization schemas...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsManager::InitializeStandardUnitSystems()
    {
    m_systemsById[StandardUnitSystem_None] = UnitSystem::Create (L"none", NULL, NULL);
    m_systemsById[StandardUnitSystem_SI] = UnitSystem::Create (L"si", NULL, NULL);
    m_systemsById[StandardUnitSystem_USCustomary] = UnitSystem::Create (L"usCustomary", NULL, NULL);
    m_systemsById[StandardUnitSystem_Both] = UnitSystem::Create (L"both", NULL, NULL);
    }
    
static WCharCP PASS_NULL_AS_EMPTY (WCharCP in) { return in ? in : L""; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitSystemP UnitSystem::Create (WCharCP name, WCharCP shortLabel, WCharCP longLabel)
    {
    PRECONDITION (NULL != name && 0 != *name, NULL);

    WCharCP nameP = name;
    UnitSystemP system = UnitsManager::GetManager().AddSystem (nameP, UnitSystem (PASS_NULL_AS_EMPTY(shortLabel), PASS_NULL_AS_EMPTY(longLabel)));
    if (NULL != system)
        system->m_name = nameP;

    return system;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionP Dimension::Create (WCharCP name, WCharCP displayName, WCharCP description, WCharCP derivation)
    {
    PRECONDITION (NULL != name && 0 != *name, NULL);

    WCharCP nameP = name;
    DimensionP dimension = UnitsManager::GetManager().AddDimension (nameP, Dimension (displayName ? displayName : name, PASS_NULL_AS_EMPTY(description), PASS_NULL_AS_EMPTY(derivation)));
    if (NULL != dimension)
        dimension->m_name = nameP;

    return dimension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
KindOfQuantityP Dimension::AddKindOfQuantity (WCharCP name, WCharCP desc, KindOfQuantityCP parent)
    {
    PRECONDITION (NULL != name && 0 != *name, NULL);
    PRECONDITION (NULL == parent || &parent->GetDimension() == this, NULL);

    WCharCP nameP = name;
    KindOfQuantityP koq = UnitsManager::GetManager().AddKindOfQuantity (nameP, KindOfQuantity (PASS_NULL_AS_EMPTY(desc), *this, parent));
    if (NULL != koq)
        koq->m_name = nameP;

    return koq;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP Dimension::AddUnit (UnitConverterCR converter, UnitSystemCR system, UnitCP base, WCharCP name, WCharCP shortLabel, WCharCP longLabel)
    {
    PRECONDITION (NULL != name && 0 != *name, NULL);
    
    WCharCP nameP = name;
    UnitP unit = UnitsManager::GetManager().AddUnit (nameP, Unit (PASS_NULL_AS_EMPTY(shortLabel), PASS_NULL_AS_EMPTY(longLabel), converter, system, *this, base));
    if (NULL != unit)
        unit->m_name = nameP;

    return unit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Dimension::_IncludesUnit (UnitCR u) const                   { return &u.GetDimension() == this; }
bool UnitSystem::_IncludesUnit (UnitCR u) const                  { return &u.GetSystem() == this; }
bool IUnitFilter::IncludesUnit (UnitCR u) const                 { return _IncludesUnit (u); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsCollection::const_iterator::MoveNext()
    {
    while (m_cur != m_end && !m_collection.IncludesUnit ((++m_cur)->second))
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitsCollection::const_iterator::const_iterator (UnitsCollection const& coll, bool isEnd)
    : m_collection(coll),
    m_cur(isEnd ? UnitsManager::GetManager().GetAllUnits().begin() : UnitsManager::GetManager().GetAllUnits().end()),
    m_end (isEnd ? m_cur : UnitsManager::GetManager().GetAllUnits().end())
    {
    if (!m_collection.IncludesUnit (m_cur->second))
        MoveNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::ConvertTo (double& value, UnitCR target) const
    {
    PRECONDITION (IsCompatible (target), false);

    value = target.GetConverter().FromBase (this->GetConverter().ToBase (value));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitSystemR UnitSystem::GetStandard (StandardUnitSystem id)                 { return UnitsManager::GetManager().GetStandardSystem (id); }
DimensionR Dimension::GetStandard (StandardDimension id)                    { return UnitsManager::GetManager().GetStandardDimension (id); }
KindOfQuantityR KindOfQuantity::GetStandard (StandardKindOfQuantity id)     { return UnitsManager::GetManager().GetStandardKOQ (id); }
UnitR Unit::GetStandard (StandardUnit id)                                   { return UnitsManager::GetManager().GetStandardUnit (id); }
UnitSystemP UnitSystem::GetByName (WCharCP name)                            { return UnitsManager::GetManager().GetSystemByName (name); }
DimensionP Dimension::GetByName (WCharCP name)                              { return UnitsManager::GetManager().GetDimensionByName (name); }
KindOfQuantityP KindOfQuantity::GetByName (WCharCP name)                    { return UnitsManager::GetManager().GetKOQByName (name); }
UnitP Unit::GetByName (WCharCP name)                                        { return UnitsManager::GetManager().GetUnitByName (name); }

#ifdef WIP_UNITS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static WStringR buildMessage (WStringR msg, WCharCP a, WCharCP b) { msg.clear(); msg.append (a); msg.append (b); return msg; }
static WStringR buildMessage (WStringR msg, WCharCP a, WCharCP b, WCharCP c) { buildMessage (msg, a, b); msg.append (c); return msg; }
static WStringR buildMessage (WStringR msg, WCharCP a, WCharCP b, WCharCP c, WCharCP d) { buildMessage (msg, a, b, c); msg.append (d); return msg; }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
UnitsSchemaReader::UnitsSchemaReader (bool recordErrors, bool haltOnError, bool applyDisplayLabels)
    : m_haltOnError(haltOnError), m_recordErrors(recordErrors), m_applyDisplayLabels (applyDisplayLabels), m_unitsManager(UnitsManager::GetManager())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadUnitsInfo(ECSchemaCR schema)
    {
    bool readDimensions = ReadDimensions(schema);
    if (!readDimensions && m_haltOnError)
        return false;

    return ReadUnitsAndKOQs(schema) ? readDimensions : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadDimensions(ECSchemaCR schema)
    {
    FOR_EACH (ECClassCP ecClass, schema.GetClasses())
        {
        IECInstancePtr attr = ecClass->GetCustomAttribute (DIMENSION_CUSTOMATTRIBUTE);
        if (attr.IsValid())
            {
            if (!ReadDimension (*ecClass, *attr) && m_haltOnError)
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadDimension (ECClassCR ecClass, IECInstanceCR attr)
    {
    WString errorMsg;
    
    WCharCP name = ecClass.GetName().c_str();
    DimensionP dimension = m_unitsManager.GetDimensionByName (name);
    if (NULL == dimension)
        {
        ECValue displayLabel, desc, deriv;
        WCharCP propName = DIMENSION_DISPLAYLABEL_PROPNAME;
        if (ECOBJECTS_STATUS_Success != attr.GetValue (displayLabel, propName) ||
            ECOBJECTS_STATUS_Success != attr.GetValue (desc, propName=L"DimensionalDescription") ||
            ECOBJECTS_STATUS_Success != attr.GetValue (deriv, propName=L"DimensionalDerivation"))
            {
            if (m_recordErrors)
                { /* WIP_UNITS: log? Log (Error_FailedToReadDimension, buildMessage (errorMsg, L"Dimension ", ecClass.GetName().c_str(), L" missing property ", propName)); */ }
            
            return !m_haltOnError;
            }

        dimension = Dimension::Create (name, displayLabel.GetString(), desc.GetString(), deriv.GetString());
        if (NULL == dimension)
            return false;
        }

    if (m_applyDisplayLabels)
        {
        if (ecClass.GetIsDisplayLabelDefined() && !ecClass.GetDisplayLabel().empty())
            { /*WIP_UNITS: what? */ }

        // WIP_UNITS: What is up with two custom labels, a name, and a long name?
        }
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadUnitsAndKOQs(ECSchemaCR schema)
    {
    FOR_EACH (ECClassCP ecClass, schema.GetClasses())
        {
        IECInstancePtr attr = ecClass->GetCustomAttribute (KOQ_CUSTOMATTRIBUTE);
        if (attr.IsValid())
            {
            if (!ReadKindOfQuantity (*ecClass, *attr) && m_haltOnError)
                return false;
            }
        else if ((attr = ecClass->GetCustomAttribute (UNIT_CUSTOMATTRIBUTE)).IsValid())
            {
            if (!ReadUnit (*ecClass, *attr) && m_haltOnError)
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadKindOfQuantity (ECClassCR ecClass, IECInstanceCR attr)
    {
    WCharCP name = ecClass.GetName().c_str();
    KindOfQuantityP koq = m_unitsManager.GetKOQByName (name);
    if (NULL == koq)
        {
        KindOfQuantityP parentKOQ = NULL;
        if (ecClass.HasBaseClasses())
            {
            if (ecClass.GetBaseClasses().size() > 1)
                { BeAssert (false); return false; }
            
            ECClassCP parentClass = ecClass.GetBaseClasses()[0];
            parentKOQ = m_unitsManager.GetKOQByName (parentClass->GetName().c_str());
            if (NULL == parentKOQ)
                {
                IECInstancePtr parentAttr = parentClass->GetCustomAttribute (KOQ_CUSTOMATTRIBUTE);
                if (!parentAttr.IsValid() || !ReadKindOfQuantity (*parentClass, *parentAttr))
                    { BeAssert (false); return false; }

                parentKOQ = m_unitsManager.GetKOQByName (parentClass->GetName().c_str());
                }
            }

        ECValue v;
        DimensionP dimension;
        if (ECOBJECTS_STATUS_Success != attr.GetValue (v, L"Dimension") || NULL == (dimension = m_unitsManager.GetDimensionByName (v.GetString())))
            return false;

        WCharCP description = ECOBJECTS_STATUS_Success == attr.GetValue (v, L"Description") ? v.GetString() : NULL;
        koq = dimension->AddKindOfQuantity (name, description, parentKOQ);
        if (NULL == koq)
            return false;
        }

    if (m_applyDisplayLabels)
        {
        // WIP_UNITS
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsSchemaReader::ReadUnit (ECClassCR ecClass, IECInstanceCR attr)
    {
    WCharCP name = ecClass.GetName().c_str();
    UnitP unit = m_unitsManager.GetUnitByName (name);
    if (NULL == unit)
        {
        ECValue dimensionName, systemName, conversionTypeName, baseName;
        if (ECOBJECTS_STATUS_Success != attr.GetValue (dimensionName, DIMENSION_PROPNAME) ||
            ECOBJECTS_STATUS_Success != attr.GetValue (systemName, L"UnitSystem") ||
            ECOBJECTS_STATUS_Success != attr.GetValue (conversionTypeName, L"ConversionType") ||
            ECOBJECTS_STATUS_Success != attr.GetValue (baseName, BASEUNIT_PROPNAME))
            return false;

        UnitP baseUnit = NULL;
        DimensionP dimension;
        UnitSystemP system = m_unitsManager.GetSystemByName (systemName.GetString());
        if (NULL == system)
            system = &m_unitsManager.GetStandardSystem (StandardUnitSystem_None);
        
        if (NULL == (dimension = m_unitsManager.GetDimensionByName (dimensionName.GetString())))
            return false;
        else if (0 != wcscmp (name, baseName.GetString()) && NULL == (baseUnit = m_unitsManager.GetUnitByName (baseName.GetString())))
            {
            IECInstancePtr baseUnitAttr;
            ECClassCP baseUnitClass = ecClass.GetSchema().GetClassP (baseName.GetString());  // WIP_UNITS: Do we ever end up here with base units in a referenced schema which hasn't been processed yet?
            if (NULL == baseUnitClass ||
                (baseUnitAttr = baseUnitClass->GetCustomAttribute (UNIT_CUSTOMATTRIBUTE)).IsNull() ||
                !ReadUnit (*baseUnitClass, *baseUnitAttr) ||
                NULL == (baseUnit = m_unitsManager.GetUnitByName (baseName.GetString()))
               )
                return false;
            }

        bool isSlopeConverter = 0 == wcscmp (conversionTypeName.GetString(), SLOPE_CONVERTER);
        bool isFactorOffsetConverter = !isSlopeConverter && 0 == wcscmp (conversionTypeName.GetString(), FACTOR_OFFSET_CONVERTER);
        UnitConverter converter (isSlopeConverter);
        if (isFactorOffsetConverter || 0 == wcscmp (conversionTypeName.GetString(), FACTOR_CONVERTER))
            {
            ECValue v;
            if (ECOBJECTS_STATUS_Success != attr.GetValue (v, L"ConversionFactor"))
                return false;

            double conversionFactor = v.GetDouble();
            if (isFactorOffsetConverter)
                {
                if (ECOBJECTS_STATUS_Success != attr.GetValue (v, L"ConversionOffset"))
                    return false;

                converter = UnitConverter (conversionFactor, v.GetDouble());
                }
            else
                converter = UnitConverter (conversionFactor);
            }
        else if (!isSlopeConverter && 0 != wcscmp (conversionTypeName.GetString(), BASEUNIT_CONVERTER) && 0 != wcscmp (conversionTypeName.GetString(), NOOP_CONVERTER))
            return false;   // unknown converter name

        unit = dimension->AddUnit (converter, *system, baseUnit, name, NULL, NULL);
        if (NULL == unit)
            return false;
        }

    if (m_applyDisplayLabels)
        {
        // WIP_UNITS
        }

    return true;
    }

END_BENTLEY_EC_UNITS_NAMESPACE

