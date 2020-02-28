/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static bvector<Utf8CP> s_dgnV8DeliveredSchemas = {
    "BaseElementSchema",
    "BentleyDesignLinksPersistence",
    "BentleyDesignLinksPresetnation",
    "BentleyDrawingLinksPersistence",
    "DetailSymbolExtender",
    "DgnComponentSchema",
    "DgnContentRelationshipSchema",
    "DgnCustomAttributes",
    "DgnElementSchema",
    "DgnFileSchema",
    "DgnindexQueryschema",
    "DgnLevelSchema",
    "DgnModelSchema",
    "DgnPointCloudSchema",
    "DgnTextStyleObjSchema",
    "DgnVisualizationObjSchema",
    "ExtendedElementSchema",
    "MstnPropertyFormatter"
    };

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

// =====================================================================================
// SchemaXmlReaderImpl class
// =====================================================================================

//---------------------------------------------------------------------------------------
// - OpenPlant shipped a malformed schema that has a circular reference through supplementation.
// - Therefore a special case had to be created so that we do not try to de-serialize this
// - schema
// @bsimethod                                    Carole.MacDonald                01/2012
//---------------+---------------+---------------+---------------+---------------+-------
bool  SchemaXmlReaderImpl::IsOpenPlantPidCircularReferenceSpecialCase
    (
    Utf8String& referencedECSchemaName,
    Utf8String& referencingECSchemaFullName
    )
    {
    if (0 != referencedECSchemaName.CompareTo("OpenPlant_PID"))
        return false;

    return (0 == referencingECSchemaFullName.CompareTo("OpenPlant_Supplemental_Mapping_OPPID.01.00.01") || 0 == referencingECSchemaFullName.CompareTo("OpenPlant_Supplemental_Mapping_OPPID.01.00.02"));
    }

//---------------------------------------------------------------------------------------
// The DgnV8Converter ignores any Dgn system delivered schemas.  Unfortunately, we occasionally come across a
// domain schema that references one of these schemas and uses their classes as a base class.
// These schemas have absolutely no meaning in the bis world, so it is fine to bring the domain schema
// in without those references.
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReaderImpl::IsDgnV8DeliveredSchema(Utf8CP referencedECSchemaName)
    {
    auto found = std::find_if(s_dgnV8DeliveredSchemas.begin(), s_dgnV8DeliveredSchemas.end(), [referencedECSchemaName] (Utf8CP dgnv8) ->bool { return BeStringUtilities::StricmpAscii(referencedECSchemaName, dgnv8) == 0; });
    return found != s_dgnV8DeliveredSchemas.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::_ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    BeXmlDom::IterableNodeSet schemaReferenceNodes;
    schemaNode.SelectChildNodes(schemaReferenceNodes, EC_NAMESPACE_PREFIX ":" ECXML_SCHEMAREFERENCE_ELEMENT);
    for (BeXmlNodeP& schemaReferenceNode : schemaReferenceNodes)
        {
        SchemaKey key;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(key.m_schemaName, NAME_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), NAME_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        Utf8String alias;
        if (schemaOut->OriginalECXmlVersionAtLeast(ECVersion::V3_1))
            {
            if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(alias, ALIAS_ATTRIBUTE))
                {
                LOG.errorv("Invalid ECSchemaXML: %s element must contain an %s attribute", schemaReferenceNode->GetName(), ALIAS_ATTRIBUTE);
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            }
        else
            {
            if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(alias, SCHEMAREF_PREFIX_ATTRIBUTE))
                {
                LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_PREFIX_ATTRIBUTE);
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            }

        Utf8String versionString;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(versionString, SCHEMAREF_VERSION_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_VERSION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        {
        ECObjectsStatus versionStatus = ECObjectsStatus::Success;
        if (schemaOut->OriginalECXmlVersionGreaterThan(ECVersion::V3_1))
            versionStatus = SchemaKey::ParseVersionStringStrict(key.m_versionRead, key.m_versionWrite, key.m_versionMinor, versionString.c_str());
        else
            versionStatus = SchemaKey::ParseVersionString(key.m_versionRead, key.m_versionWrite, key.m_versionMinor, versionString.c_str());

        if (ECObjectsStatus::Success != versionStatus)
            {
            LOG.errorv("Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", key.m_schemaName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

        // If the schema (uselessly) references itself, just skip it
        if (schemaOut->GetSchemaKey().m_schemaName.compare(key.m_schemaName) == 0)
            continue;

        if (schemaOut->OriginalECXmlVersionLessThan(ECVersion::V3_0) && IsDgnV8DeliveredSchema(key.GetName().c_str()))
            {
            m_droppedPrefixes.push_back(alias);
            continue;
            }

        Utf8String schemaFullName = schemaOut->GetFullSchemaName();
        if (IsOpenPlantPidCircularReferenceSpecialCase(key.m_schemaName, schemaFullName))
            continue;

        LOG.debugv("About to locate referenced ECSchema %s", key.GetFullSchemaName().c_str());

        // There are some schemas out there that reference the non-existent Unit_Attributes.1.1 schema.  We need to deliver 1.0, which does not match our criteria
        // for LatestCompatible.
        if (0 == key.GetName().CompareTo("Unit_Attributes") && 1 == key.GetVersionRead() && 1 == key.GetVersionMinor())
            key.m_versionMinor = 0;
        ECSchemaPtr referencedSchema = schemaOut->LocateSchema(key, m_schemaContext);

        if (referencedSchema.IsValid())
            {
            //We can encounter some time same schema referenced twice with different alias.
            //We will not treat it as error.
            SchemaKeyCR refSchemaKey = referencedSchema->GetSchemaKey();
            auto const& references = schemaOut->GetReferencedSchemas();
            if (references.end() != references.find(refSchemaKey))
                {
                continue;
                }

            ECObjectsStatus status = schemaOut->AddReferencedSchema(*referencedSchema, alias, m_schemaContext);
            if (ECObjectsStatus::Success != status)
                return ECObjectsStatus::SchemaHasReferenceCycle == status ? SchemaReadStatus::HasReferenceCycle : static_cast<SchemaReadStatus> (status);
            }
        else
            {
            LOG.errorv("Unable to locate referenced schema %s", key.GetFullSchemaName().c_str());
            return SchemaReadStatus::ReferencedSchemaNotFound;
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes, int ecXmlVersionMajor)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;
    bool resolveConflicts = false;
    if (2 == ecXmlVersionMajor)
        resolveConflicts = true;
    else if (m_conversionSchema.IsValid())
        resolveConflicts = m_conversionSchema->IsDefined("ResolveClassNameConflicts");

    bvector<Utf8String> comments;
    // Create ECClass Stubs (no properties)
    for (BeXmlNodeP classNode = schemaNode.GetFirstChild(BEXMLNODE_Any); nullptr != classNode; classNode = classNode->GetNextSibling(BEXMLNODE_Any))
        {
        if (m_schemaContext.GetPreserveXmlComments())
            {
            if ((BeXmlNodeType)classNode->type == BEXMLNODE_Comment)
                {
                Utf8String comment;
                if (classNode->GetContent(comment) == BeXmlStatus::BEXML_Success)
                    {
                    comments.push_back(comment);
                    }
                }
            }
        if ((BeXmlNodeType)classNode->type != BEXMLNODE_Element)
            continue;

        ECClassP ecClass = nullptr;
        if (!ReadClassNode(ecClass, *classNode, schemaOut))
            {
            // The comments read so far belong to the current element, but it's not a class, therefore we need to drop them
            // No need to check if PreserveXmlComments is true because the vector is empty and not used anyway
            comments.clear();
            continue;
            }

        if (m_schemaContext.GetPreserveXmlComments())
            {
            ecClass->m_xmlComments = comments;
            comments.clear();
            }

        if (SchemaReadStatus::Success != (status = ecClass->_ReadXmlAttributes(*classNode)))
            {
            delete ecClass;
            return status;
            }

        if (ecClass->IsStructClass())
            LOG.tracev("    Created ECStructClass Stub: %s", ecClass->GetName().c_str());
        else if (ecClass->IsCustomAttributeClass())
            LOG.tracev("    Created ECCustomAttributeClass Stub: %s", ecClass->GetName().c_str());
        else if (ecClass->IsRelationshipClass())
            LOG.tracev("    Created Relationship ECClass Stub: %s", ecClass->GetName().c_str());
        else
            LOG.tracev("    Created ECEntityClass Stub: %s", ecClass->GetName().c_str());

        ECObjectsStatus addStatus = schemaOut->AddClass(ecClass, resolveConflicts);

        if (addStatus == ECObjectsStatus::NamedItemAlreadyExists)
            {
            LOG.errorv("Duplicate class node for %s in schema %s.", ecClass->GetName().c_str(), schemaOut->GetFullSchemaName().c_str());
            delete ecClass;
            ecClass = nullptr;
            return SchemaReadStatus::DuplicateTypeName;
            }

        if (ECObjectsStatus::Success != addStatus)
            {
            delete ecClass;
            ecClass = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        classes.push_back(make_bpair(ecClass, classNode));
        }

    return status;
    }


//---------------------------------------------------------------------------------------
// - Expects class stubs have already been read and created.  They are stored in the vector passed into this method.
// - Expects referenced schemas have been resolved and read so that base classes & structs in other schemas can be located.
// - Reads the contents of each XML node cached in the classes vector and populates the in-memory EC:ECClass with
//   base classes, properties & relationship endpoints.
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::_ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    bvector<NavigationECPropertyP> navigationProperties;
    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP    ecClass;
    BeXmlNodeP  classNode;
    ECSchemaPtr conversionSchema = m_schemaContext.LocateConversionSchemaFor(schemaOut->GetName().c_str(), schemaOut->GetVersionRead(), schemaOut->GetVersionMinor());

    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass = classesIterator->first;
        classNode = classesIterator->second;
        status = ecClass->_ReadXmlContents(*classNode, m_schemaContext, conversionSchema.get(), m_droppedPrefixes, navigationProperties);
        if (SchemaReadStatus::Success != status)
            {
            LOG.errorv("Failed to read class '%s' from schema '%s'", ecClass->GetName().c_str(), schemaOut->GetName().c_str());
            return status;
            }
        }

    for (auto const& navProp : navigationProperties)
        if (!navProp->Verify())
            {
            LOG.errorv("Unable to load NavigationECProperty '%s:%s.%s' because the relationship '%s' does not support this class as a constraint when traversed in the '%s' direction or max multiplicity is greater than 1.",
                        navProp->GetClass().GetSchema().GetName().c_str(), navProp->GetClass().GetName().c_str(), navProp->GetName().c_str(),
                        navProp->GetRelationshipClass()->GetName().c_str(), SchemaParseUtils::DirectionToXmlString(navProp->GetDirection()));

            return SchemaReadStatus::InvalidECSchemaXml;
            }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                     01/2018
//---------------------------------------------------------------------------------------
template<typename T>
SchemaReadStatus SchemaXmlReaderImpl::ReadSchemaChildFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ECSchemaElementType childType)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        if (!IsSchemaChildElementNode(*candidateNode, childType))
            continue;

        T* schemaChild = new T(*schemaOut);
        status = schemaChild->ReadXml(*candidateNode, m_schemaContext);
        if (SchemaReadStatus::Success != status)
            {
            delete schemaChild;
            return status;
            }

        ECObjectsStatus addStatus = schemaOut->AddSchemaChild<T>(schemaChild, childType);
        if (ECObjectsStatus::NamedItemAlreadyExists == addStatus)
            {
            LOG.errorv("Duplicate %s node for %s in schema %s.", SchemaParseUtils::SchemaElementTypeToString(childType), schemaChild->GetName().c_str(), schemaOut->GetFullSchemaName().c_str());
            delete schemaChild;
            schemaChild = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (ECObjectsStatus::Success != addStatus)
            {
            delete schemaChild;
            schemaChild = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------------------------------------------------------------------------------
template<typename T>
SchemaReadStatus SchemaXmlReaderImpl::ReadUnitTypeFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ECSchemaElementType unitType)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        if (!IsSchemaChildElementNode(*candidateNode, unitType))
            continue;

        // All Schema Children are required to have a typeName.
        Utf8String name;
        if (BEXML_Success != candidateNode->GetAttributeStringValue(name, TYPE_NAME_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", candidateNode->GetName(), TYPE_NAME_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (!ECNameValidation::IsValidName(name.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXml: The %s element must contain a valid EC %s", candidateNode->GetName(), TYPE_NAME_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        T* unitChild = new T(*schemaOut, name.c_str());
        status = unitChild->ReadXml(*candidateNode, m_schemaContext);
        if (SchemaReadStatus::Success != status)
            {
            delete unitChild;
            return status;
            }

        ECObjectsStatus addStatus = schemaOut->AddSchemaChild<T>(unitChild, unitType);
        if (ECObjectsStatus::NamedItemAlreadyExists == addStatus)
            {
            LOG.errorv("Duplicate %s node for %s in schema %s.", SchemaParseUtils::SchemaElementTypeToString(unitType), unitChild->GetName().c_str(), schemaOut->GetFullSchemaName().c_str());
            delete unitChild;
            unitChild = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (ECObjectsStatus::Success != addStatus)
            {
            delete unitChild;
            unitChild = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    return status;
    }

// =====================================================================================
// SchemaXmlReader2 class
// =====================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader2::ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut)
    {
    Utf8CP nodeName = classNode.GetName();

    Utf8String     className;
    classNode.GetAttributeStringValue(className, TYPE_NAME_ATTRIBUTE);

    if (DropClassAttributeDefined(className))
        return false;

    Utf8String boolStr;
    bool isDomain = true; // defaults to true
    if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_DOMAINCLASS_ATTRIBUTE))
        SchemaParseUtils::ParseBooleanXmlString(isDomain, boolStr.c_str());

    if (0 == strcmp(nodeName, EC_CLASS_ELEMENT))
        {
        // Need to determine what type of class this actually is in EC 3.0
        bool isCA = false;
        bool isStruct = false;
        bool isSealed = false;
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_CUSTOMATTRIBUTE_ATTRIBUTE))
            SchemaParseUtils::ParseBooleanXmlString(isCA, boolStr.c_str());
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_STRUCT_ATTRIBUTE))
            SchemaParseUtils::ParseBooleanXmlString(isStruct, boolStr.c_str());
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_FINAL_ATTRIBUTE))
            SchemaParseUtils::ParseBooleanXmlString(isSealed, boolStr.c_str());

        ECClassType classType;
        ECClassModifier modifier;
        DetermineClassTypeAndModifier(className, schemaOut, classType, modifier, isCA, isStruct, isDomain, isSealed);

        if (ECClassType::Entity == classType)
            ecClass = CreateEntityClass(schemaOut);
        else if (ECClassType::CustomAttribute == classType)
            ecClass = CreateCustomAttributeClass(schemaOut);
        else
            ecClass = CreateStructClass(schemaOut);

        ecClass->SetClassModifier(modifier);
        }
    else if (0 == strcmp(nodeName, ECXML_RELATIONSHIP_CLASS_ELEMENT))
        {
        ecClass = CreateRelationshipClass(schemaOut);
        ecClass->SetClassModifier(DetermineRelationshipClassModifier(className, isDomain));
        }
    else
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      01/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader2::DropClassAttributeDefined(Utf8StringCR className) const
    {
    if (!m_conversionSchema.IsValid())
        return false;

    ECClassCP ecClass = m_conversionSchema->GetClassCP(className.c_str());
    if (nullptr == ecClass)
        return false;

    return ecClass->IsDefined("DropClass");
    }

void WriteLogMessage(ECClassCP ecClass, Utf8CP classDescription, Utf8CP forcedType)
    {
    LOG.debugv("Forcing %s '%s:%s' to be %s because the 'ECv3ConversionAttributes:Force%s' custom attribute is applied to the %s",
               classDescription, ecClass->GetSchema().GetFullSchemaName().c_str(), ecClass->GetName().c_str(), forcedType, forcedType, classDescription);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      01/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECClassModifier SchemaXmlReader2::DetermineRelationshipClassModifier(Utf8StringCR className, bool isDomain) const
    {
    ECClassModifier modifier = ECClassModifier::Abstract;

    // Needs to determine something different than ECClassModifier::None. None is not a supported modifier for relationships in ECXml 3.1
    if (isDomain)
        modifier = ECClassModifier::None;

    if (!m_conversionSchema.IsValid())
        return modifier;

    ECClassCP ecClass = m_conversionSchema->GetClassCP(className.c_str());
    if (nullptr == ecClass)
        return modifier;

    if (ecClass->IsDefined("ForceAbstract"))
        {
        WriteLogMessage(ecClass, "relationship class", "Abstract");
        modifier = ECClassModifier::Abstract;
        }
    else if (ecClass->IsDefined("ForceSealed"))
        {
        WriteLogMessage(ecClass, "relationship class", "Sealed");
        modifier = ECClassModifier::Sealed;
        }
    else if (ecClass->IsDefined("ForceInstantiable"))
        {
        WriteLogMessage(ecClass, "relationship class", "None");
        modifier = ECClassModifier::None;
        }

    return modifier;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      01/2016
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaXmlReader2::DetermineClassTypeAndModifier(Utf8StringCR className, ECSchemaPtr schemaOut, ECClassType& classType, ECClassModifier& classModifier, bool isCA, bool isStruct, bool isDomain, bool isSealed) const
    {
    if (isStruct)
        classType = ECClassType::Struct;
    else if (isCA)
        classType = ECClassType::CustomAttribute;
    else
        classType = ECClassType::Entity;

    int sum = (int)isCA + (int)isStruct + (int)isDomain;
    if (0 == sum)
        classModifier = ECClassModifier::Abstract;
    else if (isSealed)
        classModifier = ECClassModifier::Sealed;
    else
        classModifier = ECClassModifier::None;

    if (!m_conversionSchema.IsValid())
        {
        if (1 < sum)
            {
            // We have a dgn (at least one) from a client that has thousands of schemas with classes that are declared as structs but are instantiated.  There are only 5 unique class names but over 1200
            // schema names, so we just hardcode in those classnames since they are unique
            if (className.Equals("EWR_GENERAL") || className.Equals("EWR_LINESIDE_ASSET") || className.Equals("EWR_DOCUMENT_REFERENCE") ||
                className.Equals("EWR_INFORMATION_BLOCK") || className.Equals("EWR_UTILITY"))
                {
                classType = ECClassType::Entity;
                LOG.infov("Class '%s' in schema '%s' has more than one type flag set to true: isStruct(%d) isDomainClass(%d) isCustomAttributeClass(%d).  Only one is allowed, forcing to Domain.  ",
                             className.c_str(), schemaOut->GetFullSchemaName().c_str(), isStruct, isDomain, isCA);
                }
            else
                {
                LOG.warningv("Class '%s' in schema '%s' has more than one type flag set to true: isStruct(%d) isDomainClass(%d) isCustomAttributeClass(%d).  Only one is allowed, defaulting to %s.  "
                             "Modify the schema or use the ECv3ConversionAttributes in a conversion schema named '%s'_V3Conversion to force a different class type.",
                             className.c_str(), schemaOut->GetFullSchemaName().c_str(), isStruct, isDomain, isCA, isStruct ? "Struct" : "CustomAttribute",
                             schemaOut->GetFullSchemaName().c_str());
                }
            }
        return;
        }

    ECClassCP ecClass = m_conversionSchema->GetClassCP(className.c_str());
    if (nullptr == ecClass)
        return;

    if (ecClass->IsDefined("ForceEntityClass"))
        {
        WriteLogMessage(ecClass, "ECClass", "EntityClass");
        classType = ECClassType::Entity;
        }
    else if (ecClass->IsDefined("ForceCustomAttributeClass"))
        {
        WriteLogMessage(ecClass, "ECClass", "CustomAttributeClass");
        classType = ECClassType::CustomAttribute;
        }
    else if (ecClass->IsDefined("ForceStructClass"))
        {
        WriteLogMessage(ecClass, "ECClass", "StructClass");
        classType = ECClassType::Struct;
        }
    else if (1 < sum)
        LOG.warningv("Class '%s' in schema '%s' has more than one type flag set to true: isStruct(%d) isDomainClass(%d) isCustomAttributeClass(%d).  Only one is allowed, defaulting to %s.  "
                     "Modify the schema or use the ECv3ConversionAttributes in a conversion schema named '%s'_V3Conversion to force a different class type.",
                     className.c_str(), schemaOut->GetFullSchemaName().c_str(), isStruct, isDomain, isCA, isStruct ? "Struct" : "CustomAttribute",
                     schemaOut->GetFullSchemaName().c_str());


    if (ecClass->IsDefined("ForceAbstract"))
        {
        WriteLogMessage(ecClass, "ECClass", "Abstract");
        classModifier = ECClassModifier::Abstract;
        }
    else if (ecClass->IsDefined("ForceSealed"))
        {
        WriteLogMessage(ecClass, "ECClass", "Sealed");
        classModifier = ECClassModifier::Sealed;
        }
    }

// =====================================================================================
// SchemaXmlReader3 class
// =====================================================================================

//---------------------------------------------------------------------------------------
// Create ECClass Stubs (no attributes or properties)
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader3::ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut)
    {
    Utf8CP nodeName = classNode.GetName();
    if (0 == strcmp(ECXML_ENTITYCLASS_ELEMENT, nodeName))
        ecClass = CreateEntityClass(schemaOut);
    else if (0 == strcmp(ECXML_STRUCTCLASS_ELEMENT, nodeName))
        ecClass = CreateStructClass(schemaOut);
    else if (0 == strcmp(ECXML_CUSTOMATTRIBUTECLASS_ELEMENT, nodeName))
        ecClass = CreateCustomAttributeClass(schemaOut);
    else if (0 == strcmp(ECXML_RELATIONSHIP_CLASS_ELEMENT, nodeName))
        ecClass = CreateRelationshipClass(schemaOut);

    if (nullptr == ecClass)
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
bool SchemaXmlReaderImpl::_IsSchemaChildElementNode(BeXmlNodeR schemaNode, ECSchemaElementType childType, ECVersion xmlVersion)
    {
    Utf8CP nodeName = schemaNode.GetName();
    switch (childType)
        {
        case ECSchemaElementType::ECClass:
            if (ECVersion::V2_0 == xmlVersion)
                return 0 == strcmp(EC_CLASS_ELEMENT, nodeName) || 0 == strcmp(ECXML_RELATIONSHIP_CLASS_ELEMENT, nodeName);

            return 0 == strcmp(ECXML_STRUCTCLASS_ELEMENT, nodeName) ||
                0 == strcmp(ECXML_CUSTOMATTRIBUTECLASS_ELEMENT, nodeName) ||
                0 == strcmp(ECXML_ENTITYCLASS_ELEMENT, nodeName) ||
                0 == strcmp(ECXML_RELATIONSHIP_CLASS_ELEMENT, nodeName);
        case ECSchemaElementType::ECEnumeration:
            return 0 == strcmp(ECXML_ENUMERATION_ELEMENT, nodeName);
        case ECSchemaElementType::KindOfQuantity:
            return 0 == strcmp(KIND_OF_QUANTITY_ELEMENT, nodeName);
        case ECSchemaElementType::PropertyCategory:
            return 0 == strcmp(PROPERTY_CATEGORY_ELEMENT, nodeName);
        case ECSchemaElementType::UnitSystem:
            return 0 == strcmp(UNIT_SYSTEM_ELEMENT, nodeName);
        case ECSchemaElementType::Phenomenon:
            return 0 == strcmp(PHENOMENON_ELEMENT, nodeName);
        case ECSchemaElementType::Unit:
            return 0 == strcmp(UNIT_ELEMENT, nodeName);
        case ECSchemaElementType::InvertedUnit:
            return 0 == strcmp(INVERTED_UNIT_ELEMENT, nodeName);
        case ECSchemaElementType::Constant:
            return 0 == strcmp(CONSTANT_ELEMENT, nodeName);
        case ECSchemaElementType::Format:
            return 0 == strcmp(FORMAT_ELEMENT, nodeName);
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
 void SchemaXmlReaderImpl::PopulateSchemaElementOrder(ECSchemaElementsOrder& elementOrder, BeXmlNodeR schemaNode)
    {
    elementOrder.SetPreserveElementOrder(true);
    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        Utf8String typeName;
        // If the node does not have a typeName, it can't be an ECSchema Child Element
        if (BEXML_Success != candidateNode->GetAttributeStringValue(typeName, TYPE_NAME_ATTRIBUTE))
            continue;

        if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::ECClass))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::ECClass);
        else if(IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::ECEnumeration))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::ECEnumeration);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::KindOfQuantity))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::KindOfQuantity);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::PropertyCategory))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::PropertyCategory);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::UnitSystem))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::UnitSystem);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::Phenomenon))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::Phenomenon);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::Unit))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::Unit);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::InvertedUnit))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::InvertedUnit);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::Constant))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::Constant);
        else if (IsSchemaChildElementNode(*candidateNode, ECSchemaElementType::Format))
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::Format);
        }
    }

// =====================================================================================
// SchemaXmlReader class
// =====================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaXmlReader::SetErrorHandling(bool doAssert)
    {
    s_noAssert = !doAssert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                02/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
SchemaReadStatus SchemaXmlReader::ReadSchemaStub(SchemaKey& schemaKey, uint32_t& ecXmlMajorVersion, uint32_t& ecXmlMinorVersion, BeXmlNodeP& schemaNode, BeXmlDomR xmlDom)
    {
    BeXmlNodeP rootNode = static_cast <BeXmlNodeP>(xmlDocGetRootElement(&(xmlDom.GetDocument())));
    if (nullptr == rootNode)
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: Missing a top-level node");
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String schemaNamespace(rootNode->GetNamespace());

    if (!schemaNamespace.StartsWith(ECXML_URI))
        {
        LOG.errorv("Unknown schema namespace: %s", schemaNamespace.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String version = schemaNamespace.substr(strlen(ECXML_URI) + 1);
    Utf8String::Sscanf_safe(version.c_str(), "%d.%d", &ecXmlMajorVersion, &ecXmlMinorVersion);
    if (2 != ecXmlMajorVersion && 3 != ecXmlMajorVersion)
        {
        LOG.errorv("Unsupported ecXml version %d.%d", ecXmlMajorVersion, ecXmlMinorVersion);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    xmlDom.RegisterNamespace(EC_NAMESPACE_PREFIX, schemaNamespace.c_str());

    if ((BEXML_Success != xmlDom.SelectNode(schemaNode, "/" EC_NAMESPACE_PREFIX ":" EC_SCHEMA_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == schemaNode))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: Missing a top-level %s node in the %s namespace", EC_SCHEMA_ELEMENT, schemaNamespace.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    // schemaName is a REQUIRED attribute in order to create the schema
    Utf8String schemaName;
    if (BEXML_Success != schemaNode->GetAttributeStringValue(schemaName, ECXML_SCHEMA_NAME_ATTRIBUTE))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a schemaName attribute", EC_SCHEMA_ELEMENT);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    uint32_t versionRead = DEFAULT_VERSION_READ;
    uint32_t versionWrite = DEFAULT_VERSION_WRITE;
    uint32_t versionMinor = DEFAULT_VERSION_MINOR;

    if ((ecXmlMajorVersion >= 3 && ecXmlMinorVersion >= 2) || ecXmlMajorVersion > 3)
        {
        Utf8String versionString;
        if ((BEXML_Success != schemaNode->GetAttributeStringValue(versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
            (ECObjectsStatus::Success != SchemaKey::ParseVersionStringStrict(versionRead, versionWrite, versionMinor, versionString.c_str())))
            {
            LOG.errorv("Invalid version attribute %s while reading ECSchema '%s'.", versionString.c_str(), schemaName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    else
        {
        // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
        // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
        Utf8String     versionString;
        if ((BEXML_Success != schemaNode->GetAttributeStringValue(versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
            (ECObjectsStatus::Success != SchemaKey::ParseVersionString(versionRead, versionWrite, versionMinor, versionString.c_str())))
            {
            LOG.warningv("Invalid version attribute has been ignored while reading ECSchema '%s'.  The default version number %s has been applied.",
                         schemaName.c_str(), SchemaKey::FormatSchemaVersion(versionRead, versionWrite, versionMinor).c_str());
            }
        }

    schemaKey = SchemaKey(schemaName.c_str(), versionRead, versionWrite, versionMinor);

    LOG.debugv("Reading ECSchema %s", schemaKey.GetFullSchemaName().c_str());

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader::Deserialize(ECSchemaPtr& schemaOut, Utf8CP checksum)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;
    StopWatch overallTimer("Overall schema de-serialization timer", true);

    BeXmlNodeP schemaNode;
    SchemaKey schemaKey;
    uint32_t ecXmlMajorVersion, ecXmlMinorVersion;
    status = ReadSchemaStub(schemaKey, ecXmlMajorVersion, ecXmlMinorVersion, schemaNode, m_xmlDom);
    if (SchemaReadStatus::Success != status)
        return status;

    Utf8String alias;
    // Alias is a required attribute for EC3.1. If it is missing from <= EC3.0 schemas it is set to the schemaName
    if ((ecXmlMajorVersion == 3 && ecXmlMinorVersion >= 1) || ecXmlMajorVersion > 3)
        {
        if (BEXML_Success != schemaNode->GetAttributeStringValue(alias, ALIAS_ATTRIBUTE) || Utf8String::IsNullOrEmpty(alias.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain an alias attribute", EC_SCHEMA_ELEMENT);
            BeAssert(s_noAssert);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    else
        {
        if (BEXML_Success != schemaNode->GetAttributeStringValue(alias, SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE) || Utf8String::IsNullOrEmpty(alias.c_str()))
            alias = schemaKey.GetName();
        }

    ECObjectsStatus createStatus = ECSchema::CreateSchema(schemaOut, schemaKey.GetName(), alias, schemaKey.GetVersionRead(), schemaKey.GetVersionWrite(), schemaKey.GetVersionMinor());
    if (ECObjectsStatus::Success != createStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    schemaOut->m_originalECXmlVersionMajor = ecXmlMajorVersion;
    schemaOut->m_originalECXmlVersionMinor = ecXmlMinorVersion;

    // If checksum comparison is enabled on context then use the original context
    if (m_schemaContext.GetCalculateChecksum() && nullptr != checksum)
        schemaOut->m_key.m_checksum = checksum;

    // Handle conversion of encoded ECName to name + display label for legacy schemas
    if (schemaOut->OriginalECXmlVersionLessThan(ECVersion::V3_1))
        schemaOut->SetName(schemaKey.GetName().c_str());

    if (ECObjectsStatus::DuplicateSchema == m_schemaContext.AddSchema(*schemaOut))
        return SchemaReadStatus::DuplicateSchema;

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    Utf8String value;  // used by macro.
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), DESCRIPTION_ATTRIBUTE, schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), ECXML_DISPLAY_LABEL_ATTRIBUTE, schemaOut, DisplayLabel)

    StopWatch readingSchemaReferences("Reading Schema References", true);
    SchemaXmlReaderImpl* reader = nullptr;
    if (2 == ecXmlMajorVersion)
        reader = new SchemaXmlReader2(m_schemaContext, schemaOut, m_xmlDom);
    else
        reader = new SchemaXmlReader3(m_schemaContext, m_xmlDom);

    if (SchemaReadStatus::Success != (status = reader->ReadSchemaReferencesFromXml(schemaOut, *schemaNode)))
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingSchemaReferences.Stop();
    LOG.tracev("Reading schema references for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingSchemaReferences.GetElapsedSeconds());

    // Class Stubs
    ClassDeserializationVector classes;
    StopWatch readingClassStubs("Reading class stubs", true);
    status = reader->ReadClassStubsFromXml(schemaOut, *schemaNode, classes, ecXmlMajorVersion);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingClassStubs.Stop();
    LOG.tracev("Reading class stubs for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassStubs.GetElapsedSeconds());

    // Enumerations
    StopWatch readingEnumerations("Reading enumerations", true);
    status = reader->ReadSchemaChildFromXml<ECEnumeration>(schemaOut, *schemaNode, ECSchemaElementType::ECEnumeration);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingEnumerations.Stop();
    LOG.tracev("Reading enumerations for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingEnumerations.GetElapsedSeconds());

    // PropertyCategory
    StopWatch readingPropertyCategories("Reading property category", true);
    status = reader->ReadSchemaChildFromXml<PropertyCategory>(schemaOut, *schemaNode, ECSchemaElementType::PropertyCategory);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingPropertyCategories.Stop();
    LOG.tracev("Reading property category elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingPropertyCategories.GetElapsedSeconds());

    // UnitSystems
    StopWatch readingUnitSystems("Reading unit systems", true);
    status = reader->ReadUnitTypeFromXml<UnitSystem>(schemaOut, *schemaNode, ECSchemaElementType::UnitSystem);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingUnitSystems.Stop();
    LOG.tracev("Reading unit system elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingUnitSystems.GetElapsedSeconds());

    // Phenomena
    StopWatch readingPhenomena("Reading Phenomena", true);
    status = reader->ReadUnitTypeFromXml<Phenomenon>(schemaOut, *schemaNode, ECSchemaElementType::Phenomenon);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingPhenomena.Stop();
    LOG.tracev("Reading Phenomenon elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingPhenomena.GetElapsedSeconds());

    // ECUnits
    StopWatch readingUnits("Reading Units", true);
    status = reader->ReadUnitTypeFromXml<ECUnit>(schemaOut, *schemaNode, ECSchemaElementType::Unit);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingUnits.Stop();
    LOG.tracev("Reading Unit elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingUnits.GetElapsedSeconds());

    // Inverted Units
    StopWatch readingInvertedUnit("Reading InvertedUnits", true);
    status = reader->ReadUnitTypeFromXml<ECUnit>(schemaOut, *schemaNode, ECSchemaElementType::InvertedUnit);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingInvertedUnit.Stop();
    LOG.tracev("Reading InvertedUnit elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingInvertedUnit.GetElapsedSeconds());

    // Constants
    StopWatch readConstants("Reading constants", true);
    status = reader->ReadUnitTypeFromXml<ECUnit>(schemaOut, *schemaNode, ECSchemaElementType::Constant);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readConstants.Stop();
    LOG.tracev("Reading Constant elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readConstants.GetElapsedSeconds());

    // Format
    StopWatch readingUnitFormats("Reading Formats", true);
    status = reader->ReadUnitTypeFromXml<ECFormat>(schemaOut, *schemaNode, ECSchemaElementType::Format);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingUnitFormats.Stop();
    LOG.tracev("Reading Formats for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingUnitFormats.GetElapsedSeconds());

    // KindOfQuantity
    StopWatch readingKindOfQuantities("Reading kind of quantity", true);
    status = reader->ReadSchemaChildFromXml<KindOfQuantity>(schemaOut, *schemaNode, ECSchemaElementType::KindOfQuantity);

    if (SchemaReadStatus::Success != status)
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingKindOfQuantities.Stop();
    LOG.tracev("Reading kind of quantity elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingKindOfQuantities.GetElapsedSeconds());

    // Class Contents
    StopWatch readingClassContents("Reading class contents", true);
    if (SchemaReadStatus::Success != (status = reader->ReadClassContentsFromXml(schemaOut, classes)))
        {
        delete reader; reader = nullptr;
        return status;
        }

    readingClassContents.Stop();
    LOG.tracev("Reading class contents for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassContents.GetElapsedSeconds());

    // Schema-level Custom Attributes
    StopWatch readingCustomAttributes("Reading custom attributes", true);
    if (CustomAttributeReadStatus::InvalidCustomAttributes == schemaOut->ReadCustomAttributes(*schemaNode, m_schemaContext, *schemaOut))
        {
        LOG.errorv("Failed to read schema because one or more invalid custom attributes were applied to it.", schemaOut->GetName().c_str());
        delete reader; reader = nullptr;
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    readingCustomAttributes.Stop();
    LOG.tracev("Reading custom attributes for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingCustomAttributes.GetElapsedSeconds());

    // If switch is set in reading context, the class order of the schema xml will be preserved that it can be written out in the same order.
    if (m_schemaContext.GetPreserveElementOrder())
        reader->PopulateSchemaElementOrder(schemaOut->m_serializationOrder, *schemaNode);


    overallTimer.Stop();
    LOG.debugv("Overall schema de-serialization for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), overallTimer.GetElapsedSeconds());

    if (m_schemaContext.GetSkipValidation())
        {
        LOG.infov("Skipping validation for '%s' because the read context has skip validation property set to true", schemaOut->GetFullSchemaName().c_str());
        delete reader; reader = nullptr;
        return SchemaReadStatus::Success;
        }

    if (!schemaOut->Validate(true))
        {
        delete reader; reader = nullptr;
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    delete reader; reader = nullptr;
    return SchemaReadStatus::Success;
    }

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteSchemaReferences()
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    for (auto const& pair : m_ecSchema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = pair.second;
        m_xmlWriter.WriteElementStart(ECXML_SCHEMAREFERENCE_ELEMENT);
        m_xmlWriter.WriteAttribute(NAME_ATTRIBUTE, refSchema->GetName().c_str());

        if (ECVersion::V2_0 == m_ecXmlVersion)
            m_xmlWriter.WriteAttribute(SCHEMAREF_VERSION_ATTRIBUTE, refSchema->GetSchemaKey().GetLegacyVersionString().c_str());
        else
            m_xmlWriter.WriteAttribute(SCHEMAREF_VERSION_ATTRIBUTE, refSchema->GetSchemaKey().GetVersionString().c_str());

        const Utf8String alias = refSchema->GetAlias();
        if (m_ecXmlVersion >= ECVersion::V3_1)
            m_xmlWriter.WriteAttribute(ALIAS_ATTRIBUTE, alias.c_str());
        else
            m_xmlWriter.WriteAttribute(SCHEMAREF_PREFIX_ATTRIBUTE, alias.c_str());

        m_xmlWriter.WriteElementEnd();
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteCustomAttributeDependencies(IECCustomAttributeContainerCR container)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    for (IECInstancePtr instance : container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass(currentClass);
        if (SchemaWriteStatus::Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteClass(ECClassCR ecClass)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    // don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != &m_ecSchema)
        return status;

    auto setIterator = m_context.m_alreadyWrittenClasses.find(ecClass.GetName().c_str());
    // Make sure we don't write any class twice
    if (setIterator != m_context.m_alreadyWrittenClasses.end())
        return status;

    m_context.m_alreadyWrittenClasses.insert(ecClass.GetName().c_str());

    // If schema element order shouldn't be preserved, baseclasses and contraints will be written
    // before the actual class to write. Else the order given by the WriteClass calls is used.
    if (!ecClass.GetSchema().m_serializationOrder.GetPreserveElementOrder())
        {
        // write the base classes first.
        for (ECClassP baseClass : ecClass.GetBaseClasses())
            WriteClass(*baseClass);

        // Serialize relationship constraint dependencies
        ECRelationshipClassP relClass = const_cast<ECRelationshipClassP>(ecClass.GetRelationshipClassCP());
        if (nullptr != relClass)
            {
            for (auto source : relClass->GetSource().GetConstraintClasses())
                WriteClass(*source);

            for (auto target : relClass->GetTarget().GetConstraintClasses())
                WriteClass(*target);
            }
        }

    WritePropertyDependencies(ecClass);
    WriteCustomAttributeDependencies(ecClass);

    return ecClass._WriteXml(m_xmlWriter, m_ecXmlVersion);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WritePropertyDependencies(ECClassCR ecClass)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    for (ECPropertyP prop : ecClass.GetProperties(false))
        {
        if (prop->GetIsStruct())
            {
            StructECPropertyP structProperty = prop->GetAsStructPropertyP();
            WriteClass(structProperty->GetType());
            }
        else if (prop->GetIsStructArray())
            {
            StructArrayECPropertyP arrayProperty = prop->GetAsStructArrayPropertyP();
            WriteClass(arrayProperty->GetStructElementType());
            }
        WriteCustomAttributeDependencies(*prop);
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaXmlWriter::Serialize(bool utf16)
    {
    // Checks to make sure the schema is not of a lower version than it's to be written to.
    if (m_ecSchema.GetECVersion() < m_ecXmlVersion)
        {
        LOG.errorv("Schema Serialization Violation: The ECVersion %s provided is higher than the ECVersion of the schema, %s.",
                   ECSchema::GetECVersionString(m_ecXmlVersion), ECSchema::GetECVersionString(m_ecSchema.GetECVersion()));
        return SchemaWriteStatus::FailedToCreateXml;
        }

    if (utf16)
        m_xmlWriter.WriteDocumentStart(XML_CHAR_ENCODING_UTF16LE);
    else
        m_xmlWriter.WriteDocumentStart(XML_CHAR_ENCODING_UTF8);

    Utf8PrintfString ns("%s.%s", ECXML_URI, ECSchema::GetECVersionString(m_ecXmlVersion));
    m_xmlWriter.WriteElementStart(EC_SCHEMA_ELEMENT, ns.c_str());

    m_xmlWriter.WriteAttribute(ECXML_SCHEMA_NAME_ATTRIBUTE, m_ecSchema.GetName().c_str());

    if (m_ecXmlVersion <= ECVersion::V3_0)
        {
        m_xmlWriter.WriteAttribute(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, m_ecSchema.GetAlias().c_str());
        m_xmlWriter.WriteAttribute(SCHEMA_VERSION_ATTRIBUTE, m_ecSchema.GetSchemaKey().GetLegacyVersionString().c_str());
        }
    else
        {
        m_xmlWriter.WriteAttribute(ALIAS_ATTRIBUTE, m_ecSchema.GetAlias().c_str());
        m_xmlWriter.WriteAttribute(SCHEMA_VERSION_ATTRIBUTE, m_ecSchema.GetSchemaKey().GetVersionString().c_str());
        }

    m_xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, m_ecSchema.GetInvariantDescription().c_str());
    if (m_ecSchema.GetIsDisplayLabelDefined())
        m_xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, m_ecSchema.GetInvariantDisplayLabel().c_str());

    WriteSchemaReferences();

    WriteCustomAttributeDependencies(m_ecSchema);
    m_ecSchema.WriteCustomAttributes(m_xmlWriter, m_ecXmlVersion);

    // if there is no Element Order specified, create a new default one (enumerations, classes) ordered alphabetically.
    auto& serializationOrder = m_ecSchema.m_serializationOrder;
    if (!serializationOrder.GetPreserveElementOrder())
        serializationOrder.CreateAlphabeticalOrder(m_ecSchema);

    // Serializes the Class and Enumerations in the given order...
    SchemaWriteStatus writeChildStatus;
    for (auto schemaElementEntry : serializationOrder)
        {
        Utf8CP elementName = schemaElementEntry.first.c_str();
        auto elementType = schemaElementEntry.second;
        if (elementType == ECSchemaElementType::ECClass)
            {
            ECClassCP ecClass = m_ecSchema.GetClassCP(elementName);
            if (nullptr != ecClass)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteClass(*ecClass)))
                    return writeChildStatus;
            }
        else if (elementType == ECSchemaElementType::ECEnumeration)
            {
            ECEnumerationCP ecEnumeration = m_ecSchema.GetEnumerationCP(elementName);
            if (nullptr != ecEnumeration)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<ECEnumeration>(*ecEnumeration)))
                    return writeChildStatus;
            }
        else if (elementType == ECSchemaElementType::KindOfQuantity)
            {
            KindOfQuantityCP kindOfQuantity = m_ecSchema.GetKindOfQuantityCP(elementName);
            if (nullptr != kindOfQuantity)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<KindOfQuantity>(*kindOfQuantity)))
                    return writeChildStatus;
            }
        else if (elementType == ECSchemaElementType::PropertyCategory)
            {
            PropertyCategoryCP propertyCategory = m_ecSchema.GetPropertyCategoryCP(elementName);
            if (nullptr != propertyCategory)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<PropertyCategory>(*propertyCategory)))
                    return writeChildStatus;
            }
        else if (ECSchemaElementType::UnitSystem == elementType)
            {
            UnitSystemCP system = m_ecSchema.GetUnitSystemCP(elementName);
            if (nullptr != system)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<UnitSystem>(*system)))
                    return writeChildStatus;
            }
        else if (ECSchemaElementType::Phenomenon == elementType)
            {
            PhenomenonCP phenom = m_ecSchema.GetPhenomenonCP(elementName);
            if (nullptr != phenom)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<Phenomenon>(*phenom)))
                    return writeChildStatus;
            }
        else if (ECSchemaElementType::Unit == elementType)
            {
            ECUnitCP unit = m_ecSchema.GetUnitCP(elementName);
            if(nullptr != unit)
                if (SchemaWriteStatus::Success != (writeChildStatus = WriteSchemaChild<ECUnit>(*unit)))
                    return writeChildStatus;
            }
        else if (ECSchemaElementType::InvertedUnit == elementType)
            {
            ECUnitCP unit = m_ecSchema.GetInvertedUnitCP(elementName);
            if(nullptr != unit)
                {
                if(&(unit->GetSchema()) != &m_ecSchema)
                    return SchemaWriteStatus::Success;
                unit->WriteInvertedUnitXml(m_xmlWriter, m_ecXmlVersion);
                }
            }
        else if (ECSchemaElementType::Constant == elementType)
            {
            ECUnitCP unit = m_ecSchema.GetConstantCP(elementName);
            if(nullptr != unit)
                {
                if(&(unit->GetSchema()) != &m_ecSchema)
                    return SchemaWriteStatus::Success;
                unit->WriteConstantXml(m_xmlWriter, m_ecXmlVersion);
                }
            }
        else if (ECSchemaElementType::Format == elementType)
            {
            ECFormatCP format = m_ecSchema.GetFormatCP(elementName);
            if(nullptr != format)
                WriteSchemaChild<ECFormat>(*format);
            }
        }

    m_xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
