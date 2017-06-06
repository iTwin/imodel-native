/*--------------------------------------------------------------------------------------+
|
|     $Source: src/KindOfQuantity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void KindOfQuantity::SetName(Utf8CP name)
    {
    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    }

//TODO: add string representation for FUS once we can call ToText on invalid FUS.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                03/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::Verify() const
    {
    bool isValid = true;
    if (m_persistenceFUS.HasProblem())
        {
        LOG.errorv("Validation Error - KindOfQuantity '%s' persistence FormatUnitSet has a problem: %s",
                   GetFullName().c_str(), m_persistenceFUS.GetProblemDescription().c_str());
        isValid = false;
        }
    
    for (Formatting::FormatUnitSetCR presFUS : m_presentationFUS)
        {
        if (!presFUS.HasProblem())
            continue;

        LOG.errorv("Validation Error - KindOfQuantity '%s' presentation FormatUnitSet has a problem: %s",
                   GetFullName().c_str(), presFUS.GetProblemDescription().c_str());
        isValid = false;
        }

    return isValid;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR KindOfQuantity::GetFullName() const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String KindOfQuantity::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an KindOfQuantity name with an alias unless the schema containing the KindOfQuantity is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  KindOfQuantity: %s\n ECSchema containing KindOfQuantity: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//Following two methods need to be exported as the ValidatedName struct does not export its methods.
void KindOfQuantity::SetDisplayLabel(Utf8CP value) { m_validatedName.SetDisplayLabel(value); }

ECObjectsStatus KindOfQuantity::ParseName(Utf8StringR alias, Utf8StringR kindOfQuantityName, Utf8StringCR stringToParse)
    {
    if (0 == stringToParse.length())
        {
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = stringToParse.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        kindOfQuantityName = stringToParse;
        return ECObjectsStatus::Success;
        }

    if (stringToParse.length() == colonIndex + 1)
        {
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        alias.clear();
    else
        alias = stringToParse.substr(0, colonIndex);

    kindOfQuantityName = stringToParse.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR KindOfQuantity::GetDisplayLabel() const
    {
    return GetSchema().GetLocalizedStrings().GetKindOfQuantityDisplayLabel(*this, GetInvariantDisplayLabel()); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR KindOfQuantity::GetDescription() const 
    { 
    return GetSchema().GetLocalizedStrings().GetKindOfQuantityDescription(*this, m_description); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus KindOfQuantity::WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_0)
        { //will only be serialized in 3.0 and later
        return SchemaWriteStatus::Success;
        }

    Utf8CP elementName = KIND_OF_QUANTITY_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    auto& displayLabel = GetInvariantDisplayLabel();
    if (!displayLabel.empty())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, displayLabel.c_str());

    if (GetPersistenceUnit().HasProblem())
        {
        LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), GetPersistenceUnit().GetProblemDescription().c_str());
        return SchemaWriteStatus::FailedToSaveXml;
        }
    Utf8String persistenceUnitString = GetPersistenceUnit().ToText(false);
    xmlWriter.WriteAttribute(PERSISTENCE_UNIT_ATTRIBUTE, persistenceUnitString.c_str());

    double relError = GetRelativeError();
    xmlWriter.WriteAttribute(RELATIVE_ERROR_ATTRIBUTE, relError);

    bvector<Formatting::FormatUnitSet> const& presentationUnits = GetPresentationUnitList();
    if (presentationUnits.size() > 0)
        {
        Utf8String presentationUnitString;
        bool first = true;
        for(Formatting::FormatUnitSetCR fus : presentationUnits)
            {
            if (fus.HasProblem())
                {
                LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), fus.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToSaveXml;
                }
            if (!first)
                presentationUnitString += ";";
            presentationUnitString += fus.ToText(false);
            first = false;
            }
        xmlWriter.WriteAttribute(PRESENTATION_UNITS_ATTRIBUTE, presentationUnitString.c_str());
        }
    
    //WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus KindOfQuantity::ReadXml(BeXmlNodeR kindOfQuantityNode, ECSchemaReadContextR context)
    {
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        {
        if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, TYPE_NAME_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", TYPE_NAME_ATTRIBUTE, kindOfQuantityNode.GetName());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        SetName(value.c_str());
        }

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, DESCRIPTION_ATTRIBUTE))
        {
        SetDescription(value.c_str());
        }

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, DISPLAY_LABEL_ATTRIBUTE))
        {
        SetDisplayLabel(value.c_str());
        }
    
    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, PERSISTENCE_UNIT_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", kindOfQuantityNode.GetName(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Formatting::FormatUnitSet persistenceFUS(value.c_str());
    if (persistenceFUS.HasProblem())
        LOG.warningv("Persistence FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                     value.c_str(), kindOfQuantityNode.GetName(), persistenceFUS.GetProblemDescription().c_str());
    SetPersistenceUnit(persistenceFUS);

    double relError;
    if (BEXML_Success != kindOfQuantityNode.GetAttributeDoubleValue(relError, RELATIVE_ERROR_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", kindOfQuantityNode.GetName(), RELATIVE_ERROR_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    SetRelativeError(relError);

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presUnit : presentationUnits)
            {
            Formatting::FormatUnitSet presFUS(presUnit.c_str());
            if (presFUS.HasProblem())
                LOG.warningv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                             presUnit.c_str(), kindOfQuantityNode.GetName(), presFUS.GetProblemDescription().c_str());
            m_presentationFUS.push_back(presFUS);
            }
        }
    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Formatting::FormatUnitSetCP KindOfQuantity::GetPresentationFUS(size_t indx) const
    { 
    if (m_presentationFUS.size() > 0)
        {
        return (indx < m_presentationFUS.size())? &m_presentationFUS[indx] :  m_presentationFUS.begin();
        }
    else
        return &m_persistenceFUS;
    };


/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
   /* Formatting::FormatUnitSetCP KindOfQuantityFormatting::GetPresentationFUS(size_t indx) const
    {
    if (m_presentationFUS.size() > 0)
        {
        return (indx < m_presentationFUS.size()) ? &m_presentationFUS[indx] : m_presentationFUS.begin();
        }
    else
        return &m_persistenceFUS;
    };*/

//Formatting::FormatUnitSet KindOfQuantityFormatting::GetDefaultPresentationFUS() const 
//    { 
//    return GetPresentationFUS(0);
//    //return m_presentationFUS.size() > 0 ? *(m_presentationFUS.begin()) : m_persistenceFUS; 
//    };

Json::Value KindOfQuantity::PresentationJson(BEU::QuantityCR qty, size_t indx, bool useAlias) const
    {
    Formatting::FormatUnitSetCP fusCP = GetPresentationFUS(indx);
    Json::Value jval = fusCP->FormatQuantityJson(qty, useAlias);
    return jval;
    }

END_BENTLEY_ECOBJECT_NAMESPACE



