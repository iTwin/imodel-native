/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PropertyCategory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PropertyCategory::SetName(Utf8CP name)
    {
    if (!ECNameValidation::IsValidName(name))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetFullName() const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();

    return m_fullName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String PropertyCategory::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an PropertyCategory name with an alias unless the schema containing the PropertyCategry is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  PropertyCategory: %s\n ECSchema containing PropertyCategory: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PropertyCategory::SetDisplayLabel(Utf8CP displayLabel)
    {
    m_validatedName.SetDisplayLabel(displayLabel);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetDisplayLabel() const
    {
    return GetSchema().GetLocalizedStrings().GetPropertyCategoryDisplayLabel(*this, GetInvariantDisplayLabel()); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetDescription() const
    { 
    return GetSchema().GetLocalizedStrings().GetPropertyCategoryDescription(*this, m_description); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus PropertyCategory::ReadXml(BeXmlNodeR propertyCategoryNode, ECSchemaReadContextR context)
    {
    Utf8String value;
    READ_REQUIRED_XML_ATTRIBUTE(propertyCategoryNode, TYPE_NAME_ATTRIBUTE, this, Name, propertyCategoryNode.GetName())
    READ_OPTIONAL_XML_ATTRIBUTE(propertyCategoryNode, DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(propertyCategoryNode, DESCRIPTION_ATTRIBUTE, this, Description)

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus PropertyCategory::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_1)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = PROPERTY_CATEGORY_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteElementEnd();
    return status;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
