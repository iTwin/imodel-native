/*--------------------------------------------------------------------------------------+
|
|     $Source: src/KindOfQuantity.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>

#include <ECObjects/StronglyConnectedGraph.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void KindOfQuantity::SetName(Utf8CP name)
    {
    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR KindOfQuantity::GetFullName () const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String KindOfQuantity::GetQualifiedName (ECSchemaCR primarySchema) const
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
Utf8StringCR KindOfQuantity::GetDisplayLabel() const { return m_validatedName.GetDisplayLabel(); }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus KindOfQuantity::WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    if (ecXmlVersionMajor < 3)
        { //will only be serialized in 3.0 and later
        return SchemaWriteStatus::Success;
        }

    Utf8CP elementName = KIND_OF_QUANTITY_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetDescription().c_str());
    auto& displayLabel = GetDisplayLabel();
    if (!displayLabel.empty())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, displayLabel.c_str());

    auto& persistenceUnit = GetPersistenceUnit();
    if(!persistenceUnit.empty())
        xmlWriter.WriteAttribute(PERSISTENCE_UNIT_ATTRIBUTE, persistenceUnit.c_str());

    auto precision = GetPrecision();
    if (precision != 0)
        xmlWriter.WriteAttribute(PRECISION_ATTRIBUTE, precision);

    auto& presentationUnit = GetDefaultPresentationUnit();
    if (!presentationUnit.empty())
        xmlWriter.WriteAttribute(DEFAULT_PRESENTATION_UNIT_ATTRIBUTE, presentationUnit.c_str());

    bvector<Utf8String> const& altPresUnits = GetAlternativePresentationUnitList();
    if (altPresUnits.size() > 0)
        {
        if(altPresUnits.size() == 1)
            xmlWriter.WriteAttribute(ALT_PRESENTATION_UNITS_ATTRIBUTE, altPresUnits[0].c_str());
        else
            {
            Utf8String altPresUnitsJoined = BeStringUtilities::Join(altPresUnits, ";");
            xmlWriter.WriteAttribute(ALT_PRESENTATION_UNITS_ATTRIBUTE, altPresUnitsJoined.c_str());
            }
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
    
    // BACKING_TYPE_NAME_ATTRIBUTE is a required attribute.  If it is missing, an error will be returned.
    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, PERSISTENCE_UNIT_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", kindOfQuantityNode.GetName(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetPersistenceUnit(value.c_str());

    uint32_t precision;
    if (BEXML_Success == kindOfQuantityNode.GetAttributeUInt32Value(precision, PRECISION_ATTRIBUTE))
        {
        SetPrecision(precision);
        }

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, DEFAULT_PRESENTATION_UNIT_ATTRIBUTE))
        {
        SetDefaultPresentationUnit(value.c_str());
        }

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, ALT_PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> altPresUnits;
        BeStringUtilities::Split(value.c_str(), ";", altPresUnits);
        if (altPresUnits.size() >= 1)
            {
            GetAlternativePresentationUnitListR().assign(altPresUnits.begin(), altPresUnits.end());
            }
        }
    return SchemaReadStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE



