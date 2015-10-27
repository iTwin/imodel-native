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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassNameComparer (ECClassP class1, ECClassP class2)
    {
    // We should never have a NULL ECClass here.
    // However we will pretend a NULL ECClass is always less than a non-NULL ECClass
    BeAssert (NULL != class1 && NULL != class2);
    if (NULL == class1)
        return NULL != class2;      // class 1 < class2 if class2 non-null, equal otherwise
    else if (NULL == class2)
        return false;               // class1 > class2

    int comparison = class1->GetName().CompareTo (class2->GetName());
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
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlReader::SchemaXmlReader(ECSchemaReadContextR context, BeXmlDomR xmlDom) : m_schemaContext(context), m_xmlDom(xmlDom)
    {

    }

//---------------------------------------------------------------------------------------
// - OpenPlant shipped a malformed schema that has a circular reference through supplementation.
// - Therefore a special case had to be created so that we do not try to de-serialize this
// - schema
// @bsimethod                                    Carole.MacDonald                01/2012
//---------------+---------------+---------------+---------------+---------------+-------
bool  SchemaXmlReader::IsOpenPlantPidCircularReferenceSpecialCase
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
SchemaReadStatus SchemaXmlReader::ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    // m_referencedSchemaNamespaceMap.clear();

    BeXmlDom::IterableNodeSet schemaReferenceNodes;
    schemaNode.SelectChildNodes (schemaReferenceNodes, EC_NAMESPACE_PREFIX ":" EC_SCHEMAREFERENCE_ELEMENT);
    for (BeXmlNodeP& schemaReferenceNode: schemaReferenceNodes)
        {
        SchemaKey key;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (key.m_schemaName, SCHEMAREF_NAME_ATTRIBUTE))
            {
            LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_NAME_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        Utf8String prefix;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (prefix, SCHEMAREF_PREFIX_ATTRIBUTE))
            {
            LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_PREFIX_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }


        Utf8String versionString;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (versionString, SCHEMAREF_VERSION_ATTRIBUTE))
            {
            LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", schemaReferenceNode->GetName(), SCHEMAREF_VERSION_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        if (ECOBJECTS_STATUS_Success != ECSchema::ParseVersionString (key.m_versionMajor, key.m_versionMinor, versionString.c_str()))
            {
            LOG.errorv ("Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", key.m_schemaName.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        // If the schema (uselessly) references itself, just skip it
        if (schemaOut->m_key.m_schemaName.compare(key.m_schemaName) == 0)
            continue;

        Utf8String schemaFullName = schemaOut->GetFullSchemaName();
        if (IsOpenPlantPidCircularReferenceSpecialCase(key.m_schemaName, schemaFullName))
            continue;

        LOG.debugv ("About to locate referenced ECSchema %s", key.GetFullSchemaName().c_str());

        ECSchemaPtr referencedSchema = schemaOut->LocateSchema (key, m_schemaContext);

        if (referencedSchema.IsValid())
            {
            //We can encounter some time same schema referenced twice with different namespacePrefix.
            //We will not treat it as error.
            SchemaKeyCR refSchemaKey = referencedSchema->GetSchemaKey ();
            auto const& references = schemaOut->GetReferencedSchemas ();
            if (references.end () != references.find (refSchemaKey))
                {
                continue;
                }

            ECObjectsStatus status = schemaOut->AddReferencedSchema (*referencedSchema, prefix, m_schemaContext);
            if (ECOBJECTS_STATUS_Success != status)
                return ECOBJECTS_STATUS_SchemaHasReferenceCycle == status ? SCHEMA_READ_STATUS_HasReferenceCycle : static_cast<SchemaReadStatus> (status);
            }
        else
            {
            LOG.errorv("Unable to locate referenced schema %s", key.GetFullSchemaName().c_str());
            return SCHEMA_READ_STATUS_ReferencedSchemaNotFound;
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader::ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    // Create ECClass Stubs (no attributes or properties)
    for (BeXmlNodeP classNode = schemaNode.GetFirstChild (); NULL != classNode; classNode = classNode->GetNextSibling ())
        {
        ECClassP                ecClass;
        ECRelationshipClassP    ecRelationshipClass;
        Utf8CP nodeName = classNode->GetName ();

        if (0 == strcmp (nodeName, EC_CLASS_ELEMENT))
            {
            ecClass = new ECClass (*schemaOut);
            ecRelationshipClass = NULL;
            }
        else if (0 == strcmp (nodeName, EC_RELATIONSHIP_CLASS_ELEMENT))
            {
            ecRelationshipClass = new ECRelationshipClass (*schemaOut);
            ecClass = ecRelationshipClass;
            }
        else
            continue;

        if (SCHEMA_READ_STATUS_Success != (status = ecClass->_ReadXmlAttributes (*classNode)))
            {
            delete ecClass;
            return status;
            }

        ECClassP existingClass = schemaOut->GetClassP (ecClass->GetName().c_str());

        if (NULL != existingClass)
            {
            existingClass->_ReadXmlAttributes (*classNode);
            delete ecClass;
            ecClass = existingClass;
            }
        else if (ECOBJECTS_STATUS_Success != schemaOut->AddClass (ecClass))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

        if (NULL == ecRelationshipClass)
            LOG.tracev ("    Created ECClass Stub: %s", ecClass->GetName().c_str());
        else
            LOG.tracev ("    Created Relationship ECClass Stub: %s", ecClass->GetName().c_str());

        classes.push_back (make_bpair (ecClass, classNode));
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
SchemaReadStatus SchemaXmlReader::ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP    ecClass;
    BeXmlNodeP  classNode;
    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass     = classesIterator->first;
        classNode   = classesIterator->second;
        status = ecClass->_ReadXmlContents (*classNode, m_schemaContext);
        if (SCHEMA_READ_STATUS_Success != status)
            return status;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus SchemaXmlReader::Deserialize(ECSchemaPtr& schemaOut, uint32_t checkSum)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
    StopWatch overallTimer(L"Overall schema de-serialization timer", true);

    m_xmlDom.RegisterNamespace (EC_NAMESPACE_PREFIX, ECXML_URI_2_0);

    BeXmlNodeP      schemaNode;
    if ( (BEXML_Success != m_xmlDom.SelectNode (schemaNode, "/" EC_NAMESPACE_PREFIX ":" EC_SCHEMA_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == schemaNode) )
        {
        BeAssert (s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: Missing a top-level %s node in the %s namespace", EC_SCHEMA_ELEMENT, ECXML_URI_2_0);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    // schemaName is a REQUIRED attribute in order to create the schema
    Utf8String schemaName;
    if (BEXML_Success != schemaNode->GetAttributeStringValue (schemaName, SCHEMA_NAME_ATTRIBUTE))
        {
        BeAssert (s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a schemaName attribute", EC_SCHEMA_ELEMENT);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    uint32_t versionMajor = DEFAULT_VERSION_MAJOR;
    uint32_t versionMinor = DEFAULT_VERSION_MINOR;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    Utf8String     versionString;
    if ( (BEXML_Success != schemaNode->GetAttributeStringValue (versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
        (ECOBJECTS_STATUS_Success != ECSchema::ParseVersionString (versionMajor, versionMinor, versionString.c_str())) )
        {
        LOG.warningv ("Invalid version attribute has been ignored while reading ECSchema '%s'.  The default version number %02d.%02d has been applied.",
            schemaName.c_str(), versionMajor, versionMinor);
        }

    LOG.debugv ("Reading ECSchema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);

    ECObjectsStatus createStatus = ECSchema::CreateSchema (schemaOut, schemaName, versionMajor, versionMinor);
    if (ECOBJECTS_STATUS_Success != createStatus)
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

    schemaOut->m_key.m_checkSum = checkSum;

    if (ECOBJECTS_STATUS_DuplicateSchema == m_schemaContext.AddSchema (*schemaOut))
        {
        return SCHEMA_READ_STATUS_DuplicateSchema;
        }

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    Utf8String value;  // used by macro.
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    StopWatch readingSchemaReferences(L"Reading Schema References", true);
    if (SCHEMA_READ_STATUS_Success != (status = ReadSchemaReferencesFromXml (schemaOut, *schemaNode)))
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }

    readingSchemaReferences.Stop();
    LOG.tracev("Reading schema references for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingSchemaReferences.GetElapsedSeconds());

    ClassDeserializationVector classes;
    StopWatch readingClassStubs(L"Reading class stubs", true);
    if (SCHEMA_READ_STATUS_Success != (status = ReadClassStubsFromXml (schemaOut, *schemaNode, classes)))
        {
        m_schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }
    readingClassStubs.Stop();
    LOG.tracev("Reading class stubs for %s took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassStubs.GetElapsedSeconds());

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    StopWatch readingClassContents(L"Reading class contents", true);
    if (SCHEMA_READ_STATUS_Success != (status = ReadClassContentsFromXml (schemaOut, classes)))
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

    return SCHEMA_READ_STATUS_Success;
    }

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaXmlWriter::SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema) : m_xmlWriter(xmlWriter), m_ecSchema(ecSchema)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteSchemaReferences()
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    bmap<ECSchemaP, Utf8String>::const_iterator iterator;
    for (iterator = m_ecSchema.m_referencedSchemaNamespaceMap.begin(); iterator != m_ecSchema.m_referencedSchemaNamespaceMap.end(); iterator++)
        {
        bpair<ECSchemaP, const Utf8String> mapPair = *(iterator);
        ECSchemaP   refSchema           = mapPair.first;
        m_xmlWriter.WriteElementStart(EC_SCHEMAREFERENCE_ELEMENT);
        m_xmlWriter.WriteAttribute(SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str());

        Utf8Char versionString[8];
        sprintf(versionString, "%02d.%02d", refSchema->GetVersionMajor(), refSchema->GetVersionMinor());
        m_xmlWriter.WriteAttribute (SCHEMAREF_VERSION_ATTRIBUTE, versionString);

        const Utf8String prefix = mapPair.second;
        m_xmlWriter.WriteAttribute(SCHEMAREF_PREFIX_ATTRIBUTE, prefix.c_str());
        m_xmlWriter.WriteElementEnd();
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteCustomAttributeDependencies (IECCustomAttributeContainerCR container)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    for (IECInstancePtr instance: container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass (currentClass);
        if (SCHEMA_WRITE_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WriteClass (ECClassCR ecClass)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
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
    for (ECClassP baseClass: ecClass.GetBaseClasses())
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

    ecClass._WriteXml (m_xmlWriter);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus SchemaXmlWriter::WritePropertyDependencies (ECClassCR ecClass)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    for (ECPropertyP prop: ecClass.GetProperties(false))
        {
        if (prop->GetIsStruct())
            {
            StructECPropertyP structProperty = prop->GetAsStructPropertyP();
            WriteClass(structProperty->GetType());
            }
        else if (prop->GetIsArray())
            {
            ArrayECPropertyP arrayProperty = prop->GetAsArrayPropertyP();
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                WriteClass(*(arrayProperty->GetStructElementType()));
                }
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
    m_xmlWriter.WriteElementStart(EC_SCHEMA_ELEMENT, ECXML_URI_2_0);

    Utf8Char versionString[8];
    BeStringUtilities::Snprintf (versionString, "%02d.%02d", m_ecSchema.m_key.m_versionMajor, m_ecSchema.m_key.m_versionMinor);

    m_xmlWriter.WriteAttribute(SCHEMA_NAME_ATTRIBUTE, m_ecSchema.GetName().c_str());
    m_xmlWriter.WriteAttribute(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, m_ecSchema.GetNamespacePrefix().c_str());
    m_xmlWriter.WriteAttribute(SCHEMA_VERSION_ATTRIBUTE, versionString);
    m_xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, m_ecSchema.GetInvariantDescription().c_str());
    if (m_ecSchema.GetIsDisplayLabelDefined())
        m_xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, m_ecSchema.GetInvariantDisplayLabel().c_str());

    WriteSchemaReferences ();

    WriteCustomAttributeDependencies(m_ecSchema);
    m_ecSchema.WriteCustomAttributes(m_xmlWriter);

    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are written is predictable.
    for (ECClassP pClass: m_ecSchema.GetClasses())
        {
        if (NULL == pClass)
            {
            BeAssert(false);
            continue;
            }
        else
            sortedClasses.push_back(pClass);
        }

    sortedClasses.sort (ClassNameComparer);

    for (ECClassP pClass: sortedClasses)
        {
        WriteClass (*pClass);
        }

    m_xmlWriter.WriteElementEnd();
    return SCHEMA_WRITE_STATUS_Success;

    }

END_BENTLEY_ECOBJECT_NAMESPACE

