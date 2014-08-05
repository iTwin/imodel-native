/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Units/Units.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECUnits/Units.h>
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//IECClassLocaterPtr IECClassLocater::s_registeredClassLocater = NULL;

static WCharCP const  UNITS_SCHEMA                      = L"Units_Schema";
static WCharCP const  KOQ_SCHEMA                        = L"KindOfQuantity_Schema";

static WCharCP const  UNIT_ATTRIBUTES                   = L"Unit_Attributes";
static WCharCP const  KOQ_ATTRIBUTES                    = L"KindOfQuantity_Attributes";
static WCharCP const  IS_UNIT_SYSTEM_SCHEMA             = L"IsUnitSystemSchema";

static WCharCP const  CONVERSION_TYPE                   = L"ConversionType";
static WCharCP const  CONVERSION_FACTOR                 = L"ConversionFactor";
static WCharCP const  CONVERSION_OFFSET                 = L"ConversionOffset";
static WCharCP const  SHORT_LABEL                       = L"ShortLabel";
static WCharCP const  BASE_UNIT                         = L"BaseUnit";
static WCharCP const  UNIT_SPECIFICATION                = L"UnitSpecification";
static WCharCP const  UNIT_SPECIFICATIONS               = L"UnitSpecifications";
static WCharCP const  UNIT_SPECIFICATION_LIST           = L"UnitSpecificationList";
static WCharCP const  DISPLAY_UNIT_SPECIFICATION        = L"DisplayUnitSpecification";
static WCharCP const  UNIT_NAME                         = L"UnitName";
static WCharCP const  KOQ_NAME                          = L"KindOfQuantityName";
static WCharCP const  DIMENSION_NAME                    = L"DimensionName";
static WCharCP const  DISPLAY_UNIT_NAME                 = L"DisplayUnitName";
static WCharCP const  DISPLAY_FORMAT_STRING             = L"DisplayFormatString";
static WCharCP const  DIMENSION                         = L"Dimension";

static WCharCP const  FACTOR_CONVERTER                  = L"Factor Converter";
static WCharCP const  FACTOR_OFFSET_CONVERTER           = L"Factor Offset Converter";
static WCharCP const  BASEUNIT_CONVERTER                = L"BaseUnit Converter";
static WCharCP const  NOOP_CONVERTER                    = L"NoOp Converter";
static WCharCP const  SLOPE_CONVERTER                   = L"Slope Converter";

typedef RefCountedPtr<ECUnitsClassLocater> ECUnitsClassLocaterPtr;

//---------------------------------------------------------------------------------
// Because Graphite loads ECClasses from schemas dynamically, ECSchemas are not pre-
// processed for units info and units info is not cached.
// @bsistruct                                                    Paul.Connelly   09/12
//+---------------+---------------+---------------+---------------+---------------+------*/
struct ECUnitsClassLocater : RefCounted<IECClassLocater>
    {
private:
    ECSchemaReadContextPtr   m_context;
    ECSchemaPtr              m_unitsSchema, m_koqSchema;
    static ECUnitsClassLocaterPtr s_unitsECClassLocaterPtr;

    bool Initialize();

    ECClassCP _LocateClass (WCharCP schemaName, WCharCP className);

protected:
    ECUnitsClassLocater () {}
    ~ECUnitsClassLocater() {}

public:
    static ECUnitsClassLocaterPtr Create();
    ECOBJECTS_EXPORT static bool LoadUnitsSchemas (ECSchemaReadContextR context);
    };

ECUnitsClassLocaterPtr ECUnitsClassLocater::s_unitsECClassLocaterPtr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
double UnitConverter::ToBase (double val) const
    {
    switch (m_type)
        {
    case UnitConversionType_Identity:           return val;
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
    case UnitConversionType_Factor:             return val * m_factor;
    case UnitConversionType_Slope:              return 0.0 == val ? 0.0 : 1.0 / val;
    case UnitConversionType_FactorAndOffset:    return (val * m_factor) + m_offset;
    default:                                    BeAssert (false); return 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitConverter::IsEquivalent (UnitConverterCR other) const
    {
    if (m_type != other.m_type)
        return false;

    switch (m_type)
        {
        case UnitConversionType_FactorAndOffset:
            if (!DoubleOps::AlmostEqual (m_offset, other.m_offset))
                return false;
            // Fall-through intentional...
        case UnitConversionType_Factor:
            if (!DoubleOps::AlmostEqual (m_factor, other.m_factor))
                return false;
            break;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitSpec::ConvertTo (double& value, UnitSpecCR target) const
    {
    PRECONDITION (IsCompatible (target), false);

    value = target.GetConverter().FromBase (this->GetConverter().ToBase (value));
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECUnitsClassLocaterPtr ECUnitsClassLocater::Create()
    {
    if (s_unitsECClassLocaterPtr.IsNull())
        s_unitsECClassLocaterPtr = new ECUnitsClassLocater();
    return s_unitsECClassLocaterPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsClassLocater::LoadUnitsSchemas (ECSchemaReadContextR context)
    {
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    ECSchemaPtr koqSchema = context.LocateSchema (keyKoqSchema, SCHEMAMATCHTYPE_LatestCompatible);
    POSTCONDITION (koqSchema.IsValid() && "Cannot load KOQ_SCHEMA", false);

    SchemaKey keyUnitsSchema (UNITS_SCHEMA, 1, 0);
    ECSchemaPtr unitsSchema = context.LocateSchema (keyUnitsSchema, SCHEMAMATCHTYPE_LatestCompatible);
    POSTCONDITION (unitsSchema.IsValid() && "Cannot load UNITS_SCHEMA", false);

    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsClassLocater::Initialize()
    {
    if (!m_context.IsNull())
        return true;
    m_context = ECSchemaReadContext::CreateContext (NULL, false);

    if (!LoadUnitsSchemas (*m_context))
        {
        m_context = NULL;
        return false;
        }
    
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    m_koqSchema = m_context->GetFoundSchema (keyKoqSchema, SCHEMAMATCHTYPE_LatestCompatible);

    SchemaKey keyUnitsSchema (UNITS_SCHEMA, 1, 0);
    m_unitsSchema = m_context->LocateSchema (keyUnitsSchema, SCHEMAMATCHTYPE_LatestCompatible);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECUnitsClassLocater::_LocateClass (WCharCP schemaName, WCharCP className)
    {
    if (!Initialize())
        return NULL;

    if (UNITS_SCHEMA == schemaName)
        return m_unitsSchema->GetClassCP (className);
    else if (KOQ_SCHEMA == schemaName)
        return m_koqSchema->GetClassCP (className);
    /* else */
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Note that the managed Units framework expects the client app to load any extra Units or
* units customizations manually. Since schemas using Units from external units schemas
* do not require a reference to those schemas, we have no way of locating those units.
* Therefore, this implementation only supports units defined in the standard schemas.
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitLocater
    {
private:
    ECPropertyCR            m_ecprop;
    bool                    m_createIfNonStandard;
    IECClassLocaterR        m_ecUnitsClassLocater;

    static bool    GetUnitFromAttribute (UnitR unit, IECInstanceCR attr, WCharCP unitName);
    bool    LocateUnitBySpecification (UnitR unit, WCharCP propName, WCharCP propValue) const;
    bool    LocateUnitByKOQ (UnitR unit, WCharCP koqName) const;
    bool    GetUnitFromSpecifications (UnitR unit, WCharCP propName, WCharCP propValue, IECInstanceCR specsAttr) const;
public:
    UnitLocater (ECPropertyCR ecprop, bool createIfNonStandard, IECClassLocaterR ecUnitsClassLocater);

    bool    LocateUnit (UnitR unit) const;
    bool    LocateUnitByName (UnitR unit, WCharCP unitName) const;

    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UnitLocater::UnitLocater (ECPropertyCR ecprop, bool createIfNonStandard, IECClassLocaterR ecUnitsClassLocater) 
    : m_ecprop(ecprop), m_createIfNonStandard(createIfNonStandard) , m_ecUnitsClassLocater (ecUnitsClassLocater)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnit (UnitR unit) const
    {
    IECInstancePtr unitSpecAttr = m_ecprop.GetCustomAttribute (UNIT_SPECIFICATION);
    if (unitSpecAttr.IsNull())
        return false;

    // If the unit specification defines the Unit, we're done
    ECValue v;
    if (ECOBJECTS_STATUS_Success == unitSpecAttr->GetValue (v, UNIT_NAME) && !v.IsNull() && LocateUnitByName (unit, v.GetString()))
        return true;

    // If the unit specification defines a KindOfQuantity, locate a matching UnitSpecification at schema level defining the Unit
    if (ECOBJECTS_STATUS_Success == unitSpecAttr->GetValue (v, KOQ_NAME) && !v.IsNull() && LocateUnitByKOQ (unit, v.GetString()))
        return true;

    // If the unit specification defines a Dimension, locate matching UnitSpecification defining the Unit
    if (ECOBJECTS_STATUS_Success == unitSpecAttr->GetValue (v, DIMENSION_NAME) && !v.IsNull() && LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetString()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* In managed, if a Unit specified by name is not found, the system creates a Unit with
* that name, using the name as the label and itself as the base unit. This inherently
* forbids converting to/from this Unit to any other.
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnitByName (UnitR unit, WCharCP unitName) const
    {
    ECClassCP unitClass = m_ecUnitsClassLocater.LocateClass (UNITS_SCHEMA, unitName);

    IECInstancePtr unitAttr;
    if (NULL != unitClass && (unitAttr = unitClass->GetCustomAttribute (UNIT_ATTRIBUTES)).IsValid())
        return GetUnitFromAttribute (unit, *unitAttr, unitName);
    else if (m_createIfNonStandard)
        {
        unit = Unit (unitName, unitName, UnitConverter (false), unitName);
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::GetUnitFromAttribute (UnitR unit, IECInstanceCR attr, WCharCP unitName)
    {
    // 1. Extract conversion info
    ECValue v;
    if (ECOBJECTS_STATUS_Success != attr.GetValue (v, CONVERSION_TYPE) || v.IsNull())
        return false;

    bool isSlope = (0 == wcscmp (v.GetString(), SLOPE_CONVERTER));
    bool isFactorOffset = !isSlope && (0 == wcscmp (v.GetString(), FACTOR_OFFSET_CONVERTER));
    UnitConverter cvtr (isSlope);
    if (isFactorOffset || 0 == wcscmp (v.GetString(), FACTOR_CONVERTER))
        {
        if (ECOBJECTS_STATUS_Success != attr.GetValue (v, CONVERSION_FACTOR) || v.IsNull())
            return false;

        double conversionFactor = v.GetDouble();
        if (isFactorOffset)
            {
            if (ECOBJECTS_STATUS_Success != attr.GetValue (v, CONVERSION_OFFSET) || v.IsNull())
                return false;

            cvtr = UnitConverter (conversionFactor, v.GetDouble());
            }
        else
            cvtr = UnitConverter (conversionFactor);
        }
    else if (!isSlope && 0 != wcscmp (v.GetString(), BASEUNIT_CONVERTER) && 0 != wcscmp (v.GetString(), NOOP_CONVERTER))
        return false;       // unknown conversion type

    // 2. Extract label
    ECValue labelV;
    if (ECOBJECTS_STATUS_Success != attr.GetValue (labelV, SHORT_LABEL) || labelV.IsNull())
        return false;

    // 3. Extract base unit name
    WCharCP baseUnitName = (ECOBJECTS_STATUS_Success == attr.GetValue (v, BASE_UNIT) && !v.IsNull()) ? v.GetString() : unitName;

    unit = Unit (unitName, labelV.GetString(), cvtr, baseUnitName);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* The domain schema can contain a UnitSpecifications custom attribute containing a list
* of UnitSpecification[] which associate Dimensions or Kinds of Quantity with a Unit.
* The schema may also reference a schema defining defaults.
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnitBySpecification (UnitR unit, WCharCP propName, WCharCP propValue) const
    {
    ECSchemaCR schema = m_ecprop.GetClass().GetSchema();
    IECInstancePtr specsAttr = schema.GetCustomAttribute (UNIT_SPECIFICATIONS);
    if (specsAttr.IsValid() && GetUnitFromSpecifications (unit, propName, propValue, *specsAttr))
        return true;
    
    // WIP_UNITS: figure out which (if any) referenced schema contains specifications once and cache it
    // (not sure how this works on graphite)
    for (ECSchemaReferenceList::value_type const& refSchemaEntry: schema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = refSchemaEntry.second;
        if (refSchema->GetCustomAttribute (IS_UNIT_SYSTEM_SCHEMA).IsValid() && (specsAttr = refSchema->GetCustomAttribute (UNIT_SPECIFICATIONS)).IsValid())
            return GetUnitFromSpecifications (unit, propName, propValue, *specsAttr);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::GetUnitFromSpecifications (UnitR unit, WCharCP propName, WCharCP propValue, IECInstanceCR specsAttr) const
    {
    ECValue v;
    if (ECOBJECTS_STATUS_Success != specsAttr.GetValue (v, UNIT_SPECIFICATION_LIST))
        return false;

    UInt32 nSpecs = v.GetArrayInfo().GetCount();

    for (UInt32 i = 0; i < nSpecs; i++)
        {
        // Find the UnitSpecifications[] entry matching the input criterion
        IECInstancePtr spec;
        if (ECOBJECTS_STATUS_Success == specsAttr.GetValue (v, UNIT_SPECIFICATION_LIST, i) && (spec = v.GetStruct()).IsValid())
            {
            if (ECOBJECTS_STATUS_Success == spec->GetValue (v, propName) && !v.IsNull() && 0 == wcscmp (v.GetString(), propValue))
                {
                // Find a UnitName defined on this UnitSpecification, and from that get the Unit
                if (ECOBJECTS_STATUS_Success == spec->GetValue (v, UNIT_NAME) && !v.IsNull() && LocateUnitByName (unit, v.GetString()))
                    return true;
                else if (0 == wcscmp (KOQ_NAME, propName) && ECOBJECTS_STATUS_Success == spec->GetValue (v, DIMENSION_NAME) && !v.IsNull())
                    {
                    // Managed supports creating a KindOfQuantity simply by referencing it in conjunction with a DimensionName in a UnitSpecification....
                    if (LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetString()))
                        return true;
                    }
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnitByKOQ (UnitR unit, WCharCP koqName) const
    {
    if (LocateUnitBySpecification (unit, KOQ_NAME, koqName))
        return true;

    // Recurse on parent KOQ (KOQ ECClasses are defined in a hierarchy)
    ECClassCP koqClass =  m_ecUnitsClassLocater.LocateClass (KOQ_SCHEMA, koqName);
    if (NULL != koqClass)
        {
        if (koqClass->HasBaseClasses())
            return LocateUnitByKOQ (unit, koqClass->GetBaseClasses()[0]->GetName().c_str());
        else
            {
            // check Dimension of the base KOQ
            ECValue v;
            IECInstancePtr koqAttr = koqClass->GetCustomAttribute (KOQ_ATTRIBUTES);
            if (koqAttr.IsValid() && ECOBJECTS_STATUS_Success == koqAttr->GetValue (v, DIMENSION) && !v.IsNull())
                return LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetString());
            }
        }

    return false;
    }

////*---------------------------------------------------------------------------------**//**
////* @bsimethod                                                    Paul.Connelly   04/14
////+---------------+---------------+---------------+---------------+---------------+------*/
//bool Unit::GetUnitByName (UnitR unit, WCharCP unitName, bool createIfNotFound)
//    {
//    ECUnitsClassLocaterPtr classLocater = ECUnitsClassLocater::Create();
//    return UnitLocater::LocateUnitByName (unit, unitName, *classLocater, createIfNotFound);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetUnitForECProperty (UnitR unit, ECPropertyCR ecprop, IECClassLocaterR unitsECClassLocater)
    {
    IECInstancePtr unitSpecAttr = ecprop.GetCustomAttribute (UNIT_SPECIFICATION);
    return unitSpecAttr.IsValid() ? UnitLocater (ecprop, true, unitsECClassLocater).LocateUnit (unit) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ramanujam.Raman   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetUnitForECProperty (UnitR unit, ECPropertyCR ecprop)
    {
    ECUnitsClassLocaterPtr unitsECClassLocater =  ECUnitsClassLocater::Create();
    return GetUnitForECProperty (unit, ecprop, *unitsECClassLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetDisplayUnitAndFormatForECProperty (UnitR displayUnit, WStringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop)
    {
    ECUnitsClassLocaterPtr unitsECClassLocater =  ECUnitsClassLocater::Create();
    return GetDisplayUnitAndFormatForECProperty (displayUnit, displayFormat, storedUnit, ecprop, *unitsECClassLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetDisplayUnitAndFormatForECProperty (UnitR displayUnit, WStringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop, IECClassLocaterR unitsECClassLocater)
    {
    IECInstancePtr attr = ecprop.GetCustomAttribute (DISPLAY_UNIT_SPECIFICATION);
    if (attr.IsValid())
        {
        ECValue v;
        if (ECOBJECTS_STATUS_Success != attr->GetValue(v, DISPLAY_UNIT_NAME) || v.IsNull() || !UnitLocater(ecprop, false, unitsECClassLocater).LocateUnitByName(displayUnit, v.GetString()))
            displayUnit = storedUnit;
        
        displayFormat.clear();
        if (ECOBJECTS_STATUS_Success == attr->GetValue (v, DISPLAY_FORMAT_STRING) && !v.IsNull())
            displayFormat = v.GetString();

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::FormatValue (WStringR formatted, ECValueCR inputVal, ECPropertyCR ecprop, IECInstanceCP instance)
    {
    ECUnitsClassLocaterPtr unitsECClassLocater =  ECUnitsClassLocater::Create();
    return FormatValue (formatted, inputVal, ecprop, instance, *unitsECClassLocater);
    }

/*---------------------------------------------------------------------------------**//**
* This method was requested by Graphite team. They have a need to format Units values
* without dependency on DgnPlatform, where ECUnitsTypeAdapter lives.
* If DgnPlatform is present, the method will use the type adapter.
* Otherwise we will do the formatting here in ECObjects.
* (The difference is that DgnPlatform will search for and apply any unit label customization
* schemas present in the instance's DgnFile).
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::FormatValue (WStringR formatted, ECValueCR inputVal, ECPropertyCR ecprop, IECInstanceCP instance, IECClassLocaterR unitsECClassLocater)
    {
    Unit storedUnit;
    ECValue v (inputVal);

    // if GetForECProperty() fails, we have no valid UnitSpecification attribute
    if (v.IsNull() || !v.ConvertToPrimitiveType (PRIMITIVETYPE_Double) || !Unit::GetUnitForECProperty (storedUnit, ecprop, unitsECClassLocater))
        return false;

    // See if we've got a registered ECUnitsTypeAdapter from DgnPlatform
    IECTypeAdapter* typeAdapter;
    if (NULL != instance && NULL != (typeAdapter = ecprop.GetTypeAdapter()))
        return typeAdapter->ConvertToString (formatted, inputVal, *IECTypeAdapterContext::Create (ecprop, *instance));

    // No TypeAdapter
    WCharCP label = storedUnit.GetShortLabel();

    Unit displayUnit;
    WString fmt;
    if (Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, fmt, storedUnit, ecprop, unitsECClassLocater))
        {
        double displayValue = v.GetDouble();
        if (!displayUnit.IsCompatible (storedUnit) || !storedUnit.ConvertTo (displayValue, displayUnit))
            return false;

        v.SetDouble (displayValue);
        label = displayUnit.GetShortLabel();
        }

    formatted.clear();
    WCharCP fmtCP = fmt.empty() ? L"f" : fmt.c_str();
    if (!v.ApplyDotNetFormatting (formatted, fmtCP))
        return false;

    if (NULL != label)
        {
        formatted.append (1, ' ');
        formatted.append (label);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString UnitSpec::ToECExpressionString() const
    {
    WString str = m_baseUnitName;
    str.append (m_converter.ToECExpressionString());
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString UnitConverter::ToECExpressionString() const
    {
    WString str;
    if (UnitConversionType_Factor == m_type || UnitConversionType_FactorAndOffset == m_type)
        {
        str.Sprintf (L"::%f", m_factor);
        if (UnitConversionType_FactorAndOffset == m_type)
            str.Sprintf (L"::%f", m_offset);
        }

    return str;
    }

END_BENTLEY_ECOBJECT_NAMESPACE

