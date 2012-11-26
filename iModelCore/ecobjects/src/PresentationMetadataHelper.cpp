/*--------------------------------------------------------------------------------------+
|
|  $Source: src/PresentationMetadataHelper.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects\PresentationMetadataHelper.h>

USING_NAMESPACE_EC

static WCharCP EDITOR_SCHEMA_NAME                = L"EditorCustomAttributes";
static WCharCP EXTENDEDTYPE_CLASSNAME            = L"ExtendType";
static WCharCP MEMBER_EXTENDEDTYPE_CLASSNAME     = L"MemberExtendedType";
static WCharCP PROPERTY_PRIORITY_CLASSNAME       = L"PropertyPriority";
static WCharCP CLASS_PRIORITY_CLASSNAME          = L"ClassPriority";
static WCharCP CATEGORY_CLASSNAME                = L"Category";
static WCharCP STANDARD_VALUES_CLASSNAME         = L"StandardValues";
static WCharCP BOOLEAN_DISPLAY_CLASSNAME         = L"BooleanDisplay";
static WCharCP BOOLEAN_EXPRESSION_CLASSNAME      = L"UseBooleanInExpressions";
static WCharCP FORMAT_STRING_CLASSNAME           = L"Format";
static WCharCP MEMBERS_INDEPENDENT_CLASSNAME     = L"MembersIndependent";
static WCharCP HIDE_PROPERTY_CLASSNAME           = L"HideProperty";
static WCharCP HIDE_MEMBERS_CLASSNAME            = L"HideMembers";
static WCharCP ALWAYS_EXPAND_CLASSNAME           = L"AlwaysExpand";
static WCharCP DWG_PROPERTYNAME_CLASSNAME        = L"DWGProperty";
static WCharCP DWG_FORMAT_CLASSNAME              = L"DWGFormat";
static WCharCP REQUIRES_RELOAD_CLASSNAME         = L"RequiresReload";
static WCharCP REQUIRES_REFRESH_CLASSNAME        = L"RequiresRefresh";
static WCharCP DONT_SHOW_NULL_PROPS_CLASSNAME    = L"DontShowNullProperties";
static WCharCP IGNOREZ_CLASSNAME                 = L"IgnoreZ";
static WCharCP ARRAY_BEHAVIOR_CLASSNAME          = L"ArrayBehaviorAttributes";
static WCharCP FILEPICKER_INFO_CLASSNAME         = L"FilePickerAttributes";
static WCharCP STRIKE_THRU_CLASSNAME             = L"StrikethroughSpecification";
static WCharCP MEMBER_NAME_FROM_VALUE_CLASSNAME  = L"ArrayMemberNameFromValue";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationMetadataHelper::CustomAttributeData
    {
protected:
    WCharCP         m_name;
    ECValue         m_value;
public:
    CustomAttributeData() : m_name(NULL) { }

    template <typename T_PRIMITIVE>
    CustomAttributeData (WCharCP name, T_PRIMITIVE value) : m_name(name), m_value(value) { }

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
        ECObjectsLogger::Log()->error (L"Unable to locate EditorCustomAttributes schema");
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
IECInstancePtr PresentationMetadataHelper::CreateInstance (WCharCP className) const
    {
    IECInstancePtr instance;
    ECClassCP ecClass = m_customAttributesSchema->GetClassCP (className);
    if (NULL == ecClass)
        {
        BeAssert (false);
        ECObjectsLogger::Log()->errorv (L"Failed to locate ECClass %ls in EditorCustomAttributes schema", className);
        }
    else if ((instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance()).IsNull())
        {
        BeAssert (false);
        ECObjectsLogger::Log()->errorv (L"Failed to create instance of ECClass %ls", className);
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
ECObjectsStatus PresentationMetadataHelper::CreateCustomAttribute (IECCustomAttributeContainerR container, WCharCP className, CustomAttributeData const& data) const
    {
    return CreateCustomAttribute (container, className, &data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::CreateCustomAttribute (IECCustomAttributeContainerR container, WCharCP className, CustomAttributeData const* data) const
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
ECObjectsStatus PresentationMetadataHelper::SetExtendedType (ECPropertyR ecproperty, WCharCP extendTypeName) const
    {
    return CreateCustomAttribute (ecproperty, EXTENDEDTYPE_CLASSNAME, CustomAttributeData (L"Name", extendTypeName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetExtendedType (ECPropertyR ecproperty, Int32 standardTypeId) const
    {
    return CreateCustomAttribute (ecproperty, EXTENDEDTYPE_CLASSNAME, CustomAttributeData (L"Standard", standardTypeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMemberExtendedType (ArrayECPropertyR ecproperty, WCharCP extendTypeName) const
    {
    return CreateCustomAttribute (ecproperty, MEMBER_EXTENDEDTYPE_CLASSNAME, CustomAttributeData (L"Name", extendTypeName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMemberExtendedType (ArrayECPropertyR ecproperty, Int32 standardTypeId) const
    {
    return CreateCustomAttribute (ecproperty, MEMBER_EXTENDEDTYPE_CLASSNAME, CustomAttributeData (L"Standard", standardTypeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetPriority (ECPropertyR ecproperty, Int32 priority) const
    {
    return CreateCustomAttribute (ecproperty, PROPERTY_PRIORITY_CLASSNAME, CustomAttributeData (L"Priority", priority));
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
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PresentationMetadataHelper::SetMembersIndependent (ECPropertyR ecproperty) const
    {
    if (NULL != ecproperty.GetAsPrimitiveProperty())
        {
        BeAssert (false);
        ECObjectsLogger::Log()->error (L"MembersIndependent custom attribute not permitted on primitive properties");
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
        ECObjectsLogger::Log()->error (L"AlwaysExpand custom attribute only valid for complex properties");
        return ECOBJECTS_STATUS_Error;
        }
    else if (NULL != ecproperty.GetAsArrayProperty() && andArrayMembers)
        return CreateCustomAttribute (ecproperty, ALWAYS_EXPAND_CLASSNAME, CustomAttributeData (L"ArrayMembers", true));
    else
        return CreateCustomAttribute (ecproperty, ALWAYS_EXPAND_CLASSNAME); // "ArrayMembers" defaults to false, and ignored for non-array properties
    }


