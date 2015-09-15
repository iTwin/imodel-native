/*--------------------------------------------------------------------------------------+
|
|  $Source: src/PresentationMetadataHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/PresentationMetadataHelper.h>

USING_NAMESPACE_BENTLEY_EC

static Utf8CP EDITOR_SCHEMA_NAME                = "EditorCustomAttributes";
static Utf8CP EXTENDEDTYPE_CLASSNAME            = "ExtendType";
static Utf8CP MEMBER_EXTENDEDTYPE_CLASSNAME     = "MemberExtendedType";
static Utf8CP PROPERTY_PRIORITY_CLASSNAME       = "PropertyPriority";
// static Utf8CP CLASS_PRIORITY_CLASSNAME          = "ClassPriority";
static Utf8CP CATEGORY_CLASSNAME                = "Category";
// static Utf8CP STANDARD_VALUES_CLASSNAME         = "StandardValues";
// static Utf8CP BOOLEAN_DISPLAY_CLASSNAME         = "BooleanDisplay";
// static Utf8CP BOOLEAN_EXPRESSION_CLASSNAME      = "UseBooleanInExpressions";
// static Utf8CP FORMAT_STRING_CLASSNAME           = "Format";
static Utf8CP MEMBERS_INDEPENDENT_CLASSNAME     = "MembersIndependent";
// static Utf8CP HIDE_PROPERTY_CLASSNAME           = "HideProperty";
// static Utf8CP HIDE_MEMBERS_CLASSNAME            = "HideMembers";
static Utf8CP ALWAYS_EXPAND_CLASSNAME           = "AlwaysExpand";
// static Utf8CP DWG_PROPERTYNAME_CLASSNAME        = "DWGProperty";
// static Utf8CP DWG_FORMAT_CLASSNAME              = "DWGFormat";
static Utf8CP REQUIRES_RELOAD_CLASSNAME         = "RequiresReload";
// static Utf8CP REQUIRES_REFRESH_CLASSNAME        = "RequiresRefresh";
static Utf8CP DONT_SHOW_NULL_PROPS_CLASSNAME    = "DontShowNullProperties";
static Utf8CP IGNOREZ_CLASSNAME                 = "IgnoreZ";
// static Utf8CP ARRAY_BEHAVIOR_CLASSNAME          = "ArrayBehaviorAttributes";
// static Utf8CP FILEPICKER_INFO_CLASSNAME         = "FilePickerAttributes";
// static Utf8CP STRIKE_THRU_CLASSNAME             = "StrikethroughSpecification";
// static Utf8CP MEMBER_NAME_FROM_VALUE_CLASSNAME  = "ArrayMemberNameFromValue";
static Utf8CP STORES_UNITS_AS_UORS_CLASSNAME    = "StoresUnitsAsUors";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationMetadataHelper::CustomAttributeData
    {
protected:
    Utf8CP         m_name;
    ECValue         m_value;
public:
    CustomAttributeData() : m_name(NULL) { }

    template <typename T_PRIMITIVE>
    CustomAttributeData (Utf8CP name, T_PRIMITIVE value) : m_name(name), m_value(value) { }

    template <Utf8CP>
    CustomAttributeData (Utf8CP name, Utf8CP value) : m_name(name), m_value(value, false) { }

    ECObjectsStatus Apply (IECInstanceR instance) const
        {
        return m_name ? instance.SetValue (m_name, m_value) : ECOBJECTS_STATUS_Success;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationMetadataHelper::PresentationMetadataHelper (ECSchemaReadContextR schemaContext)
    {
    SchemaKey schemaKey (EDITOR_SCHEMA_NAME, 1, 0);
    m_customAttributesSchema = schemaContext.LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
    if (m_customAttributesSchema.IsNull())
        {
        // If we can't find the custom attributes schema, all methods will return ECOBJECTS_STATUS_SchemaNotFound
        BeAssert (false);
        LOG.error (L"Unable to locate EditorCustomAttributes schema");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationMetadataHelper::PresentationMetadataHelper (ECSchemaR editorCustomAttributesSchema)
    {
    SchemaKey schemaKey (EDITOR_SCHEMA_NAME, 1, 0);
    if (editorCustomAttributesSchema.GetSchemaKey().Matches (schemaKey, SCHEMAMATCHTYPE_Latest))
        m_customAttributesSchema = &editorCustomAttributesSchema;
    else
        {
        BeAssert (false);
        LOG.error ("Passed invalid schema, expected EditorCustomAttributes.01.00 or compatible");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationMetadataHelper::~PresentationMetadataHelper()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr PresentationMetadataHelper::CreateInstance (Utf8CP className) const
    {
    IECInstancePtr instance;
    ECClassCP ecClass = m_customAttributesSchema->GetClassCP (className);
    if (NULL == ecClass)
        {
        BeAssert (false);
        LOG.errorv ("Failed to locate ECClass %s in EditorCustomAttributes schema", className);
        }
    else if ((instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance()).IsNull())
        {
        BeAssert (false);
        LOG.errorv ("Failed to create instance of ECClass %s", className);
        }

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::EnsureSchemaReference (IECCustomAttributeContainerR container) const
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_SchemaNotFound;
    if (m_customAttributesSchema.IsValid())
        {
        ECSchemaP containerSchema = container.GetContainerSchema();
        if (NULL != containerSchema)
            {
            if (containerSchema == m_customAttributesSchema.get() || ECSchema::IsSchemaReferenced (*containerSchema, *m_customAttributesSchema))
                status = ECOBJECTS_STATUS_Success;
            else
                status = containerSchema->AddReferencedSchema (*m_customAttributesSchema);
            }
        }

    BeAssert (ECOBJECTS_STATUS_Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::CreateCustomAttribute (IECCustomAttributeContainerR container, Utf8CP className, CustomAttributeData const& data) const
    {
    return CreateCustomAttribute (container, className, &data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::CreateCustomAttribute (IECCustomAttributeContainerR container, Utf8CP className, CustomAttributeData const* data) const
    {
    IECInstancePtr instance;
    ECObjectsStatus status = EnsureSchemaReference (container);
    if (ECOBJECTS_STATUS_Success == status && (instance = CreateInstance (className)).IsValid())
        {
        if (NULL != data)
            status = data->Apply (*instance);

        if (ECOBJECTS_STATUS_Success == status)
            status = container.SetCustomAttribute (*instance);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetExtendedType (ECPropertyR ecproperty, Utf8CP extendTypeName) const
    {
    return CreateCustomAttribute (ecproperty, EXTENDEDTYPE_CLASSNAME, CustomAttributeData ("Name", extendTypeName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetExtendedType (ECPropertyR ecproperty, int32_t standardTypeId) const
    {
    return CreateCustomAttribute (ecproperty, EXTENDEDTYPE_CLASSNAME, CustomAttributeData ("Standard", standardTypeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMemberExtendedType (ArrayECPropertyR ecproperty, Utf8CP extendTypeName) const
    {
    return CreateCustomAttribute (ecproperty, MEMBER_EXTENDEDTYPE_CLASSNAME, CustomAttributeData ("Name", extendTypeName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMemberExtendedType (ArrayECPropertyR ecproperty, int32_t standardTypeId) const
    {
    return CreateCustomAttribute (ecproperty, MEMBER_EXTENDEDTYPE_CLASSNAME, CustomAttributeData ("Standard", standardTypeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetPriority (ECPropertyR ecproperty, int32_t priority) const
    {
    return CreateCustomAttribute (ecproperty, PROPERTY_PRIORITY_CLASSNAME, CustomAttributeData ("Priority", priority));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetIgnoreZ (ECPropertyR ecproperty) const
    {
    return CreateCustomAttribute (ecproperty, IGNOREZ_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetHideNullProperties (ECClassR ecclass) const
    {
    return CreateCustomAttribute (ecclass, DONT_SHOW_NULL_PROPS_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetStoresUnitsAsUORs (ECSchemaR schema) const
    {
    return CreateCustomAttribute (schema, STORES_UNITS_AS_UORS_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMembersIndependent (ECPropertyR ecproperty) const
    {
    if (NULL != ecproperty.GetAsPrimitiveProperty())
        {
        BeAssert (false);
        LOG.error ("MembersIndependent custom attribute not permitted on primitive properties");
        return ECOBJECTS_STATUS_Error;
        }

    return CreateCustomAttribute (ecproperty, MEMBERS_INDEPENDENT_CLASSNAME);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetAlwaysExpand (ECPropertyR ecproperty, bool andArrayMembers) const
    {
    if (NULL != ecproperty.GetAsPrimitiveProperty() && PRIMITIVETYPE_Point3D != ecproperty.GetAsPrimitiveProperty()->GetType())
        {
        BeAssert (false);
        LOG.error ("AlwaysExpand custom attribute only valid for complex properties");
        return ECOBJECTS_STATUS_Error;
        }
    else if (NULL != ecproperty.GetAsArrayProperty() && andArrayMembers)
        return CreateCustomAttribute (ecproperty, ALWAYS_EXPAND_CLASSNAME, CustomAttributeData ("ArrayMembers", true));
    else
        return CreateCustomAttribute (ecproperty, ALWAYS_EXPAND_CLASSNAME); // "ArrayMembers" defaults to false, and ignored for non-array properties
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetRequiresReload (ECPropertyR ecproperty) const
    {
    return CreateCustomAttribute (ecproperty, REQUIRES_RELOAD_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetStandardCategory (ECPropertyR ecproperty, int32_t standardCategoryId) const
    {
    return CreateCustomAttribute (ecproperty, CATEGORY_CLASSNAME, CustomAttributeData ("Standard", standardCategoryId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetCustomCategory (ECPropertyR ecproperty, Utf8CP uniqueName, Utf8CP displayLabel, int32_t priority, bool expand, Utf8CP description) const
    {
    if (NULL == uniqueName || 0 == *uniqueName)
        return ECOBJECTS_STATUS_Error;

    IECInstancePtr attr;
    ECObjectsStatus status = EnsureSchemaReference (ecproperty);
    if (ECOBJECTS_STATUS_Success == status && (attr = CreateInstance (CATEGORY_CLASSNAME)).IsValid())
        {
        attr->SetValue ("Name", ECValue (uniqueName, false));
        attr->SetValue ("DisplayLabel", ECValue (displayLabel, false));
        attr->SetValue ("Description", ECValue (description, false));
        attr->SetValue ("Priority", ECValue (priority));
        attr->SetValue ("Expand", ECValue (expand));

        status = ecproperty.SetCustomAttribute (*attr);
        }

    return status;
    }


