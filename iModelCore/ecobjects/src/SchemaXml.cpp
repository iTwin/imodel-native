/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaXml.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"
#include <list>

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

        bool IsOpenPlantPidCircularReferenceSpecialCase(Utf8String& referencedECSchemaName, Utf8String& referencingECSchemaFullName);
        virtual bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) = 0;
        SchemaReadStatus _ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes, int ecXmlVersionMajor);

    protected:
        ECEntityClassP CreateEntityClass(ECSchemaPtr& schemaOut);
        ECStructClassP CreateStructClass(ECSchemaPtr& schemaOut);
        ECCustomAttributeClassP CreateCustomAttributeClass(ECSchemaPtr& schemaOut);
        ECRelationshipClassP CreateRelationshipClass(ECSchemaPtr& schemaOut);

    public:
        SchemaXmlReaderImpl(ECSchemaReadContextR context, BeXmlDomR xmlDom);
        SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);

        virtual SchemaReadStatus ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes);
        virtual SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) = 0;
        SchemaReadStatus ReadEnumerationsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaXmlReader2 : SchemaXmlReaderImpl
    {
    protected:
        bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) override;

    public:
        SchemaXmlReader2(ECSchemaReadContextR context, BeXmlDomR xmlDom);
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
        SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassNameComparer(ECClassP class1, ECClassP class2)
    {
    // We should never have a NULL ECClass here.
    // However we will pretend a NULL ECClass is always less than a non-NULL ECClass
    BeAssert(NULL != class1 && NULL != class2);
    if (NULL == class1)
        return NULL != class2;      // class 1 < class2 if class2 non-null, equal otherwise
    else if (NULL == class2)
        return false;               // class1 > class2

    int comparison = class1->GetName().CompareTo(class2->GetName());
    return comparison < 0;
    }


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

    return (0 == referencingECSchemaFullName.CompareTo("OpenPlant_Supplemental_Mapping_OPPID.01.01") || 0 == referencingECSchemaFullName.CompareTo("OpenPlant_Supplemental_Mapping_OPPID.01.02"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
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

        Utf8String prefix;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(prefix, SCHEMAREF_PREFIX_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_PREFIX_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }


        Utf8String versionString;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue(versionString, SCHEMAREF_VERSION_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_VERSION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (ECObjectsStatus::Success != ECSchema::ParseVersionString(key.m_versionMajor, key.m_versionMinor, versionString.c_str()))
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
            //We can encounter some time same schema referenced twice with different namespacePrefix.
            //We will not treat it as error.
            SchemaKeyCR refSchemaKey = referencedSchema->GetSchemaKey();
            auto const& references = schemaOut->GetReferencedSchemas();
            if (references.end() != references.find(refSchemaKey))
                {
                continue;
                }

            ECObjectsStatus status = schemaOut->AddReferencedSchema(*referencedSchema, prefix, m_schemaContext);
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

    // Create ECClass Stubs (no properties)
    for (BeXmlNodeP classNode = schemaNode.GetFirstChild(); NULL != classNode; classNode = classNode->GetNextSibling())
        {
        ECClassP       ecClass = nullptr;
        if (!ReadClassNode(ecClass, *classNode, schemaOut))
            continue;

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

        Utf8StringCR name = ecClass->GetName();
        ECObjectsStatus addStatus = schemaOut->AddClass(ecClass);

        if (addStatus == ECObjectsStatus::NamedItemAlreadyExists)
            {
            LOG.errorv("Duplicate class node for %s in schema %s.", name.c_str(), schemaOut->GetFullSchemaName().c_str());
            return SchemaReadStatus::DuplicateTypeName;
            }

        if (ECObjectsStatus::Success != addStatus)
            return SchemaReadStatus::InvalidECSchemaXml;

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
SchemaReadStatus SchemaXmlReaderImpl::_ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes, int ecXmlVersionMajor)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP    ecClass;
    BeXmlNodeP  classNode;
    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass = classesIterator->first;
        classNode = classesIterator->second;
        status = ecClass->_ReadXmlContents(*classNode, m_schemaContext, ecXmlVersionMajor);
        if (SchemaReadStatus::Success != status)
            return status;
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
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader2::SchemaXmlReader2(ECSchemaReadContextR context, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom)
{ }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaXmlReader2::ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut)
    {
    // Create ECClass Stubs (no properties)
    ECStructClassP structClass = nullptr;
    ECRelationshipClassP relationshipClass = nullptr;
    ECCustomAttributeClassP caClass = nullptr;

    Utf8CP nodeName = classNode.GetName();

    if (0 == strcmp(nodeName, EC_CLASS_ELEMENT))
        {
        // Need to determine what type of class this actually is in EC 3.0
        Utf8String boolStr;
        bool isCA = false;
        bool isStruct = false;
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_CUSTOMATTRIBUTE_ATTRIBUTE))
            ECXml::ParseBooleanString(isCA, boolStr.c_str());
        if (BEXML_Success == classNode.GetAttributeStringValue(boolStr, IS_STRUCT_ATTRIBUTE))
            ECXml::ParseBooleanString(isStruct, boolStr.c_str());

        if (isCA && isStruct)
            {
            Utf8String     className;
            classNode.GetAttributeStringValue(className, TYPE_NAME_ATTRIBUTE);
            if (className.CompareTo("TransformationValueMap") != 0)
                {
                LOG.errorv("Class %s in Schema %s is marked as both Struct and CustomAttribute.  This is not allowed.", className.c_str(), schemaOut->GetFullSchemaName().c_str());
                //return SchemaReadStatus::InvalidECSchemaXml;
                }
            }
        if (isStruct)
            {
            structClass = CreateStructClass(schemaOut);
            ecClass = structClass;
            }
        else if (isCA)
            {
            caClass = CreateCustomAttributeClass(schemaOut);
            ecClass = caClass;
            }
        else
            ecClass = CreateEntityClass(schemaOut);
        }
    else if (0 == strcmp(nodeName, EC_RELATIONSHIP_CLASS_ELEMENT))
        {
        relationshipClass = CreateRelationshipClass(schemaOut);
        ecClass = relationshipClass;
        }
    else
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader2::ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    return _ReadClassContentsFromXml(schemaOut, classes, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader3::SchemaXmlReader3(ECSchemaReadContextR context, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom)
{ }

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

    Utf8String modifierStr = false;
    ECClassModifier modifier;
    if (BEXML_Success == classNode.GetAttributeStringValue(modifierStr, MODIFIER_ATTRIBUTE))
        {
        ECXml::ParseModifierString(modifier, modifierStr);
        ecClass->SetClassModifier(modifier);
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader3::ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    return _ReadClassContentsFromXml(schemaOut, classes, 3);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReaderImpl::ReadEnumerationsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    // Create ECClass Stubs (no properties)
    for (BeXmlNodeP candidateNode = schemaNode.GetFirstChild(); nullptr != candidateNode; candidateNode = candidateNode->GetNextSibling())
        {
        Utf8CP nodeName = candidateNode->GetName();
        if (0 != strcmp(EC_ENUMERATION_ELEMENT, nodeName))
            {
            continue; //node is not an enumeration
            }

        ECEnumerationP ecEnumeration = new ECEnumeration(*schemaOut);
        status = ecEnumeration->_ReadXml(*candidateNode, m_schemaContext);
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
            return SchemaReadStatus::DuplicateTypeName;
            }

        if (ECObjectsStatus::Success != addStatus)
            return SchemaReadStatus::InvalidECSchemaXml;
        }
    return status;
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
    StopWatch overallTimer(L"Overall schema de-serialization timer", true);

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
    uint32_t versionMinor = DEFAULT_VERSION_MINOR;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    Utf8String     versionString;
    if ((BEXML_Success != schemaNode->GetAttributeStringValue(versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
        (ECObjectsStatus::Success != ECSchema::ParseVersionString(versionMajor, versionMinor, versionString.c_str())))
        {
        LOG.warningv("Invalid version attribute has been ignored while reading ECSchema '%s'.  The default version number %02d.%02d has been applied.",
                     schemaName.c_str(), versionMajor, versionMinor);
        }

    LOG.debugv("Reading ECSchema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);

    ECObjectsStatus createStatus = ECSchema::CreateSchema(schemaOut, schemaName, versionMajor, versionMinor);
    if (ECObjectsStatus::Success != createStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    schemaOut->m_key.m_checkSum = checkSum;

    if (ECObjectsStatus::DuplicateSchema == m_schemaContext.AddSchema(*schemaOut))
        {
        return SchemaReadStatus::DuplicateSchema;
        }

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    Utf8String value;  // used by macro.
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), DESCRIPTION_ATTRIBUTE, schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE((*schemaNode), DISPLAY_LABEL_ATTRIBUTE, schemaOut, DisplayLabel)

    StopWatch readingSchemaReferences(L"Reading Schema References", true);
    SchemaXmlReaderImpl* reader = nullptr;
    if (2 == ecXmlMajorVersion)
        reader = new SchemaXmlReader2(m_schemaContext, m_xmlDom);
    else
        reader = new SchemaXmlReader3(m_schemaContext, m_xmlDom);

    if (SchemaReadStatus::Success != (status = reader->ReadSchemaReferencesFromXml(schemaOut, *schemaNode)))
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }

    readingSchemaReferences.Stop();
    LOG.tracev("Reading schema references for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingSchemaReferences.GetElapsedSeconds());

    ClassDeserializationVector classes;
    StopWatch readingClassStubs(L"Reading class stubs", true);
    status = reader->ReadClassStubsFromXml(schemaOut, *schemaNode, classes);

    if (SchemaReadStatus::Success != status)
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }
    readingClassStubs.Stop();
    LOG.tracev("Reading class stubs for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassStubs.GetElapsedSeconds());

    StopWatch readingEnumerations(L"Reading enumerations", true);
    status = reader->ReadEnumerationsFromXml(schemaOut, *schemaNode);

    if (SchemaReadStatus::Success != status)
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = nullptr;
        return status;
        }
    readingEnumerations.Stop();
    LOG.tracev("Reading enumerations stubs for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingEnumerations.GetElapsedSeconds());

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    StopWatch readingClassContents(L"Reading class contents", true);
    if (SchemaReadStatus::Success != (status = reader->ReadClassContentsFromXml(schemaOut, classes)))
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }
    readingClassContents.Stop();
    LOG.tracev("Reading class contents for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassContents.GetElapsedSeconds());

    StopWatch readingCustomAttributes(L"Reading custom attributes", true);
    schemaOut->ReadCustomAttributes(*schemaNode, m_schemaContext, *schemaOut);
    readingCustomAttributes.Stop();
    LOG.tracev("Reading custom attributes for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingCustomAttributes.GetElapsedSeconds());


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
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteSchemaReferences()
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    bmap<ECSchemaP, Utf8String>::const_iterator iterator;
    for (iterator = m_ecSchema.m_referencedSchemaNamespaceMap.begin(); iterator != m_ecSchema.m_referencedSchemaNamespaceMap.end(); iterator++)
        {
        bpair<ECSchemaP, const Utf8String> mapPair = *(iterator);
        ECSchemaP   refSchema = mapPair.first;
        m_xmlWriter.WriteElementStart(EC_SCHEMAREFERENCE_ELEMENT);
        m_xmlWriter.WriteAttribute(SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str());

        Utf8Char versionString[8];
        sprintf(versionString, "%02d.%02d", refSchema->GetVersionMajor(), refSchema->GetVersionMinor());
        m_xmlWriter.WriteAttribute(SCHEMAREF_VERSION_ATTRIBUTE, versionString);

        const Utf8String prefix = mapPair.second;
        m_xmlWriter.WriteAttribute(SCHEMAREF_PREFIX_ATTRIBUTE, prefix.c_str());
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
    return ecEnumeration._WriteXml(m_xmlWriter, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
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
SchemaWriteStatus SchemaXmlWriter::Serialize()
    {
    m_xmlWriter.WriteDocumentStart(XML_CHAR_ENCODING_UTF8);
    Utf8PrintfString ns("%s.%d.%d", ECXML_URI, m_ecXmlVersionMajor, m_ecXmlVersionMinor);
    m_xmlWriter.WriteElementStart(EC_SCHEMA_ELEMENT, ns.c_str());

    Utf8Char versionString[8];
    BeStringUtilities::Snprintf(versionString, "%02d.%02d", m_ecSchema.m_key.m_versionMajor, m_ecSchema.m_key.m_versionMinor);

    m_xmlWriter.WriteAttribute(SCHEMA_NAME_ATTRIBUTE, m_ecSchema.GetName().c_str());
    m_xmlWriter.WriteAttribute(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, m_ecSchema.GetNamespacePrefix().c_str());
    m_xmlWriter.WriteAttribute(SCHEMA_VERSION_ATTRIBUTE, versionString);
    m_xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, m_ecSchema.GetInvariantDescription().c_str());
    if (m_ecSchema.GetIsDisplayLabelDefined())
        m_xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, m_ecSchema.GetInvariantDisplayLabel().c_str());

    WriteSchemaReferences();

    WriteCustomAttributeDependencies(m_ecSchema);
    m_ecSchema.WriteCustomAttributes(m_xmlWriter);

    for (ECEnumerationCP pEnum : m_ecSchema.GetEnumerations())
        {
        if (NULL == pEnum)
            {
            BeAssert(false);
            continue;
            }
        else
            WriteEnumeration(*pEnum);
        }

    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are written is predictable.
    for (ECClassP pClass : m_ecSchema.GetClasses())
        {
        if (NULL == pClass)
            {
            BeAssert(false);
            continue;
            }
        else
            sortedClasses.push_back(pClass);
        }

    sortedClasses.sort(ClassNameComparer);

    for (ECClassP pClass : sortedClasses)
        {
        WriteClass(*pClass);
        }

    m_xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;

    }

END_BENTLEY_ECOBJECT_NAMESPACE

