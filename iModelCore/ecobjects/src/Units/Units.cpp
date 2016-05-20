/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Units/Units.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECUnits/Units.h>
#include <ECUnits/ECUnitsClassLocater.h>
#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//IECClassLocaterPtr IECClassLocater::s_registeredClassLocater = NULL;

static Utf8CP const  UNITS_SCHEMA                      = "Units_Schema";
static Utf8CP const  KOQ_SCHEMA                        = "KindOfQuantity_Schema";

static Utf8CP const  UNIT_ATTRIBUTES                   = "Unit_Attributes";
static Utf8CP const  KOQ_ATTRIBUTES                    = "KindOfQuantity_Attributes";
static Utf8CP const  IS_UNIT_SYSTEM_SCHEMA             = "IsUnitSystemSchema";

static Utf8CP const  CONVERSION_TYPE                   = "ConversionType";
static Utf8CP const  CONVERSION_FACTOR                 = "ConversionFactor";
static Utf8CP const  CONVERSION_OFFSET                 = "ConversionOffset";
static Utf8CP const  SHORT_LABEL                       = "ShortLabel";
static Utf8CP const  BASE_UNIT                         = "BaseUnit";
static Utf8CP const  UNIT_SPECIFICATION                = "UnitSpecificationAttr";
static Utf8CP const  UNIT_SPECIFICATIONS               = "UnitSpecifications";
static Utf8CP const  UNIT_SPECIFICATION_LIST           = "UnitSpecificationList";
static Utf8CP const  DISPLAY_UNIT_SPECIFICATION        = "DisplayUnitSpecificationAttr";
static Utf8CP const  UNIT_NAME                         = "UnitName";
static Utf8CP const  KOQ_NAME                          = "KindOfQuantityName";
static Utf8CP const  DIMENSION_NAME                    = "DimensionName";
static Utf8CP const  DISPLAY_UNIT_NAME                 = "DisplayUnitName";
static Utf8CP const  DISPLAY_FORMAT_STRING             = "DisplayFormatString";
static Utf8CP const  DIMENSION                         = "Dimension";

static Utf8CP const  FACTOR_CONVERTER                  = "Factor Converter";
static Utf8CP const  FACTOR_OFFSET_CONVERTER           = "Factor Offset Converter";
static Utf8CP const  BASEUNIT_CONVERTER                = "BaseUnit Converter";
static Utf8CP const  NOOP_CONVERTER                    = "NoOp Converter";
static Utf8CP const  SLOPE_CONVERTER                   = "Slope Converter";


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

    bool    LocateUnitBySpecification (UnitR unit, Utf8CP propName, Utf8CP propValue) const;
    bool    LocateUnitByKOQ (UnitR unit, Utf8CP koqName) const;
    bool    GetUnitFromSpecifications (UnitR unit, Utf8CP propName, Utf8CP propValue, IECInstanceCR specsAttr) const;
public:
    UnitLocater (ECPropertyCR ecprop, bool createIfNonStandard, IECClassLocaterR ecUnitsClassLocater);

    bool    LocateUnit (UnitR unit) const;
    bool    LocateUnitByName (UnitR unit, Utf8CP unitName) const;

    static bool    GetUnitFromAttribute(UnitR unit, IECInstanceCR attr, Utf8CP unitName);
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
    IECInstancePtr unitSpecAttr = m_ecprop.GetCustomAttribute (UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
    if (unitSpecAttr.IsNull())
        return false;

    // If the unit specification defines the Unit, we're done
    ECValue v;
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, UNIT_NAME) && !v.IsNull() && LocateUnitByName (unit, v.GetUtf8CP()))
        return true;

    // If the unit specification defines a KindOfQuantity, locate a matching UnitSpecification at schema level defining the Unit
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, KOQ_NAME) && !v.IsNull() && LocateUnitByKOQ (unit, v.GetUtf8CP()))
        return true;

    // If the unit specification defines a Dimension, locate matching UnitSpecification defining the Unit
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, DIMENSION_NAME) && !v.IsNull() && LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetUtf8CP()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* In managed, if a Unit specified by name is not found, the system creates a Unit with
* that name, using the name as the label and itself as the base unit. This inherently
* forbids converting to/from this Unit to any other.
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnitByName (UnitR unit, Utf8CP unitName) const
    {
    ECClassCP unitClass = m_ecUnitsClassLocater.LocateClass (UNITS_SCHEMA, unitName);

    IECInstancePtr unitAttr;
    if (NULL != unitClass && (unitAttr = unitClass->GetCustomAttribute (UNIT_ATTRIBUTES, UNIT_ATTRIBUTES)).IsValid())
        return GetUnitFromAttribute (unit, *unitAttr, unitName);
    else if (m_createIfNonStandard && !Utf8String::IsNullOrEmpty(unitName))
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
bool UnitLocater::GetUnitFromAttribute (UnitR unit, IECInstanceCR attr, Utf8CP unitName)
    {
    // 1. Extract conversion info
    ECValue v;
    if (ECObjectsStatus::Success != attr.GetValue (v, CONVERSION_TYPE) || v.IsNull())
        return false;

    bool isSlope = (0 == strcmp (v.GetUtf8CP(), SLOPE_CONVERTER));
    bool isFactorOffset = !isSlope && (0 == strcmp (v.GetUtf8CP(), FACTOR_OFFSET_CONVERTER));
    UnitConverter cvtr (isSlope);
    if (isFactorOffset || 0 == strcmp (v.GetUtf8CP(), FACTOR_CONVERTER))
        {
        if (ECObjectsStatus::Success != attr.GetValue (v, CONVERSION_FACTOR) || v.IsNull())
            return false;

        double conversionFactor = v.GetDouble();
        if (isFactorOffset)
            {
            if (ECObjectsStatus::Success != attr.GetValue (v, CONVERSION_OFFSET) || v.IsNull())
                return false;

            cvtr = UnitConverter (conversionFactor, v.GetDouble());
            }
        else
            cvtr = UnitConverter (conversionFactor);
        }
    else if (!isSlope && 0 != strcmp (v.GetUtf8CP(), BASEUNIT_CONVERTER) && 0 != strcmp (v.GetUtf8CP(), NOOP_CONVERTER))
        return false;       // unknown conversion type

    // 2. Extract label
    ECValue labelV;
    if (ECObjectsStatus::Success != attr.GetValue (labelV, SHORT_LABEL) || labelV.IsNull())
        return false;

    // 3. Extract base unit name
    Utf8CP baseUnitName = (ECObjectsStatus::Success == attr.GetValue (v, BASE_UNIT) && !v.IsNull()) ? v.GetUtf8CP() : unitName;

    unit = Unit (unitName, labelV.GetUtf8CP(), cvtr, baseUnitName);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* The domain schema can contain a UnitSpecifications custom attribute containing a list
* of UnitSpecification[] which associate Dimensions or Kinds of Quantity with a Unit.
* The schema may also reference a schema defining defaults.
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::LocateUnitBySpecification (UnitR unit, Utf8CP propName, Utf8CP propValue) const
    {
    ECSchemaCR schema = m_ecprop.GetClass().GetSchema();
    IECInstancePtr specsAttr = schema.GetCustomAttribute (UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    if (specsAttr.IsValid() && GetUnitFromSpecifications (unit, propName, propValue, *specsAttr))
        return true;
    
    // WIP_UNITS: figure out which (if any) referenced schema contains specifications once and cache it
    // (not sure how this works on graphite)
    for (ECSchemaReferenceList::value_type const& refSchemaEntry: schema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = refSchemaEntry.second;
        if (refSchema->GetCustomAttribute (UNIT_ATTRIBUTES, IS_UNIT_SYSTEM_SCHEMA).IsValid() && 
		   (specsAttr = refSchema->GetCustomAttribute (UNIT_ATTRIBUTES,UNIT_SPECIFICATIONS)).IsValid())
            return GetUnitFromSpecifications (unit, propName, propValue, *specsAttr);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitLocater::GetUnitFromSpecifications (UnitR unit, Utf8CP propName, Utf8CP propValue, IECInstanceCR specsAttr) const
    {
    ECValue v;
    if (ECObjectsStatus::Success != specsAttr.GetValue (v, UNIT_SPECIFICATION_LIST))
        return false;

    uint32_t nSpecs = v.GetArrayInfo().GetCount();

    for (uint32_t i = 0; i < nSpecs; i++)
        {
        // Find the UnitSpecifications[] entry matching the input criterion
        IECInstancePtr spec;
        if (ECObjectsStatus::Success == specsAttr.GetValue (v, UNIT_SPECIFICATION_LIST, i) && (spec = v.GetStruct()).IsValid())
            {
            if (ECObjectsStatus::Success == spec->GetValue (v, propName) && !v.IsNull() && 0 == strcmp (v.GetUtf8CP(), propValue))
                {
                // Find a UnitName defined on this UnitSpecification, and from that get the Unit
                if (ECObjectsStatus::Success == spec->GetValue (v, UNIT_NAME) && !v.IsNull() && LocateUnitByName (unit, v.GetUtf8CP()))
                    return true;
                else if (0 == strcmp (KOQ_NAME, propName) && ECObjectsStatus::Success == spec->GetValue (v, DIMENSION_NAME) && !v.IsNull())
                    {
                    // Managed supports creating a KindOfQuantity simply by referencing it in conjunction with a DimensionName in a UnitSpecification....
                    if (LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetUtf8CP()))
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
bool UnitLocater::LocateUnitByKOQ (UnitR unit, Utf8CP koqName) const
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
            IECInstancePtr koqAttr = koqClass->GetCustomAttribute (UNIT_ATTRIBUTES, KOQ_ATTRIBUTES);
            if (koqAttr.IsValid() && ECObjectsStatus::Success == koqAttr->GetValue (v, DIMENSION) && !v.IsNull())
                return LocateUnitBySpecification (unit, DIMENSION_NAME, v.GetUtf8CP());
            }
        }

    return false;
    }

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   04/14
//+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetUnitByName (UnitR unit, Utf8CP unitName)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        return false;

    ECUnitsClassLocaterPtr classLocater = ECUnitsClassLocater::Create();

    ECClassCP unitClass = classLocater->LocateClass(UNITS_SCHEMA, unitName);

    IECInstancePtr unitAttr;
    if (NULL != unitClass && (unitAttr = unitClass->GetCustomAttribute(UNIT_ATTRIBUTES)).IsValid())
        return UnitLocater::GetUnitFromAttribute(unit, *unitAttr, unitName);

    return false;
    }

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
bool Unit::GetDisplayUnitAndFormatForECProperty (UnitR displayUnit, Utf8StringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop)
    {
    ECUnitsClassLocaterPtr unitsECClassLocater =  ECUnitsClassLocater::Create();
    return GetDisplayUnitAndFormatForECProperty (displayUnit, displayFormat, storedUnit, ecprop, *unitsECClassLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::GetDisplayUnitAndFormatForECProperty (UnitR displayUnit, Utf8StringR displayFormat, UnitCR storedUnit, ECPropertyCR ecprop, IECClassLocaterR unitsECClassLocater)
    {
    IECInstancePtr attr = ecprop.GetCustomAttribute (UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
    if (attr.IsValid())
        {
        ECValue v;
        if (ECObjectsStatus::Success != attr->GetValue(v, DISPLAY_UNIT_NAME) || v.IsNull() || !UnitLocater(ecprop, false, unitsECClassLocater).LocateUnitByName(displayUnit, v.GetUtf8CP()))
            displayUnit = storedUnit;
        
        displayFormat.clear();
        if (ECObjectsStatus::Success == attr->GetValue (v, DISPLAY_FORMAT_STRING) && !v.IsNull())
            displayFormat = v.GetUtf8CP();

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::FormatValue (Utf8StringR formatted, ECValueCR inputVal, ECPropertyCR ecprop, IECInstanceCP instance)
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
bool Unit::FormatValue (Utf8StringR formatted, ECValueCR inputVal, ECPropertyCR ecprop, IECInstanceCP instance, IECClassLocaterR unitsECClassLocater)
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
    Utf8CP label = storedUnit.GetShortLabel();

    Unit displayUnit;
    Utf8String fmt;
    if (Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, fmt, storedUnit, ecprop, unitsECClassLocater))
        {
        double displayValue = v.GetDouble();
        if (!displayUnit.IsCompatible (storedUnit) || !storedUnit.ConvertTo (displayValue, displayUnit))
            return false;

        v.SetDouble (displayValue);
        label = displayUnit.GetShortLabel();
        }

    formatted.clear();
    Utf8CP fmtCP = fmt.empty() ? "f" : fmt.c_str();
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
Utf8String UnitSpec::ToECExpressionString() const
    {
    Utf8String str = m_baseUnitName;
    str.append (m_converter.ToECExpressionString());
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UnitConverter::ToECExpressionString() const
    {
    Utf8String str;
    if (UnitConversionType_Factor == m_type || UnitConversionType_FactorAndOffset == m_type)
        {
        str.Sprintf ("::%f", m_factor);
        if (UnitConversionType_FactorAndOffset == m_type)
            str.Sprintf ("::%f", m_offset);
        }

    return str;
    }

END_BENTLEY_ECOBJECT_NAMESPACE

