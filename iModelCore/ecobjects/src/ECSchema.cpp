/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECSchema.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema
(
)
    {
    // NEEDSWORK make sure everything is destroyed
    Logger::GetLogger()->infov (L"~~~~ Destroying ECSchema: %s\n", GetName().c_str());
    ClassMap::iterator          classIterator = m_classMap.begin();
    ClassMap::const_iterator    classEnd = m_classMap.end();        
    Logger::GetLogger()->tracev (L"     Freeing memory for %d classes\n", m_classMap.size());
    while (classIterator != classEnd)
        {
        ECClassP ecClass = classIterator->second;        
        classIterator = m_classMap.erase(classIterator);        
        delete ecClass;
        }

    assert (m_classMap.empty());

    memset (this, 0xececdead, sizeof(this));
    /*
    for (ECSchemaReferenceVector::iterator sit = m_referencedSchemas.begin(); sit != m_referencedSchemas.end(); sit++)
        {
        CECSchemaReference & schemaRef = *sit;
        if (NULL != schemaRef.m_pECSchema)
            delete schemaRef.m_pECSchema; //needswork: are we sure that something else isn't holding it... we need a DgnECSchemaManager
        }
    m_referencedSchemas.clear();*/
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECSchema::GetName
(
) const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName
(
std::wstring const& name
)
    {        
    //NEEDSWORK name needs to be validated
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECSchema::GetNamespacePrefix
(
) const
    {        
    return m_namespacePrefix;    
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetNamespacePrefix
(
std::wstring const& namespacePrefix
)
    {        
    m_namespacePrefix = namespacePrefix;  
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECSchema::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDescription
(
std::wstring const& description
)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECSchema::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel
(
std::wstring const& displayLabel
)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECSchema::GetVersionMajor
(
) const
    {
    return m_versionMajor;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMajor
(
const UInt32 versionMajor
)
    {        
    m_versionMajor = versionMajor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECSchema::GetVersionMinor
(
) const
    {
    return m_versionMinor;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMinor
(
const UInt32 versionMinor
)
    {        
    m_versionMinor = versionMinor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECSchema::GetClassP
(
std::wstring const& name
) const
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name.c_str());
    
    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass 
(
ECClassP&                 pClass
)
    {
    std::pair < stdext::hash_map<const wchar_t *, ECClassP>::iterator, bool > resultPair;
    resultPair = m_classMap.insert (std::pair<const wchar_t *, ECClassP> (pClass->Name.c_str(), pClass));
    if (resultPair.second == false)
        {
        Logger::GetLogger()->warningv (L"Can not create class '%s' because it already exists in the schema", pClass->Name.c_str());
        delete pClass;
        pClass = NULL;        
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateClass 
(
ECClassP&                 pClass, 
std::wstring const&     name
)
    {
    pClass = new ECClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return AddClass (pClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass 
(
ECRelationshipClassP&     pClass, 
std::wstring const&     name
)
    {
    pClass = new ECRelationshipClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    std::pair < stdext::hash_map<const wchar_t *, ECClassP>::iterator, bool > resultPair;
    resultPair = m_classMap.insert (std::pair<const wchar_t *, ECClassP> (pClass->Name.c_str(), pClass));
    if (resultPair.second == false)
        {
        delete pClass;
        pClass = NULL;
        Logger::GetLogger()->warningv (L"Can not create relationship class '%s' because it already exists in the schema", name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLAINATION L" Format must be MM.mm where MM is major version and mm is minor version.\n"
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString 
(
UInt32&                 versionMajor, 
UInt32&                 versionMinor, 
std::wstring const&     versionString
)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (versionString.empty())
        return ECOBJECTS_STATUS_Success;

    const wchar_t * version = versionString.c_str();
    const wchar_t * theDot = wcschr (version, L'.');
    if (NULL == theDot)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t majorLen = theDot - version;
    if (majorLen < 1 || majorLen > 3)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t minorLen = wcslen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    wchar_t * end = NULL;    
    versionMajor = wcstoul (version, &end, 10);
    if (version == end)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    versionMinor = wcstoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString 
(
std::wstring const& versionString
)
    {
    UInt32 versionMajor;
    UInt32 versionMinor;
    ECObjectsStatus status;
    if ((ECOBJECTS_STATUS_Success != (status = ParseVersionString (versionMajor, versionMinor, versionString))) ||         
        (ECOBJECTS_STATUS_Success != (status = this->SetVersionMajor (versionMajor))) ||
        (ECOBJECTS_STATUS_Success != (status = this->SetVersionMinor (versionMinor))))
        return status;
    else
        return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema
(
ECSchemaPtr&              schemaOut, 
std::wstring const&     schemaName
)
    {    
    schemaOut = new ECSchema();
    return schemaOut->SetName (schemaName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::GetSchemaByNamespacePrefixP
(
std::wstring const&     namespacePrefix
) const
    {
    if ((namespacePrefix.length() == 0) || (namespacePrefix == m_namespacePrefix))
        return (ECSchemaP)this;

    // NEEDSWORK lookup referenced schema by prefix

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const* ECSchema::ResolveNamespacePrefix
(
ECSchemaCR    schema
) const
    {
    if (&schema == this)
        return &EMPTY_STRING;

    // NEEDSWORK support referenced schemas

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadClassStubsFromXml
(
MSXML2::IXMLDOMNode& schemaNode,
ClassDeserializationVector&  classes
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    // Create ECClass Stubs (no attributes or properties)
    size_t classElementLength = wcslen (EC_CLASS_ELEMENT);
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = schemaNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_CLASS_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_RELATIONSHIP_CLASS_ELEMENT);
    _bstr_t baseName;    
    ECClassP pClass;
    ECRelationshipClassP pRelationshipClass;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {        
        baseName = xmlNodePtr->baseName;
        
        if (baseName.length() == classElementLength)
            {            
            pClass = new ECClass (*this);
            pRelationshipClass = NULL;
            }
        else
            {            
            pRelationshipClass = new ECRelationshipClass (*this);            
            pClass = pRelationshipClass;
            }

        if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = pClass->ReadXmlAttributes(xmlNodePtr)))
            return status;           

        if (ECOBJECTS_STATUS_Success != this->AddClass (pClass))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;

        if (NULL == pRelationshipClass)
            Logger::GetLogger()->tracev (L"    Created ECClass Stub: %s\n", pClass->Name.c_str());
        else
            Logger::GetLogger()->tracev (L"    Created Relationship ECClass Stub: %s\n", pClass->Name.c_str());

        classes.push_back (std::make_pair (pClass, xmlNodePtr));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 - Expects class stubs have already been read and created.  They are stored in the vector passed into this method.
 - Expects referenced schemas have been resolved and deserialized so that base classes & structs in other schemas can be located.
 - Reads the contents of each XML node cached in the classes vector and populates the in-memory EC:ECClass with
   base classes, properties & relationship endpoints.
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadClassContentsFromXml
(
ClassDeserializationVector&  classes
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP pClass;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    for(classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        pClass = classesIterator->first;
        xmlNodePtr = classesIterator->second;
        status = pClass->ReadXmlContents (xmlNodePtr);
        if (SCHEMA_DESERIALIZATION_STATUS_Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXml
(
ECSchemaPtr&                        schemaOut, 
MSXML2::IXMLDOMDocument2&           pXmlDoc
)
    {            
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;
    
    pXmlDoc.setProperty("SelectionNamespaces", L"xmlns:" EC_NAMESPACE_PREFIX L"='" ECXML_URI_2_0 L"'");
    MSXML2::IXMLDOMNodePtr xmlNodePtr = pXmlDoc.selectSingleNode (L"/" EC_NAMESPACE_PREFIX L":" EC_SCHEMA_ELEMENT);
    if (NULL == xmlNodePtr)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: Missing a top-level " EC_SCHEMA_ELEMENT L" node in the " ECXML_URI_2_0 L" namespace\n");
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }
    
    MSXML2::IXMLDOMNodePtr schemaNodePtr = xmlNodePtr;       
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = schemaNodePtr->attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
    
    // schemaName is a REQUIRED attribute in order to create the schema
    if ((NULL == nodeAttributesPtr) || (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMA_NAME_ATTRIBUTE))))
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: " EC_SCHEMA_ELEMENT L" element must contain a schemaName attribute\n");
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }

    if (ECOBJECTS_STATUS_Success != CreateSchema (schemaOut, (const wchar_t *)attributePtr->text))
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;

    // OPTIONAL attributes - If these attributes exist they MUST be valid        
    READ_OPTIONAL_XML_ATTRIBUTE (SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (SCHEMA_VERSION_ATTRIBUTE,                  schemaOut, VersionFromString)
    if (ECOBJECTS_STATUS_Success != setterStatus)
        Logger::GetLogger()->warningv (L"Invalid version attribute has been ignored while deserializing ECSchema '%s'.  The default version number %d.%d has been applied.\n", 
            schemaOut->Name.c_str(), schemaOut->VersionMajor, schemaOut->VersionMinor);

    ClassDeserializationVector classes;
    if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = schemaOut->ReadClassStubsFromXml (schemaNodePtr, classes)))
        return status;

    // NEEDSWORK Find and deserialize referenced schemas

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = schemaOut->ReadClassContentsFromXml (classes)))
        return status;

    // NEEDSWORK Custom attributes (schema, classes, properties, etc)

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXml
(
MSXML2::IXMLDOMDocument2* pXmlDoc
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMProcessingInstructionPtr piPtr = NULL;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;

    piPtr = pXmlDoc->createProcessingInstruction(L"xml", L"version='1.0'  encoding='UTF-8'");
    APPEND_CHILD_TO_PARENT(piPtr, pXmlDoc);
    //CREATE_AND_ADD_TEXT_NODE(L"\n", piPtr);
    
    MSXML2::IXMLDOMElementPtr schemaElementPtr = pXmlDoc->createNode(NODE_ELEMENT, EC_SCHEMA_ELEMENT, ECXML_URI_2_0);
    APPEND_CHILD_TO_PARENT(schemaElementPtr, pXmlDoc);
    
    MSXML2::IXMLDOMAttributePtr attributePtr;
    WRITE_XML_ATTRIBUTE(L"xmlns:" EC_NAMESPACE_PREFIX, ECXML_URI_2_0, schemaElementPtr);
    WRITE_XML_ATTRIBUTE(SCHEMA_NAME_ATTRIBUTE, this->Name.c_str(), schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, NamespacePrefix, schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, schemaElementPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, schemaElementPtr);
    
    wchar_t versionString[8];
    swprintf(versionString, 8, L"%02d.%02d", m_versionMajor, m_versionMinor);
    WRITE_XML_ATTRIBUTE(SCHEMA_VERSION_ATTRIBUTE, versionString, schemaElementPtr);

    // NEEDSWORK Serialize schema references
    
    // NEEDSWORK Serialize custom attributes
    
    std::vector<std::wstring> alreadySerializedClasses;
    // sort the classes by name so the order in which they are serialized is predictable.
    
    for each (ECClassP pClass in Classes)
        {
        // NEEDSWORK Make sure haven't already serialized this class
        pClass->WriteXml(schemaElementPtr);
        }
        
    CREATE_AND_ADD_TEXT_NODE(L"\n", schemaElementPtr);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LogXmlLoadError
(
MSXML2::IXMLDOMDocument2& pXmlDoc
)
    {        
    MSXML2::IXMLDOMParseErrorPtr pXMLError = pXmlDoc.parseError;
    if (pXMLError != NULL)
        {
        LONG lErrorCode = pXMLError->errorCode;
        if (lErrorCode != 0)
            {
            long line = pXMLError->Getline();
            long linePos = pXMLError->Getlinepos();
            _bstr_t pBURL = pXMLError->Geturl();
            _bstr_t pBReason = pXMLError->Getreason();

            std::wstring file;
            if (NULL != pBURL.GetBSTR())
                file = pBURL;
                
            std::wstring reason = pBReason;
                        
            Logger::GetLogger()->errorv (L"line %d, position %d parsing ECSchema file %s. %s\n", line, linePos, file.c_str(), reason.c_str());            
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXmlFromFile
(
ECSchemaPtr&          schemaOut, 
const wchar_t *     ecSchemaXmlFile
)
    {                  
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlFile);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to deserialize XML file: %s\n", ecSchemaXmlFile);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXmlFromString
(
ECSchemaPtr&          schemaOut, 
const wchar_t *     ecSchemaXml
)
    {                  
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->loadXML(ecSchemaXml);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to deserialize XML from string: %s\n", ecSchemaXml);
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
//SchemaDeserializationStatus ECSchema::ReadXmlFromStream
//(
//ECSchemaPtr&          schemaOut, 
//IStream *           ecSchemaXmlStream
//)
//    {                  
//    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;
//
//    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
//    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
//    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
//    xmlDocPtr->put_async(VARIANT_FALSE);
//    
//    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
//    if (returnCode != VARIANT_TRUE)
//        {
//        LogXmlLoadError(xmlDocPtr);
//        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
//        }
//
//    status = ReadXml (schemaOut, xmlDocPtr);
//    if (ECOBJECTS_STATUS_Success != status)
//        Logger::GetLogger()->errorv (L"Failed to deserialize XML from stream\n");
//    return status;
//    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXmlToString
(
const wchar_t*  &ecSchemaXml
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    ecSchemaXml = xmlDocPtr->xml;
    
    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXmlToFile
(
const wchar_t * ecSchemaXmlFile
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlFile), SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml);

    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
//SchemaSerializationStatus ECSchema::WriteXmlToStream
//(
//IStream * ecSchemaXmlStream
//)
//    {
//    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
//
//    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
//    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
//    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
//    xmlDocPtr->put_async(VARIANT_FALSE);
//    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
//    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
//    
//    status = WriteXml(xmlDocPtr);
//    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
//        return status;
//        
//    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlStream), SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml);
//
//    return status;
//    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainerCR ECSchema::GetClasses
(
) const
    {
    return m_classContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator  ECClassContainer::begin () const
    {
    return ECClassContainer::const_iterator(m_classMap.begin());        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator  ECClassContainer::end () const
    {
    return ECClassContainer::const_iterator(m_classMap.end());        
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator& ECClassContainer::const_iterator::operator++()
    {
    m_state->m_mapIterator++;    
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECClassContainer::const_iterator::operator!= (const_iterator const& rhs) const
    {
    return (m_state->m_mapIterator != rhs.m_state->m_mapIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP   ECClassContainer::const_iterator::operator*() const
    {
    std::pair<const wchar_t * , ECClassP> mapPair = *(m_state->m_mapIterator);
    return mapPair.second;
    };

END_BENTLEY_EC_NAMESPACE
