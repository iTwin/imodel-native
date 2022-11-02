/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

EC_TYPEDEFS (LegacyKoqLocater);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static Utf8CP const  KOQ_SCHEMA                        = "KindOfQuantity_Schema";

static Utf8CP const  UNIT_ATTRIBUTES                   = "Unit_Attributes";
static Utf8CP const  KOQ_ATTRIBUTES                    = "KindOfQuantity_Attributes";
static Utf8CP const  IS_UNIT_SYSTEM_SCHEMA             = "IsUnitSystemSchema";

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

typedef RefCountedPtr<LegacyKoqLocater> LegacyKoqLocaterPtr;

//---------------------------------------------------------------------------------
// Because Graphite loads ECClasses from schemas dynamically, ECSchemas are not pre-
// processed for units info and units info is not cached.
// @bsistruct
//+---------------+---------------+---------------+---------------+---------------+------*/
struct LegacyKoqLocater : RefCounted<IECClassLocater>
    {
    private:
        ECSchemaReadContextPtr   m_context;
        ECSchemaPtr              m_koqSchema;
        static LegacyKoqLocaterPtr s_unitsECClassLocaterPtr;

        bool Initialize();

        ECClassCP _LocateClass (Utf8CP schemaName, Utf8CP className);

    protected:
        LegacyKoqLocater () {}
        ~LegacyKoqLocater() {}

    public:
        ECOBJECTS_EXPORT static LegacyKoqLocaterPtr Create();
        ECOBJECTS_EXPORT static bool LoadUnitsSchemas (ECSchemaReadContextR context);
    };
LegacyKoqLocaterPtr LegacyKoqLocater::s_unitsECClassLocaterPtr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LegacyKoqLocaterPtr LegacyKoqLocater::Create()
    {
    if (s_unitsECClassLocaterPtr.IsNull())
        s_unitsECClassLocaterPtr = new LegacyKoqLocater();
    return s_unitsECClassLocaterPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyKoqLocater::LoadUnitsSchemas (ECSchemaReadContextR context)
    {
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    ECSchemaPtr koqSchema = context.LocateSchema (keyKoqSchema, SchemaMatchType::LatestWriteCompatible);
    POSTCONDITION (koqSchema.IsValid() && "Cannot load KOQ_SCHEMA", false);

    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyKoqLocater::Initialize()
    {
    if (!m_context.IsNull())
        return true;
    m_context = ECSchemaReadContext::CreateContext (nullptr, false);

    if (!LoadUnitsSchemas (*m_context))
        {
        m_context = NULL;
        return false;
        }
    
    SchemaKey keyKoqSchema (KOQ_SCHEMA, 1, 0);
    m_koqSchema = m_context->GetFoundSchema (keyKoqSchema, SchemaMatchType::LatestWriteCompatible);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP LegacyKoqLocater::_LocateClass (Utf8CP schemaName, Utf8CP className)
    {
    if (!Initialize())
        return NULL;
    
    if (0 == strcmp(KOQ_SCHEMA, schemaName))
        return m_koqSchema->GetClassCP(className);

    return nullptr;
    }

struct LegacyUnitsLocater
{
    static bool LocateUnitByKOQ (ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP koqName);
    static bool LocateUnitBySpecification(ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP specAttrName, Utf8CP specAttrValue);
    static bool GetUnitFromSpecifications (ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP specAttrName, Utf8CP specAttrValue, IECInstanceCR specsAttr);
    static bool LocateUnit (ECPropertyCR ecProp, Utf8StringR unitName);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnitsLocater::LocateUnitByKOQ (ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP koqName)
    {
    if (LocateUnitBySpecification (ecProp, unitName, KOQ_NAME, koqName))
        return true;

    const auto koqLocater = LegacyKoqLocater::Create();
    // Recurse on parent KOQ (KOQ ECClasses are defined in a hierarchy)
    ECClassCP koqClass =  koqLocater->LocateClass (KOQ_SCHEMA, koqName);
    if (NULL != koqClass)
        {
        if (koqClass->HasBaseClasses())
            return LocateUnitByKOQ (ecProp, unitName, koqClass->GetBaseClasses()[0]->GetName().c_str());
        else
            {
            // check Dimension of the base KOQ
            ECValue v;
            IECInstancePtr koqAttr = koqClass->GetCustomAttribute (UNIT_ATTRIBUTES, KOQ_ATTRIBUTES);
            if (koqAttr.IsValid() && ECObjectsStatus::Success == koqAttr->GetValue (v, DIMENSION) && !v.IsNull())
                return LocateUnitBySpecification (ecProp, unitName, DIMENSION_NAME, v.GetUtf8CP());
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* The domain schema can contain a UnitSpecifications custom attribute containing a list
* of UnitSpecification[] which associate Dimensions or Kinds of Quantity with a Unit.
* The schema may also reference a schema defining defaults.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnitsLocater::LocateUnitBySpecification (ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP specAttrName, Utf8CP specAttrValue)
    {
    ECSchemaCR schema = ecProp.GetClass().GetSchema();
    IECInstancePtr specsAttr = schema.GetCustomAttribute (UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    if (specsAttr.IsValid() && GetUnitFromSpecifications (ecProp, unitName, specAttrName, specAttrValue, *specsAttr))
        return true;
    
    // WIP_UNITS: figure out which (if any) referenced schema contains specifications once and cache it
    // (not sure how this works on graphite)
    for (ECSchemaReferenceList::value_type const& refSchemaEntry: schema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = refSchemaEntry.second;
        if (refSchema->GetCustomAttribute (UNIT_ATTRIBUTES, IS_UNIT_SYSTEM_SCHEMA).IsValid() && 
           (specsAttr = refSchema->GetCustomAttribute (UNIT_ATTRIBUTES,UNIT_SPECIFICATIONS)).IsValid())
            return GetUnitFromSpecifications (ecProp, unitName, specAttrName, specAttrValue, *specsAttr);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnitsLocater::GetUnitFromSpecifications (ECPropertyCR ecProp, Utf8StringR unitName, Utf8CP specAttrName, Utf8CP specAttrValue, IECInstanceCR specsAttr)
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
            if (ECObjectsStatus::Success == spec->GetValue (v, specAttrName) && !v.IsNull() && 0 == strcmp (v.GetUtf8CP(), specAttrValue))
                {
                // Find a UnitName defined on this UnitSpecification, and from that get the Unit
                if (ECObjectsStatus::Success == spec->GetValue (v, UNIT_NAME) && !v.IsNull() && !Utf8String::IsNullOrEmpty(v.GetUtf8CP()))
                    {
                    unitName = v.GetUtf8CP();
                    return true;
                    }
                else if (0 == strcmp (KOQ_NAME, specAttrName) && ECObjectsStatus::Success == spec->GetValue (v, DIMENSION_NAME) && !v.IsNull())
                    {
                    // Managed supports creating a KindOfQuantity simply by referencing it in conjunction with a DimensionName in a UnitSpecification....
                    if (LocateUnitBySpecification (ecProp, unitName, DIMENSION_NAME, v.GetUtf8CP()))
                        return true;
                    }
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnitsLocater::LocateUnit (ECPropertyCR ecProp, Utf8StringR unitName)
    {
    IECInstancePtr unitSpecAttr = ecProp.GetCustomAttribute (UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
    if (unitSpecAttr.IsNull())
        return false;

    // If the unit specification defines the Unit, we're done
    ECValue v;
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, UNIT_NAME) && !v.IsNull() && !Utf8String::IsNullOrEmpty(v.GetUtf8CP()))
        {
        unitName = v.GetUtf8CP();
        return true;
        }

    // If the unit specification defines a KindOfQuantity, locate a matching UnitSpecification at schema level defining the Unit
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, KOQ_NAME) && !v.IsNull() && LocateUnitByKOQ (ecProp, unitName, v.GetUtf8CP()))
        return true;

    // If the unit specification defines a Dimension, locate matching UnitSpecification defining the Unit
    if (ECObjectsStatus::Success == unitSpecAttr->GetValue (v, DIMENSION_NAME) && !v.IsNull() && LocateUnitBySpecification (ecProp, unitName, DIMENSION_NAME, v.GetUtf8CP()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnits::GetUnitForECProperty (Utf8StringR unitName, ECPropertyCR ecProp)
    {
    IECInstancePtr unitSpecAttr = ecProp.GetCustomAttribute (UNIT_SPECIFICATION);
    return unitSpecAttr.IsValid() ? LegacyUnitsLocater::LocateUnit (ecProp, unitName) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyUnits::GetDisplayUnitAndFormatForECProperty (Utf8StringR displayUnitName, Utf8StringR displayFormat, Utf8StringCR storedUnitName, ECPropertyCR ecProp)
    {
    IECInstancePtr attr = ecProp.GetCustomAttribute (UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
    if (attr.IsValid())
        {
        ECValue v;
        if (ECObjectsStatus::Success != attr->GetValue(v, DISPLAY_UNIT_NAME) || v.IsNull() || Utf8String::IsNullOrEmpty(v.GetUtf8CP()))
            displayUnitName = storedUnitName;
        else
            displayUnitName = v.GetUtf8CP();
        
        displayFormat.clear();
        if (ECObjectsStatus::Success == attr->GetValue (v, DISPLAY_FORMAT_STRING) && !v.IsNull())
            displayFormat = v.GetUtf8CP();

        return true;
        }

    return false;
    }

END_BENTLEY_ECOBJECT_NAMESPACE

