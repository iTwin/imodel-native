/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaXml.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<bpair<ECClassP, BeXmlNodeP> >  ClassDeserializationVector;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaXmlReaderImpl
    {
    protected:
        BeXmlDomR               m_xmlDom;
        ECSchemaReadContextR    m_schemaContext;
        ECSchemaPtr m_conversionSchema;

        bool IsOpenPlantPidCircularReferenceSpecialCase(Utf8String& referencedECSchemaName, Utf8String& referencingECSchemaFullName);
        virtual bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) = 0;
        virtual SchemaReadStatus _ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes);
        virtual SchemaReadStatus _ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);

    protected:
        ECEntityClassP CreateEntityClass(ECSchemaPtr& schemaOut);
        ECStructClassP CreateStructClass(ECSchemaPtr& schemaOut);
        ECCustomAttributeClassP CreateCustomAttributeClass(ECSchemaPtr& schemaOut);
        ECRelationshipClassP CreateRelationshipClass(ECSchemaPtr& schemaOut);

    public:
        SchemaXmlReaderImpl(ECSchemaReadContextR context, BeXmlDomR xmlDom);
        virtual SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) = 0;

        virtual SchemaReadStatus ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes);
        virtual SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) = 0;
        SchemaReadStatus ReadEnumerationsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);
        SchemaReadStatus ReadKindOfQuantitiesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);
        
        void PopulateSchemaElementOrder(ECSchemaElementsOrder& elementOrder, BeXmlNodeR schemaNode);
        virtual bool IsECClassElementNode(BeXmlNodeR schemaNode);
        virtual bool IsECEnumerationElementNode(BeXmlNodeR schemaNode);
        virtual bool IsKindOfQuantityElementNode(BeXmlNodeR schemaNode);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaXmlReader2 : SchemaXmlReaderImpl
    {
    private:
        void DetermineClassTypeAndModifier(Utf8StringCR className, ECSchemaPtr schemaOut, ECClassType& classType, ECClassModifier& classModifier, bool isCA, bool isStruct, bool isDomain, bool isSealed) const;
        ECClassModifier DetermineRelationshipClassModifier(Utf8StringCR className, bool isDomain) const;
        bool DropClassAttributeDefined(Utf8StringCR className) const;

    protected:
        bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) override;

    public:
        SchemaXmlReader2(ECSchemaReadContextR context, ECSchemaPtr schemaOut, BeXmlDomR xmlDom);
        SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) override;
        SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaXmlReader3 : SchemaXmlReaderImpl
    {
    protected:
        bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) override;

    public:
        SchemaXmlReader3(ECSchemaReadContextR context, BeXmlDomR xmlDom);
        SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) override;
        SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) override;
        
        virtual bool IsECClassElementNode(BeXmlNodeR schemaNode) override;
    };

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaXmlReader::SetErrorHandling(bool doAssert)
    {
    s_noAssert = !doAssert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReaderImpl::SchemaXmlReaderImpl(ECSchemaReadContextR context, BeXmlDomR xmlDom) : m_schemaContext(context), m_xmlDom(xmlDom)
{ }

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
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::_ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    BeXmlDom::IterableNodeSet schemaReferenceNodes;
    schemaNode.SelectChildNodes(schemaReferenceNodes, EC_NAMESPACE_PREFIX ":" EC_SCHEMAREFERENCE_ELEMENT);
    for (BeXmlNodeP& schemaReferenceNode : schemaReferenceNodes)
        {
        SchemaKey key;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(key.m_schemaName, SCHEMAREF_NAME_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_NAME_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        Utf8String alias;
        if (schemaOut->GetOriginalECXmlVersionMajor() >= 3 && schemaOut->GetOriginalECXmlVersionMinor() >= 1)
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

        if (ECObjectsStatus::Success != SchemaKey::ParseVersionString(key.m_versionMajor, key.m_versionWrite, key.m_versionMinor, versionString.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", key.m_schemaName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        // If the schema (uselessly) references itself, just skip it
        if (schemaOut->GetSchemaKey().m_schemaName.compare(key.m_schemaName) == 0)
            continue;

        Utf8String schemaFullName = schemaOut->GetFullSchemaName();
        if (IsOpenPlantPidCircularReferenceSpecialCase(key.m_schemaName, schemaFullName))
            continue;

        LOG.debugv("About to locate referenced ECSchema %s", key.GetFullSchemaName().c_str());

        // There are some schemas out there that reference the non-existent Unit_Attributes.1.1 schema.  We need to deliver 1.0, which does not match our criteria
        // for LatestCompatible.
        if (0 == key.GetName().CompareTo("Unit_Attributes") && 1 == key.GetVersionMajor() && 1 == key.GetVersionMinor())
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
SchemaReadStatus SchemaXmlReaderImpl::ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;
    bool resolveConflicts = false;
    if (m_conversionSchema.IsValid())
        resolveConflicts = m_conversionSchema->IsDefined("ResolveClassNameConflicts");

    bvector<Utf8String> comments;
    // Create ECClass Stubs (no properties)
    for (BeXmlNodeP classNode = schemaNode.GetFirstChild(BEXMLNODE_Any); NULL != classNode; classNode = classNode->GetNextSibling(BEXMLNODE_Any))
        {
        if (m_schemaContext.GetPreserveXmlComments())
            {
            if (classNode->type == BEXMLNODE_Comment)
                {
                Utf8String comment;
                if (classNode->GetContent(comment) == BeXmlStatus::BEXML_Success)
                    {
                    comments.push_back(comment);
                    }
                }
            }
        if (classNode->type != BEXMLNODE_Element)
            continue;

        ECClassP       ecClass = nullptr;
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
    ECSchemaPtr conversionSchema = m_schemaContext.LocateConversionSchemaFor(schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor());

    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass = classesIterator->first;
        classNode = classesIterator->second;
        status = ecClass->_ReadXmlContents(*classNode, m_schemaContext, conversionSchema.get(), navigationProperties);
        if (SchemaReadStatus::Success != status)
            return status;
        }

    for (auto const& navProp : navigationProperties)
        if (!navProp->Verify())
            {
            LOG.errorv("Unable to load NavigationECProperty '%s:%s.%s' because the relationship '%s' does not support this class as a constraint when traversed in the '%s' direction or max multiplicity is greater than 1.",
                        navProp->GetClass().GetSchema().GetName().c_str(), navProp->GetClass().GetName().c_str(), navProp->GetName().c_str(),
                        navProp->GetRelationshipClass()->GetName().c_str(), ECXml::DirectionToString(navProp->GetDirection()));
                
            return SchemaReadStatus::InvalidECSchemaXml;
            }

    return status;
    }


//---------------------------------------------------------------------------------------
// These class constructors are declared as private to prevent regular uses from creating unnamed
// classes outside of a schema.  Instead of adding each variant of the reader as a friend, this
// way seemed simpler.
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECEntityClassP SchemaXmlReaderImpl::CreateEntityClass(ECSchemaPtr& schemaOut)
    {
    return new ECEntityClass(*schemaOut);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECStructClassP SchemaXmlReaderImpl::CreateStructClass(ECSchemaPtr& schemaOut)
    {
    return new ECStructClass(*schemaOut);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECCustomAttributeClassP SchemaXmlReaderImpl::CreateCustomAttributeClass(ECSchemaPtr& schemaOut)
    {
    return new ECCustomAttributeClass(*schemaOut);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECRelationshipClassP SchemaXmlReaderImpl::CreateRelationshipClass(ECSchemaPtr& schemaOut)
    {
    return new ECRelationshipClass(*schemaOut);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReaderImpl::IsECEnumerationElementNode(BeXmlNodeR elementNode)
    {
    return 0 == strcmp(EC_ENUMERATION_ELEMENT, elementNode.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReaderImpl::IsKindOfQuantityElementNode(BeXmlNodeR elementNode)
    {
    return 0 == strcmp(KIND_OF_QUANTITY_ELEMENT, elementNode.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReaderImpl::IsECClassElementNode(BeXmlNodeR elementNode)
    {
    return 0 == strcmp(EC_CLASS_ELEMENT, elementNode.GetName()) || 
           0 == strcmp(EC_RELATIONSHIP_CLASS_ELEMENT, elementNode.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader2::SchemaXmlReader2(ECSchemaReadContextR context, ECSchemaPtr schemaOut, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom)
    {
    m_conversionSchema = context.LocateConversionSchemaFor(schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor());
    }

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
        ECXml::ParseBooleanString(isDomain, boolStr.c_str());

    if (0 == strcmp(nodeName, EC_CLASS_ELEMENT))
        {
        // Need to determine what type of class this actually is in EC 3.0
        bool isCA = false;
        bool isStruct = false;
        bool isSealed = false;
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_CUSTOMATTRIBUTE_ATTRIBUTE))
            ECXml::ParseBooleanString(isCA, boolStr.c_str());
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_STRUCT_ATTRIBUTE))
            ECXml::ParseBooleanString(isStruct, boolStr.c_str());
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_FINAL_ATTRIBUTE))
            ECXml::ParseBooleanString(isSealed, boolStr.c_str());

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
    else if (0 == strcmp(nodeName, EC_RELATIONSHIP_CLASS_ELEMENT))
        {
        ecClass = CreateRelationshipClass(schemaOut);
        ecClass->SetClassModifier(DetermineRelationshipClassModifier(className, isDomain));
        }
    else
        return false;

    return true;
    }

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

    return modifier;
    }

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
            LOG.warningv("Class '%s' in schema '%s' has more than one type flag set to true: isStruct(%d) isDomainClass(%d) isCustomAttributeClass(%d).  Only one is allowed, defaulting to %s.  "
                     "Modify the schema or use the ECv3ConversionAttributes in a conversion schema named '%s' to force a different class type.",
                     className.c_str(), schemaOut->GetFullSchemaName().c_str(), isStruct, isDomain, isCA, isStruct ? "Struct" : "CustomAttribute",
                     schemaOut->GetFullSchemaName().insert(schemaOut->GetName().length(), "_V3Conversion").c_str());

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
                     "Modify the schema or use the ECv3ConversionAttributes in a conversion schema named '%s' to force a different class type.",
                     className.c_str(), schemaOut->GetFullSchemaName().c_str(), isStruct, isDomain, isCA, isStruct ? "Struct" : "CustomAttribute",
                     schemaOut->GetFullSchemaName().insert(schemaOut->GetName().length(), "_V3Conversion").c_str());


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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader2::ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    return _ReadClassContentsFromXml(schemaOut, classes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader2::ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader3::SchemaXmlReader3(ECSchemaReadContextR context, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom)
{ }
//---------------------------------------------------------------------------------------
// @bsimethod                           
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader3::IsECClassElementNode(BeXmlNodeR elementNode)
    {
    Utf8CP nodeName = elementNode.GetName();
    return SchemaXmlReaderImpl::IsECClassElementNode(elementNode) ||
           0 == strcmp(EC_STRUCTCLASS_ELEMENT, nodeName) ||
           0 == strcmp(EC_CUSTOMATTRIBUTECLASS_ELEMENT, nodeName) ||
           0 == strcmp(EC_ENTITYCLASS_ELEMENT, nodeName);
    }

//---------------------------------------------------------------------------------------
// Create ECClass Stubs (no attributes or properties)
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader3::ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut)
    {
    ECEntityClassP entityClass = nullptr;
    ECStructClassP structClass = nullptr;
    ECRelationshipClassP relationshipClass = nullptr;
    ECCustomAttributeClassP caClass = nullptr;

    Utf8CP nodeName = classNode.GetName();

    if (0 == strcmp(EC_CLASS_ELEMENT, nodeName))
        {}
    else if (0 == strcmp(EC_STRUCTCLASS_ELEMENT, nodeName))
        {
        structClass = CreateStructClass(schemaOut);
        ecClass = structClass;
        }
    else if (0 == strcmp(EC_CUSTOMATTRIBUTECLASS_ELEMENT, nodeName))
        {
        caClass = CreateCustomAttributeClass(schemaOut);
        ecClass = caClass;
        }
    else if (0 == strcmp(EC_ENTITYCLASS_ELEMENT, nodeName))
        {
        entityClass = CreateEntityClass(schemaOut);
        ecClass = entityClass;
        }
    else if (0 == strcmp(EC_RELATIONSHIP_CLASS_ELEMENT, nodeName))
        {
        relationshipClass = CreateRelationshipClass(schemaOut);
        ecClass = relationshipClass;
        }

    if (nullptr == ecClass)
        return false;

    return true;
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
        if (BEXML_Success != candidateNode->GetAttributeStringValue(typeName, TYPE_NAME_ATTRIBUTE))
            {
            // if it doesn't have a typename, it can't be an ECClass or an Enumeration, so
            // loop can continue.
            continue;
            }

        if (IsECClassElementNode(*candidateNode))
            {
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::ECClass);
            }
        else if(IsECEnumerationElementNode(*candidateNode))
            {
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::ECEnumeration);
            }
        else if (IsKindOfQuantityElementNode(*candidateNode))
            {
            elementOrder.AddElement(typeName.c_str(), ECSchemaElementType::KindOfQuantity);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::ReadEnumerationsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        if (!IsECEnumerationElementNode(*candidateNode))
            {
            continue; //node is not an enumeration
            }

        ECEnumerationP ecEnumeration = new ECEnumeration(*schemaOut);
        status = ecEnumeration->ReadXml(*candidateNode, m_schemaContext);
        if (SchemaReadStatus::Success != status)
            {
            delete ecEnumeration;
            return status;
            }

        Utf8StringCR name = ecEnumeration->GetName();
        ECObjectsStatus addStatus = schemaOut->AddEnumeration(ecEnumeration);

        if (addStatus == ECObjectsStatus::NamedItemAlreadyExists)
            {
            LOG.errorv("Duplicate enumeration node for %s in schema %s.", name.c_str(), schemaOut->GetFullSchemaName().c_str());
            delete ecEnumeration;
            ecEnumeration = nullptr;
            return SchemaReadStatus::DuplicateTypeName;
            }

        if (ECObjectsStatus::Success != addStatus)
            {
            delete ecEnumeration;
            ecEnumeration = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::ReadKindOfQuantitiesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        if (!IsKindOfQuantityElementNode(*candidateNode))
            {
            continue; //node is not relevant
            }

        KindOfQuantityP kindOfQuantity = new KindOfQuantity(*schemaOut);
        status = kindOfQuantity->ReadXml(*candidateNode, m_schemaContext);
        if (SchemaReadStatus::Success != status)
            {
            delete kindOfQuantity;
            return status;
            }

        ECObjectsStatus addStatus = schemaOut->AddKindOfQuantity(kindOfQuantity);

        if (addStatus == ECObjectsStatus::NamedItemAlreadyExists)
            {
            LOG.errorv("Duplicate kind of quantity node for %s in schema %s.", kindOfQuantity->GetName().c_str(), schemaOut->GetFullSchemaName().c_str());
            delete kindOfQuantity;
            kindOfQuantity = nullptr;
            return SchemaReadStatus::DuplicateTypeName;
            }

        if (ECObjectsStatus::Success != addStatus)
            {
            delete kindOfQuantity;
            kindOfQuantity = nullptr;
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader3::ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    return _ReadClassContentsFromXml(schemaOut, classes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader3::ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader::SchemaXmlReader(ECSchemaReadContextR context, BeXmlDomR xmlDom) : m_schemaContext(context), m_xmlDom(xmlDom)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader::Deserialize(ECSchemaPtr& schemaOut, uint32_t checkSum)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;
    StopWatch overallTimer("Overall schema de-serialization timer", true);

    BeXmlNodeP      rootNode;
    rootNode = static_cast <BeXmlNodeP>(xmlDocGetRootElement(&(m_xmlDom.GetDocument())));
    if (NULL == rootNode)
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
    int ecXmlMajorVersion, ecXmlMinorVersion;

    sscanf(version.c_str(), "%d.%d", &ecXmlMajorVersion, &ecXmlMinorVersion);
    if (2 != ecXmlMajorVersion && 3 != ecXmlMajorVersion)
        {
        LOG.errorv("Unsupported ecXml version %d.%d", ecXmlMajorVersion, ecXmlMinorVersion);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    m_xmlDom.RegisterNamespace(EC_NAMESPACE_PREFIX, schemaNamespace.c_str());

    BeXmlNodeP      schemaNode;
    if ((BEXML_Success != m_xmlDom.SelectNode(schemaNode, "/" EC_NAMESPACE_PREFIX ":" EC_SCHEMA_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == schemaNode))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: Missing a top-level %s node in the %s namespace", EC_SCHEMA_ELEMENT, schemaNamespace.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    // schemaName is a REQUIRED attribute in order to create the schema
    Utf8String schemaName;
    if (BEXML_Success != schemaNode->GetAttributeStringValue(schemaName, SCHEMA_NAME_ATTRIBUTE))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a schemaName attribute", EC_SCHEMA_ELEMENT);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    uint32_t versionMajor = DEFAULT_VERSION_MAJOR;
    uint32_t versionWrite = DEFAULT_VERSION_WRITE;
    uint32_t versionMinor = DEFAULT_VERSION_MINOR;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    Utf8String     versionString;
    if ((BEXML_Success != schemaNode->GetAttributeStringValue(versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
        (ECObjectsStatus::Success != SchemaKey::ParseVersionString(versionMajor, versionWrite, versionMinor, versionString.c_str())))
        {
        LOG.warningv("Invalid version attribute has been ignored while reading ECSchema '%s'.  The default version number %s has been applied.",
                     schemaName.c_str(), SchemaKey::FormatSchemaVersion(versionMajor, versionWrite, versionMinor).c_str());
        }

    LOG.debugv("Reading ECSchema %s", SchemaKey::FormatFullSchemaName(schemaName.c_str(), versionMajor, versionWrite, versionMinor).c_str());

    //Using the old overload of CreateSchema as we don't have the alias at this point.
    ECObjectsStatus createStatus = ECSchema::CreateSchema(schemaOut, schemaName, versionMajor, versionMinor);
    if (ECObjectsStatus::Success != createStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    if(schemaOut->SetVersionWrite(versionWrite) != ECObjectsStatus::Success)
        return SchemaReadStatus::InvalidECSchemaXml;

    schemaOut->m_key.m_checkSum = checkSum;

    schemaOut->m_originalECXmlVersionMajor = ecXmlMajorVersion;
    schemaOut->m_originalECXmlVersionMinor = ecXmlMinorVersion;

    if (ECObjectsStatus::DuplicateSchema == m_schemaContext.AddSchema(*schemaOut))
        {
        return SchemaReadStatus::DuplicateSchema;
        }

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    Utf8String value;  // used by macro.
    if (ecXmlMajorVersion >= 3 && ecXmlMinorVersion >= 1)
        {
        READ_REQUIRED_XML_ATTRIBUTE((*schemaNode), ALIAS_ATTRIBUTE, schemaOut, Alias, EC_SCHEMA_ELEMENT)
        }
    else
        {
        READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, schemaOut, Alias)
        }

    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), DESCRIPTION_ATTRIBUTE, schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), DISPLAY_LABEL_ATTRIBUTE, schemaOut, DisplayLabel)

    StopWatch readingSchemaReferences("Reading Schema References", true);
    SchemaXmlReaderImpl* reader = nullptr;
    if (2 == ecXmlMajorVersion)
        reader = new SchemaXmlReader2(m_schemaContext, schemaOut, m_xmlDom);
    else
        reader = new SchemaXmlReader3(m_schemaContext, m_xmlDom);

    if (SchemaReadStatus::Success != (status = reader->ReadSchemaReferencesFromXml(schemaOut, *schemaNode)))
        return status;

    readingSchemaReferences.Stop();
    LOG.tracev("Reading schema references for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingSchemaReferences.GetElapsedSeconds());

    ClassDeserializationVector classes;
    StopWatch readingClassStubs("Reading class stubs", true);
    status = reader->ReadClassStubsFromXml(schemaOut, *schemaNode, classes);

    if (SchemaReadStatus::Success != status)
        return status;
    
    readingClassStubs.Stop();
    LOG.tracev("Reading class stubs for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassStubs.GetElapsedSeconds());

    StopWatch readingEnumerations("Reading enumerations", true);
    status = reader->ReadEnumerationsFromXml(schemaOut, *schemaNode);

    if (SchemaReadStatus::Success != status)
        return status;
    
    readingEnumerations.Stop();
    LOG.tracev("Reading enumerations for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingEnumerations.GetElapsedSeconds());

    StopWatch readingKindOfQuantities("Reading kind of quantity", true);
    status = reader->ReadKindOfQuantitiesFromXml(schemaOut, *schemaNode);

    if (SchemaReadStatus::Success != status)
        return status;

    readingKindOfQuantities.Stop();
    LOG.tracev("Reading kind of quantity elements for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingKindOfQuantities.GetElapsedSeconds());

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    StopWatch readingClassContents("Reading class contents", true);
    if (SchemaReadStatus::Success != (status = reader->ReadClassContentsFromXml(schemaOut, classes)))
        return status;
    
    readingClassContents.Stop();
    LOG.tracev("Reading class contents for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassContents.GetElapsedSeconds());

    StopWatch readingCustomAttributes("Reading custom attributes", true);
    if (CustomAttributeReadStatus::InvalidCustomAttributes == schemaOut->ReadCustomAttributes(*schemaNode, m_schemaContext, *schemaOut))
        {
        LOG.errorv("Failed to read schema because one or more invalid custom attributes were applied to it.", schemaOut->GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    
    readingCustomAttributes.Stop();
    LOG.tracev("Reading custom attributes for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingCustomAttributes.GetElapsedSeconds());

    // If switch is set in reading context, the class order of the schema xml will be preserved that it can be written out in the same order.
    if (m_schemaContext.GetPreserveElementOrder())
        {
        reader->PopulateSchemaElementOrder(schemaOut->m_serializationOrder, *schemaNode);
        }

    //Compute the schema checkSum
    overallTimer.Stop();
    LOG.debugv("Overall schema de-serialization for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), overallTimer.GetElapsedSeconds());

    return SchemaReadStatus::Success;
    }

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlWriter::SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema, int ecXmlVersionMajor, int ecXmlVersionMinor) : m_xmlWriter(xmlWriter), m_ecSchema(ecSchema), m_ecXmlVersionMajor(ecXmlVersionMajor), m_ecXmlVersionMinor(ecXmlVersionMinor)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteSchemaReferences()
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    bmap<ECSchemaP, Utf8String>::const_iterator iterator;
    for (iterator = m_ecSchema.m_referencedSchemaAliasMap.begin(); iterator != m_ecSchema.m_referencedSchemaAliasMap.end(); iterator++)
        {
        bpair<ECSchemaP, const Utf8String> mapPair = *(iterator);
        ECSchemaP   refSchema = mapPair.first;
        m_xmlWriter.WriteElementStart(EC_SCHEMAREFERENCE_ELEMENT);
        m_xmlWriter.WriteAttribute(SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str());

        if (m_ecXmlVersionMajor == 2)
            {
            m_xmlWriter.WriteAttribute(SCHEMAREF_VERSION_ATTRIBUTE, refSchema->GetSchemaKey().GetLegacyVersionString().c_str());
            }
        else
            {
            m_xmlWriter.WriteAttribute(SCHEMAREF_VERSION_ATTRIBUTE, refSchema->GetSchemaKey().GetVersionString().c_str());
            }

        const Utf8String alias = mapPair.second;
        if (m_ecXmlVersionMajor >= 3 && m_ecXmlVersionMinor >= 1)
            {
            m_xmlWriter.WriteAttribute(ALIAS_ATTRIBUTE, alias.c_str());
            }
        else
            {
            m_xmlWriter.WriteAttribute(SCHEMAREF_PREFIX_ATTRIBUTE, alias.c_str());
            }

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

    bset<Utf8CP>::const_iterator setIterator;
    setIterator = m_context.m_alreadyWrittenClasses.find(ecClass.GetName().c_str());
    // Make sure we don't write any class twice
    if (setIterator != m_context.m_alreadyWrittenClasses.end())
        return status;
    else
        m_context.m_alreadyWrittenClasses.insert(ecClass.GetName().c_str());

    // If schema element order shouldn't be preserved, baseclasses and contraints will be written
    //  before the actual class to write. Else the order given by the WriteClass calls is used.
    if (!ecClass.GetSchema().m_serializationOrder.GetPreserveElementOrder())
        {
        // write the base classes first.
        for (ECClassP baseClass : ecClass.GetBaseClasses())
            WriteClass(*baseClass);

        // Serialize relationship constraint dependencies
        ECRelationshipClassP relClass = const_cast<ECRelationshipClassP>(ecClass.GetRelationshipClassCP());
        if (NULL != relClass)
            {
            for (auto source : relClass->GetSource().GetConstraintClasses())
                WriteClass(source->GetClass());

            for (auto target : relClass->GetTarget().GetConstraintClasses())
                WriteClass(target->GetClass());
            }
        }

    WritePropertyDependencies(ecClass);
    WriteCustomAttributeDependencies(ecClass);

    return ecClass._WriteXml(m_xmlWriter, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteEnumeration(ECEnumerationCR ecEnumeration)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    // don't write any enumerations that aren't in the schema we're writing.
    if (&(ecEnumeration.GetSchema()) != &m_ecSchema)
        return status;

    //WriteCustomAttributeDependencies(ecEnumeration);
    return ecEnumeration.WriteXml(m_xmlWriter, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    // don't write any elements that aren't in the schema we're writing.
    if (&(kindOfQuantity.GetSchema()) != &m_ecSchema)
        return status;

    //WriteCustomAttributeDependencies(ecEnumeration);
    return kindOfQuantity.WriteXml(m_xmlWriter, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
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
            WriteClass(*(arrayProperty->GetStructElementType()));
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
    if (utf16)
        m_xmlWriter.WriteDocumentStart(XML_CHAR_ENCODING_UTF16LE);
    else
        m_xmlWriter.WriteDocumentStart(XML_CHAR_ENCODING_UTF8);

    Utf8PrintfString ns("%s.%d.%d", ECXML_URI, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
    m_xmlWriter.WriteElementStart(EC_SCHEMA_ELEMENT, ns.c_str());

    m_xmlWriter.WriteAttribute(SCHEMA_NAME_ATTRIBUTE, m_ecSchema.GetName().c_str());

    if (m_ecXmlVersionMajor == 2)
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
        m_xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, m_ecSchema.GetInvariantDisplayLabel().c_str());

    WriteSchemaReferences();

    WriteCustomAttributeDependencies(m_ecSchema);
    m_ecSchema.WriteCustomAttributes(m_xmlWriter);

    // if there is no Element Order specified, create a new default one (enumerations, classes) ordered alphabetically.
    auto& serializationOrder = m_ecSchema.m_serializationOrder;
    if (!serializationOrder.GetPreserveElementOrder())
        {
        serializationOrder.CreateAlphabeticalOrder(m_ecSchema);
        }
    
    // Serializes the Class and Enumerations in the given order...
    for (auto schemaElementEntry : serializationOrder)
        {
        Utf8CP elementName = schemaElementEntry.first.c_str();
        auto elementType = schemaElementEntry.second;
        if (elementType == ECSchemaElementType::ECClass)
            {
            ECClassCP ecClass = m_ecSchema.GetClassCP(elementName);
            if (ecClass != nullptr)
                {
                WriteClass(*ecClass);
                }
            }
        else if (elementType == ECSchemaElementType::ECEnumeration)
            {
            ECEnumerationCP ecEnumeration = m_ecSchema.GetEnumerationCP(elementName);
            if (ecEnumeration != nullptr)
                {
                WriteEnumeration(*ecEnumeration);
                }
            }
        else if (elementType == ECSchemaElementType::KindOfQuantity)
            {
            KindOfQuantityCP kindOfQuantity = m_ecSchema.GetKindOfQuantityCP(elementName);
            if (kindOfQuantity != nullptr)
                {
                WriteKindOfQuantity(*kindOfQuantity);
                }
            }
        }

    m_xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;
    }
END_BENTLEY_ECOBJECT_NAMESPACE

