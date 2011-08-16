/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchema.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <list>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_EC_NAMESPACE

extern ECObjectsStatus GetSchemaFileName (WString& fullFileName, UInt32& foundVersionMinor, WCharCP schemaPath, bool useLatestCompatibleMatch);

//#define DEBUG_SCHEMA_LEAKS
#ifdef DEBUG_SCHEMA_LEAKS
static LeakDetector<ECSchema> g_leakDetector (L"ECSchema", L"ECSchemas", true);
#else
static LeakDetector<ECSchema> g_leakDetector (L"ECSchema", L"ECSchemas", false);
#endif

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::ECSchema (bool hideFromLeakDetection)
    :
    m_versionMajor (DEFAULT_VERSION_MAJOR), m_versionMinor (DEFAULT_VERSION_MINOR), m_classContainer(m_classMap),
    m_hideFromLeakDetection(hideFromLeakDetection)
    {
    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectCreated(*this);
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema ()
    {
    // NEEDSWORK make sure everything is destroyed
    ECObjectsLogger::Log()->debugv (L"~~~~ Destroying ECSchema: %s", GetName().c_str());
    ClassMap::iterator          classIterator = m_classMap.begin();
    ClassMap::const_iterator    classEnd = m_classMap.end();        
    ECObjectsLogger::Log()->debugv(L"     Freeing memory for %d classes", m_classMap.size());
    while (classIterator != classEnd)
        {
        ECClassP ecClass = classIterator->second;
        ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(ecClass);
        classIterator = m_classMap.erase(classIterator);
        if (NULL != relClass)
            delete relClass;
        else
            delete ecClass;
        }

    assert (m_classMap.empty());

    /*
    for (ECSchemaReferenceVector::iterator sit = m_referencedSchemas.begin(); sit != m_referencedSchemas.end(); sit++)
        {
        CECSchemaReference & schemaRef = *sit;
        if (NULL != schemaRef.m_pECSchema)
            delete schemaRef.m_pECSchema; //needswork: are we sure that something else isn't holding it... we need a DgnECManager
        }
    m_referencedSchemas.clear();*/

    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectDestroyed(*this);

    memset (this, 0xececdead, sizeof(this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILeakDetector&  ECSchema::Debug_GetLeakDetector() { return g_leakDetector; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::_GetContainerSchema() const
        {
        return this;
        }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetName
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
WStringCR name
)
    {        
    if (!NameValidator::Validate(name))
        return ECOBJECTS_STATUS_InvalidName;

    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetNamespacePrefix
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
WStringCR namespacePrefix
)
    {        
    m_namespacePrefix = namespacePrefix;  
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDescription
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
WStringCR description
)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? GetName() : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel
(
WStringCR displayLabel
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
WCharCP name
) const
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name);
    
    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

void ECSchema::DebugDump()const
    {
    wprintf(L"ECSchema: this=0x%x  %s.%d.%d, nClasses=%d\n", this, m_name.c_str(), m_versionMajor, m_versionMinor, m_classMap.size());
    for (ClassMap::const_iterator it = m_classMap.begin(); it != m_classMap.end(); ++it)
        {
        bpair<WCharCP, ECClassP>const& entry = *it;
        ECClassCP ecClass = entry.second;
        wprintf(L"    ECClass: 0x%x, %s\n", ecClass, ecClass->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass 
(
ECClassP&                 pClass
)
    {
    bpair <ClassMap::iterator, bool> resultPair;
    resultPair = m_classMap.insert (bpair<WCharCP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        ECObjectsLogger::Log()->warningv (L"Can not create class '%s' because it already exists in the schema", pClass->GetName().c_str());
        delete pClass;
        pClass = NULL;        
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
    //DebugDump(); wprintf(L"\n");
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateClass 
(
ECClassP&                 pClass, 
WStringCR     name
)
    {
    pClass = new ECClass(*this, m_hideFromLeakDetection);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    return AddClass (pClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass 
(
ECRelationshipClassP&     pClass, 
WStringCR     name
)
    {
    pClass = new ECRelationshipClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    bpair < ClassMap::iterator, bool > resultPair;
    resultPair = m_classMap.insert (bpair<WCharCP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        delete pClass;
        pClass = NULL;
        ECObjectsLogger::Log()->warningv (L"Can not create relationship class '%s' because it already exists in the schema", name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECOBJECTS_EXPORT WString ECSchema::GetFullSchemaName () const
    {
#if defined (_WIN32) // WIP_NONPORT
    std::wostringstream fullName;
    fullName << GetName().c_str() << L"." << std::setfill(L'0') << std::setw(2) << GetVersionMajor() << L"." << std::setw(2) << GetVersionMinor();
    
    return fullName.str().c_str();
#elif defined (__unix__)
    // *** NEEDS WORK: iostreams not support on Android
    return L"";
#endif
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName
(
WString&           schemaName,
UInt32&             versionMajor, 
UInt32&             versionMinor, 
WStringCR     fullName
)
    {
    if (fullName.empty())
        return ECOBJECTS_STATUS_ParseError;

    WCharCP fullNameCP = fullName.c_str();
    WCharCP firstDot = wcschr (fullNameCP, L'.');
    if (NULL == firstDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullNameCP;
    if (nameLen < 1)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullNameCP, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName
(
WString&           schemaName,
UInt32&             versionMajor, 
UInt32&             versionMinor, 
WCharCP      fullName
)
    {
    if (NULL == fullName || '\0' == *fullName)
        return ECOBJECTS_STATUS_ParseError;

    WCharCP firstDot = wcschr (fullName, L'.');
    if (NULL == firstDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullName;
    if (nameLen < 1)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullName, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECSchema::FormatSchemaVersion (UInt32& versionMajor, UInt32& versionMinor)
    {
    wchar_t versionString[80];
    BeStringUtilities::Snwprintf (versionString, _countof(versionString), L"%02d.%02d", versionMajor, versionMinor);
    return WString (versionString);
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLAINATION L" Format must be MM.mm where MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString 
(
UInt32&             versionMajor, 
UInt32&             versionMinor, 
WCharCP      versionString
)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (NULL == versionString || '\0' == *versionString)
        return ECOBJECTS_STATUS_Success;

    WCharCP theDot = wcschr (versionString, L'.');
    if (NULL == theDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t majorLen = theDot - versionString;
    if (majorLen < 1 || majorLen > 3)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t minorLen = wcslen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharP end = NULL;    
    UInt32    localMajor = wcstoul (versionString, &end, 10);
    if (versionString == end)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        {
        versionMajor = localMajor;
        }

    UInt32 localMinor = wcstoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        {
        versionMinor = localMinor;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString 
(
WCharCP  versionString
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
* @bsimethod                                                    Casey.Mullen   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchema::SchemasMatch
(
SchemaMatchType matchType,
WCharCP soughtName,
UInt32          soughtMajor,
UInt32          soughtMinor,
WCharCP candidateName,
UInt32          candidateMajor,
UInt32          candidateMinor
)
    {
    if (BeStringUtilities::Wcsicmp (candidateName, soughtName))
        return false;
    
    return ((matchType == SCHEMAMATCHTYPE_Latest) ||
            (matchType == SCHEMAMATCHTYPE_LatestCompatible && candidateMajor == soughtMajor && candidateMinor >= soughtMinor) ||
            (matchType == SCHEMAMATCHTYPE_Exact            && candidateMajor == soughtMajor && candidateMinor == soughtMinor));
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema
(
ECSchemaP&          schemaOut, 
WStringCR           schemaName,
UInt32              versionMajor,
UInt32              versionMinor,
IECSchemaOwnerR     schemaOwner,
bool                hideFromLeakDetection
)
    {    
    if (!NameValidator::Validate(schemaName))
        return ECOBJECTS_STATUS_InvalidName;

    ECSchemaP   schema = new ECSchema(hideFromLeakDetection);

    ECObjectsStatus status;
    
    if (SUCCESS != (status = schema->SetName (schemaName)) ||
        SUCCESS != (status = schema->SetVersionMajor (versionMajor)) ||
        SUCCESS != (status = schema->SetVersionMinor (versionMinor)))
        {
        delete schema;
        return status;
        }

    if (SUCCESS != (status = schemaOwner.AddSchema (*schema)))
        {
        delete schema;
        return status;
        }

    schemaOut = schema;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema
(
ECSchemaP&          schemaOut, 
WStringCR     schemaName,
UInt32              versionMajor,
UInt32              versionMinor,
IECSchemaOwnerR     schemaOwner
)
    {
    return CreateSchema (schemaOut, schemaName, versionMajor, versionMinor, schemaOwner, false);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECSchema::DestroySchema (ECSchemaP& schema)
    {
    delete schema;
    schema = NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::GetSchemaByNamespacePrefixP
(
WStringCR     namespacePrefix
) const
    {
    if ((namespacePrefix.length() == 0) || (namespacePrefix == m_namespacePrefix))
        return (ECSchemaP)this;

    // lookup referenced schema by prefix
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (0 == namespacePrefix.compare ((*schemaIterator)->m_namespacePrefix))
            return *schemaIterator;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ResolveNamespacePrefix
(
ECSchemaCR    schema,
WString &    namespacePrefix
) const
    {
    namespacePrefix = EMPTY_STRING;
    if (&schema == this)
        return ECOBJECTS_STATUS_Success;

    bmap<ECSchemaP, WString const>::const_iterator schemaIterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &schema);
    if (schemaIterator != m_referencedSchemaNamespaceMap.end())
        {
        namespacePrefix = schemaIterator->second;
        return ECOBJECTS_STATUS_Success;
        }

    return ECOBJECTS_STATUS_SchemaNotFound;
    }

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
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const ECSchemaReferenceList& ECSchema::GetReferencedSchemas
(
) const
    {
    return m_refSchemaList;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema
(
ECSchemaR       refSchema
)
    {
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == &refSchema)
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
    
    // Check for recursion
    m_refSchemaList.push_back(&refSchema);
    m_referencedSchemaNamespaceMap.insert(bpair<ECSchemaP, const WString> (&refSchema, refSchema.GetNamespacePrefix()));
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema
(
ECSchemaR       refSchema
)
    {
    ECSchemaReferenceList::iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == &refSchema)
            {
            // We need to verify that nothing references the schema before we can remove it.
            break;
            }
        }
    if (m_refSchemaList.end() == schemaIterator)
        return ECOBJECTS_STATUS_SchemaNotFound;
        
    // Can only remove the reference if nothing actually references it.
    
    ECSchemaP foundSchemaP = *schemaIterator;
    FOR_EACH (ECClassP ecClass, GetClasses())
        {
        // First, check each base class to see if the base class uses that schema
        FOR_EACH (ECClassP baseClass, ecClass->GetBaseClasses())
            {
            if ((ECSchemaP) &(baseClass->GetSchema()) == foundSchemaP)
                {
                return ECOBJECTS_STATUS_SchemaInUse;
                }
            }
            
        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(ecClass);
        if (NULL != relClass)
            {
            FOR_EACH (ECClassP target, relClass->GetTarget().GetClasses())
                {
                if ((ECSchemaP) &(target->GetSchema()) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            FOR_EACH (ECClassP source, relClass->GetSource().GetClasses())
                {
                if ((ECSchemaP) &(source->GetSchema()) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            }
            
        // And make sure that there are no struct types from another schema
        FOR_EACH (ECPropertyP prop, ecClass->GetProperties(false))
            {
            ECClassCP typeClass;
            if (prop->GetIsStruct())
                {
                typeClass = &(prop->GetAsStructProperty()->GetType());
                }
            else if (prop->GetIsArray())
                {
                typeClass = prop->GetAsArrayProperty()->GetStructElementType();
                }
            else
                {
                typeClass = NULL;
                }
            if (NULL == typeClass)
                continue;
            if (this->GetName().compare(typeClass->GetSchema().GetName()) == 0 && this->GetVersionMajor() == typeClass->GetSchema().GetVersionMajor() &&
                this->GetVersionMinor() == typeClass->GetSchema().GetVersionMinor())
                continue;
            return ECOBJECTS_STATUS_SchemaInUse;
            }
        }

    m_refSchemaList.erase(schemaIterator); 
    bmap<ECSchemaP, const WString>::iterator iterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &refSchema);
    if (iterator != m_referencedSchemaNamespaceMap.end())
        {
        m_referencedSchemaNamespaceMap.erase(iterator);
        }

    return ECOBJECTS_STATUS_Success;
    }
    
#if defined (_WIN32) // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadClassStubsFromXml
(
MSXML2::IXMLDOMNode&                schemaNode,
ClassDeserializationVector&         classes, 
ECSchemaDeserializationContextR     schemaContext
)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

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
            pClass = new ECClass (*this, m_hideFromLeakDetection);
            pRelationshipClass = NULL;
            }
        else
            {            
            pRelationshipClass = new ECRelationshipClass (*this);            
            pClass = pRelationshipClass;
            }

        if (SCHEMA_READ_STATUS_Success != (status = pClass->ReadXmlAttributes(xmlNodePtr, schemaContext.GetStandaloneEnablerLocater())))
            {
            delete pClass;
            return status;           
            }

        ECClassP existingClass = this->GetClassP(pClass->GetName().c_str());

        if (NULL != existingClass)
            {
            existingClass->ReadXmlAttributes(xmlNodePtr, schemaContext.GetStandaloneEnablerLocater());
            delete pClass;
            pClass = existingClass;
            }
        else if (ECOBJECTS_STATUS_Success != this->AddClass (pClass))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

        if (NULL == pRelationshipClass)
            ECObjectsLogger::Log()->tracev (L"    Created ECClass Stub: %s", pClass->GetName().c_str());
        else
            ECObjectsLogger::Log()->tracev (L"    Created Relationship ECClass Stub: %s", pClass->GetName().c_str());

        classes.push_back (make_bpair (pClass, xmlNodePtr));
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
SchemaReadStatus ECSchema::ReadClassContentsFromXml
(
ClassDeserializationVector&         classes, 
ECSchemaDeserializationContextR     schemaContext
)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP pClass;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    for(classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        pClass = classesIterator->first;
        xmlNodePtr = classesIterator->second;
        status = pClass->ReadXmlContents (xmlNodePtr, schemaContext.GetStandaloneEnablerLocater());
        if (SCHEMA_READ_STATUS_Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadSchemaReferencesFromXml
(
MSXML2::IXMLDOMNode&            schemaNode, 
ECSchemaDeserializationContextR schemaContext
)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
        
    m_referencedSchemaNamespaceMap.clear();

    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = schemaNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_SCHEMAREFERENCE_ELEMENT);
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {
        MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = xmlNodePtr->attributes;
        MSXML2::IXMLDOMNodePtr attributePtr;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_NAME_ATTRIBUTE)))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_NAME_ATTRIBUTE L" attribute", (WCharCP)xmlNodePtr->baseName);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
            
        WString schemaName = (WCharCP) attributePtr->text;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_PREFIX_ATTRIBUTE)))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_PREFIX_ATTRIBUTE L" attribute", (WCharCP)xmlNodePtr->baseName);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        WString prefix = (WCharCP) attributePtr->text;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_VERSION_ATTRIBUTE)))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_VERSION_ATTRIBUTE L" attribute", (WCharCP)xmlNodePtr->baseName);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        WString versionString = (WCharCP) attributePtr->text;

        UInt32 versionMajor;
        UInt32 versionMinor;
        if (ECOBJECTS_STATUS_Success != ParseVersionString (versionMajor, versionMinor, versionString.c_str()))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", schemaName.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
            
        // If the schema (uselessly) references itself, just skip it
        if (m_name.compare(schemaName) == 0)
            continue;

        ECObjectsLogger::Log()->debugv (L"About to locate referenced ECSchema %s.%d.%d", schemaName, versionMajor, versionMinor);
        ECSchemaP referencedSchema = LocateSchema(schemaName, versionMajor, versionMinor, schemaContext);

        if (NULL != referencedSchema)
            {
            AddReferencedSchema (*referencedSchema);
            }
        else
            {
            ECObjectsLogger::Log()->errorv(L"Unable to locate referenced schema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);
            return SCHEMA_READ_STATUS_ReferencedSchemaNotFound;
            }
        }

    return status;
    }
#endif //defined (_WIN32) // WIP_NONPORT

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP IECSchemaLocater::LocateSchema(WCharCP name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaDeserializationContextR schemaContext)
    {
    return _LocateSchema (name, versionMajor, versionMinor, matchType, schemaContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchema
(    
const WString &                name,
UInt32&                         versionMajor,
UInt32&                         versionMinor,
ECSchemaDeserializationContextR schemaContext
)
    {
    // Step 1: First check if the owner already owns a copy of the schema.
    //         This will catch schema referencing cycles since the schemas are
    //         added to the owner as they are constructed. 

    // try exact match first
    ECSchemaP schema = schemaContext.GetSchemaOwner().GetSchema(name.c_str(), versionMajor, versionMinor);
    if (NULL != schema)
        return schema;

    // allow latest compatible
    schema = schemaContext.GetSchemaOwner().LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible);
    if (NULL != schema)
        return schema;
  
    // Step 2: ask the schemaLocaters
    FOR_EACH (IECSchemaLocaterP schemaLocater, schemaContext.GetSchemaLocaters())

        {
        if ( ! EXPECTED_CONDITION (NULL != schemaLocater))
            continue;

        schema = schemaLocater->LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible, schemaContext);
        if (NULL != schema)
            return schema;
        }

    // Step 3: look in the paths provided by the context
    schema = LocateSchemaByPath(name, versionMajor, versionMinor, schemaContext);
    if (NULL != schema)
        return schema;
        
    // Step 4: look in a set of standard paths
    return LocateSchemaByStandardPaths (name, versionMajor, versionMinor, schemaContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath)
    {
#if defined (WIP_NONPORT)
    wchar_t  name[256];

    // only process the filename 
    ::_wsplitpath (filePath, NULL, NULL, name, NULL);

// *** NEEDS WORK: _wsplitpath does not include the extension in name
    WCharCP firstDot = wcschr (name, L'.');
    if (NULL == firstDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, filePath);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharCP suffix = wcsstr (name, L".ecschema");
    if (NULL == suffix)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FileName String: '%s' does not contain the suffix '.ecschema.xml'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, filePath);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t versionLen = suffix - firstDot;
    
    wchar_t* buffer = (wchar_t*)_alloca (versionLen*sizeof(wchar_t));
    wcsncpy (buffer, firstDot+1, versionLen-1);
    buffer[versionLen-1] = 0;

    UInt32 versionMajor;
    return ECSchema::ParseVersionString (versionMajor, versionMinor, buffer);
#else
    return ECOBJECTS_STATUS_ParseError;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchemaByPath
(
const WString&                 name,
UInt32&                         versionMajor,
UInt32&                         versionMinor,
ECSchemaDeserializationContextR schemaContext,
bool                            useLatestCompatibleMatch
)
    {
    ECSchemaP   schemaOut = NULL;
    wchar_t versionString[24];
    if (useLatestCompatibleMatch)
        swprintf(versionString, 24, L".%02d.*.ecschema.xml", versionMajor);
    else
        swprintf(versionString, 24, L".%02d.%02d.ecschema.xml", versionMajor, versionMinor);

    WString schemaName = name;
    schemaName += versionString;
    WString fullFileName;

    UInt32 foundVersionMinor;

    FOR_EACH (WString schemaPath, schemaContext.GetSchemaPaths())
        {
        if (schemaPath[schemaPath.length() - 1] != '\\')
            schemaPath += '\\';
        schemaPath += schemaName;
        ECObjectsLogger::Log()->debugv (L"Checking for existence of %s...", schemaPath.c_str());

        //Finds latest
        if (SUCCESS != GetSchemaFileName (fullFileName, foundVersionMinor, schemaPath.c_str(), useLatestCompatibleMatch))
            continue;

        //Check if schema is compatible before serializing, as serializing would add the schema to the cache.
        if (!SchemasMatch (useLatestCompatibleMatch ? SCHEMAMATCHTYPE_LatestCompatible : SCHEMAMATCHTYPE_Exact, 
                            name.c_str(),   versionMajor,   versionMinor,
                            name.c_str(),   versionMajor,   foundVersionMinor))
            continue;

        if (SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlFile (schemaOut, fullFileName.c_str(), schemaContext))
            continue;

        ECObjectsLogger::Log()->debugv (L"Located %s...", fullFileName.c_str());

        return schemaOut;
        }

    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchemaByPath
(
const WString&                 name,
UInt32&                         versionMajor,
UInt32&                         versionMinor,
ECSchemaDeserializationContextR schemaContext
)
    {
    return LocateSchemaByPath (name, versionMajor, versionMinor, schemaContext, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchemaByStandardPaths
(
const WString &                name,
UInt32&                         versionMajor,
UInt32&                         versionMinor,
ECSchemaDeserializationContextR schemaContext
)
    {
    // Make a copy of the paths stored in schemaContext
    T_WStringVector originalPaths = schemaContext.GetSchemaPaths();

    // Clear out the stored paths and replace with the standard ones
    schemaContext.ClearSchemaPaths();

    WString dllPath = ECFileUtilities::GetDllPath();
    if (0 == dllPath.length())
        return NULL;
        
    schemaContext.AddSchemaPath (dllPath.c_str());
    
    wchar_t schemaPath[MAX_PATH];
    wchar_t generalPath[MAX_PATH];
    wchar_t libraryPath[MAX_PATH];
    
    swprintf(schemaPath, MAX_PATH, L"%sSchemas", dllPath.c_str());
    swprintf(generalPath, MAX_PATH, L"%sSchemas\\General", dllPath.c_str());
    swprintf(libraryPath, MAX_PATH, L"%sSchemas\\LibraryUnits", dllPath.c_str());
    schemaContext.AddSchemaPath(schemaPath);
    schemaContext.AddSchemaPath(generalPath);
    schemaContext.AddSchemaPath(libraryPath);
    
    // Do the search
    ECSchemaP   foundSchema = LocateSchemaByPath (name, versionMajor, versionMinor, schemaContext);

    // Put the context back the way it was when we started
    schemaContext.GetSchemaPaths() = originalPaths;

    return foundSchema;
    }

#if defined (_WIN32) // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadXml
(
ECSchemaP&                          schemaOut, 
MSXML2::IXMLDOMDocument2&           pXmlDoc, 
ECSchemaDeserializationContextR     schemaContext
)
    {            
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
    
    pXmlDoc.setProperty("SelectionNamespaces", L"xmlns:" EC_NAMESPACE_PREFIX L"='" ECXML_URI_2_0 L"'");
    MSXML2::IXMLDOMNodePtr xmlNodePtr = pXmlDoc.selectSingleNode (L"/" EC_NAMESPACE_PREFIX L":" EC_SCHEMA_ELEMENT);
    if (NULL == xmlNodePtr)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: Missing a top-level " EC_SCHEMA_ELEMENT L" node in the " ECXML_URI_2_0 L" namespace");
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }
    
    MSXML2::IXMLDOMNodePtr schemaNodePtr = xmlNodePtr;       
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = schemaNodePtr->attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
    
    // schemaName is a REQUIRED attribute in order to create the schema
    if ((NULL == nodeAttributesPtr) || (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMA_NAME_ATTRIBUTE))))
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: " EC_SCHEMA_ELEMENT L" element must contain a schemaName attribute");
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    UInt32  versionMajor = DEFAULT_VERSION_MAJOR;
    UInt32  versionMinor = DEFAULT_VERSION_MINOR;
    MSXML2::IXMLDOMNodePtr versionAttributePtr;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    if ((NULL == (versionAttributePtr = nodeAttributesPtr->getNamedItem (SCHEMA_VERSION_ATTRIBUTE))) ||
        SUCCESS != ParseVersionString (versionMajor, versionMinor, (WCharCP) versionAttributePtr->text))
        {
        ECObjectsLogger::Log()->warningv (L"Invalid version attribute has been ignored while deserializing ECSchema '%s'.  The default version number %d.%d has been applied.", 
            (WCharCP)attributePtr->text, versionMajor, versionMinor);
        }

    ECObjectsLogger::Log()->debugv (L"Deserializing ECSchema %s.%d.%d", (WCharCP)attributePtr->text, versionMajor, versionMinor);

    IECSchemaOwnerR schemaOwner = schemaContext.GetSchemaOwner();
    bool            hideFromLeakDetection = schemaContext.GetHideSchemasFromLeakDetection();
    
    ECObjectsStatus createStatus = CreateSchema (schemaOut, (WCharCP)attributePtr->text, versionMajor, versionMinor, schemaOwner, hideFromLeakDetection);
    if (ECOBJECTS_STATUS_DuplicateSchema == createStatus)
        return SCHEMA_READ_STATUS_DuplicateSchema;
    
    if (ECOBJECTS_STATUS_Success != createStatus)
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

    // OPTIONAL attributes - If these attributes exist they MUST be valid        
    READ_OPTIONAL_XML_ATTRIBUTE (SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadSchemaReferencesFromXml(schemaNodePtr, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    ClassDeserializationVector classes;
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassStubsFromXml (schemaNodePtr, classes, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassContentsFromXml (classes, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    schemaOut->ReadCustomAttributes(schemaNodePtr, *schemaOut, schemaContext.GetStandaloneEnablerLocater());

    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassNameComparer
(
ECClassP class1, 
ECClassP class2
)
    {
    // null values are always less than non-null values
    if (NULL == class1)
        return true;
    if (NULL == class2)
        return false;
    int comparison = wcscmp(class1->GetName().c_str(), class2->GetName().c_str());
    return (comparison <= 0);
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteSchemaReferences (MSXML2::IXMLDOMElement &parentNode) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    ECSchemaReferenceList referencedSchemas = GetReferencedSchemas();
    
    std::set<const WString> usedPrefixes;
    std::set<const WString>::const_iterator setIterator;

#if defined USE_HASHMAP_IN_CLASSLAYOUT
    stdext::hash_map<ECSchemaP, const WString> localReferencedSchemaNamespaceMap;
#else
    std::map<ECSchemaP, const WString> localReferencedSchemaNamespaceMap;
#endif

    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        ECSchemaP refSchema = *schemaIterator;
        WString prefix(refSchema->GetNamespacePrefix());
        if (prefix.length() == 0)
            prefix = L"s";
            
        setIterator = usedPrefixes.find(prefix);
        if (setIterator != usedPrefixes.end())
            {
            int subScript;
            for (subScript = 1; subScript < 500; subScript++)
                {
                wchar_t temp[256];
                swprintf(temp, 256, L"%s%d", prefix.c_str(), subScript);
                WString tryPrefix(temp);
                setIterator = usedPrefixes.find(tryPrefix);
                if (setIterator == usedPrefixes.end())
                    {
                    prefix = tryPrefix;
                    break;
                    }
                }
            }
        usedPrefixes.insert(prefix);
        localReferencedSchemaNamespaceMap.insert(std::pair<ECSchemaP, const WString> (refSchema, prefix));
        }

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;
    MSXML2::IXMLDOMElementPtr schemaPtr = NULL;
    
#if defined USE_HASHMAP_IN_CLASSLAYOUT
    stdext::hash_map<ECSchemaP, const WString>::const_iterator iterator;
#else
    std::map<ECSchemaP, const WString>::const_iterator iterator;
#endif
    for (iterator = localReferencedSchemaNamespaceMap.begin(); iterator != localReferencedSchemaNamespaceMap.end(); iterator++)
        {
        std::pair<ECSchemaP, const WString> mapPair = *(iterator);
        ECSchemaP refSchema = mapPair.first;
        schemaPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, EC_SCHEMAREFERENCE_ELEMENT, ECXML_URI_2_0);
        APPEND_CHILD_TO_PARENT(schemaPtr, (&parentNode));
        
        WRITE_XML_ATTRIBUTE(SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str(), schemaPtr);
        
        wchar_t versionString[8];
        swprintf(versionString, 8, L"%02d.%02d", refSchema->GetVersionMajor(), refSchema->GetVersionMinor());
        WRITE_XML_ATTRIBUTE(SCHEMAREF_VERSION_ATTRIBUTE, versionString, schemaPtr);
        const WString prefix = mapPair.second;
        WRITE_XML_ATTRIBUTE(SCHEMAREF_PREFIX_ATTRIBUTE, prefix.c_str(), schemaPtr);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteCustomAttributeDependencies
(
MSXML2::IXMLDOMElement&          parentNode,
IECCustomAttributeContainerCR   container,
ECSchemaSerializationContext&   context
) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    FOR_EACH (IECInstancePtr instance, container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass(parentNode, currentClass, context);
        if (SCHEMA_WRITE_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteClass (MSXML2::IXMLDOMElement &parentNode, ECClassCR ecClass, ECSchemaSerializationContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    // don't serialize any classes that aren't in the schema we're serializing.
    if (&(ecClass.GetSchema()) != this)
        return status;
    
    bset<WCharCP>::const_iterator setIterator;
    setIterator = context.m_alreadySerializedClasses.find(ecClass.GetName().c_str());
    // Make sure we don't serialize any class twice
    if (setIterator != context.m_alreadySerializedClasses.end())
        return status;
    else
        context.m_alreadySerializedClasses.insert(ecClass.GetName().c_str());
        
    // serialize the base classes first.
    FOR_EACH (ECClassP baseClass, ecClass.GetBaseClasses())
        {
        WriteClass(parentNode, *baseClass, context);
        }
       
    // Serialize relationship constraint dependencies
    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>((ECClassP) &ecClass);
    if (NULL != relClass)
        {
        FOR_EACH (ECClassP source, relClass->GetSource().GetClasses())
            WriteClass(parentNode, *source, context);
            
        FOR_EACH (ECClassP target, relClass->GetTarget().GetClasses())
            WriteClass(parentNode, *target, context);
        }
    WritePropertyDependencies(parentNode, ecClass, context); 
    WriteCustomAttributeDependencies(parentNode, ecClass, context);
    
    ecClass.WriteXml(parentNode);
    
    return status;
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WritePropertyDependencies
(
MSXML2::IXMLDOMElement&         parentNode,
ECClassCR                       ecClass,
ECSchemaSerializationContext&   context
) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    
    FOR_EACH (ECPropertyP prop, ecClass.GetProperties(false))
        {
        if (prop->GetIsStruct())
            {
            StructECPropertyP structProperty = prop->GetAsStructProperty();
            WriteClass(parentNode, structProperty->GetType(), context);
            }
        else if (prop->GetIsArray())
            {
            ArrayECPropertyP arrayProperty = prop->GetAsArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                WriteClass(parentNode, *(arrayProperty->GetStructElementType()), context);
                }
            }
        WriteCustomAttributeDependencies(parentNode, *prop, context);
        }
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteXml (MSXML2::IXMLDOMDocument2* pXmlDoc) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    ECSchemaSerializationContext context;

    MSXML2::IXMLDOMProcessingInstructionPtr piPtr = NULL;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;

    piPtr = pXmlDoc->createProcessingInstruction(L"xml", L"version='1.0'  encoding='UTF-8'");
    APPEND_CHILD_TO_PARENT(piPtr, pXmlDoc);
    //CREATE_AND_ADD_TEXT_NODE(L"\n", piPtr);
    
    MSXML2::IXMLDOMElementPtr schemaElementPtr = pXmlDoc->createNode(NODE_ELEMENT, EC_SCHEMA_ELEMENT, ECXML_URI_2_0);
    APPEND_CHILD_TO_PARENT(schemaElementPtr, pXmlDoc);
    
    MSXML2::IXMLDOMAttributePtr attributePtr;
    wchar_t versionString[8];
    swprintf(versionString, 8, L"%02d.%02d", m_versionMajor, m_versionMinor);
    WRITE_XML_ATTRIBUTE(SCHEMA_NAME_ATTRIBUTE, this->GetName().c_str(), schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, NamespacePrefix, schemaElementPtr);
    WRITE_XML_ATTRIBUTE(SCHEMA_VERSION_ATTRIBUTE, versionString, schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, schemaElementPtr);
    if (GetIsDisplayLabelDefined())
        {
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, schemaElementPtr);
        }
    WRITE_XML_ATTRIBUTE(L"xmlns:" EC_NAMESPACE_PREFIX, ECXML_URI_2_0, schemaElementPtr);
    
    WriteSchemaReferences(schemaElementPtr);
    
    WriteCustomAttributeDependencies(schemaElementPtr, *this, context);
    WriteCustomAttributes(schemaElementPtr);
    
    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are serialized is predictable.
    FOR_EACH (ECClassP pClass, GetClasses())
        sortedClasses.push_back(pClass);
        
    sortedClasses.sort(ClassNameComparer);
    
    FOR_EACH (ECClassP pClass, sortedClasses)
        {
        WriteClass(schemaElementPtr, *pClass, context);
        }
        
    ECXml::FormatXml(pXmlDoc);
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

            WString file;
            if (NULL != pBURL.GetBSTR())
                file = pBURL.GetBSTR();
                
            WString reason = pBReason.GetBSTR();
                        
            ECObjectsLogger::Log()->errorv (L"line %d, position %d parsing ECSchema file %s. %s", line, linePos, file.c_str(), reason.c_str());            
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlFile
(
ECSchemaP&                      schemaOut, 
WCharCP                 ecSchemaXmlFile, 
ECSchemaDeserializationContextR schemaContext
)
    {                  
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlFile);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to deserialize XML file: %s", ecSchemaXmlFile);
    else
        ECObjectsLogger::Log()->infov (L"Native ECSchema Deserialized from file: fileName='%s', schemaName='%s.%d.%d' classCount='%d' address='0x%x'", 
            ecSchemaXmlFile, schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor(), schemaOut->m_classMap.size(), schemaOut);        

    return status;
    }
#endif //defined (_WIN32) // WIP_NONPORT

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaP&                      schemaOut, 
WCharCP                         ecSchemaXml,
ECSchemaDeserializationContextR schemaContext
)
    {                  
#if defined (_WIN32) // WIP_NONPORT
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->loadXML(ecSchemaXml);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (ECOBJECTS_STATUS_Success != status)
        {
        WChar first200Characters[201];
        wcsncpy (first200Characters, ecSchemaXml, 200);
        first200Characters[200] = L'\0';
        ECObjectsLogger::Log()->errorv (L"Failed to deserialize XML from string (1st 200 characters): %s", first200Characters);
        }
    else
        ECObjectsLogger::Log()->infov (L"Native ECSchema Deserialized from string: schemaName='%s.%d.%d' classCount='%d' schemaAddress='0x%x' stringAddress='0x%x'", 
            schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor(), schemaOut->m_classMap.size(), schemaOut, ecSchemaXml);
    return status;
#endif //defined (_WIN32) // WIP_NONPORT
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSchemaReferenced
(
ECSchemaCR thisSchema, 
ECSchemaCR potentiallyReferencedSchema
)
    {
    ECSchemaReferenceList referencedSchemas = thisSchema.GetReferencedSchemas();
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = referencedSchemas.begin(); schemaIterator != referencedSchemas.end(); schemaIterator++)
        {
        ECSchemaP refSchema = *schemaIterator;
        if (ECSchema::SchemasAreEqualByName (refSchema, &(potentiallyReferencedSchema)))
            {
            return true;
            }
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::SchemasAreEqualByName (ECSchemaCP thisSchema, ECSchemaCP thatSchema)
    {
    return ((thisSchema == thatSchema) ||
            ( (0 == thisSchema->GetName().compare(thatSchema->GetName())) &&
              (thisSchema->GetVersionMajor() == thatSchema->GetVersionMajor()) &&
              (thisSchema->GetVersionMinor() == thatSchema->GetVersionMinor())));
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlStream
(
ECSchemaP&                      schemaOut, 
IStreamP                        ecSchemaXmlStream,
ECSchemaDeserializationContextR schemaContext
)
    {                  
#if defined (_WIN32) // WIP_NONPORT
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged
    
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to deserialize XML from stream");
    return status;
#else
    return SCHEMA_READ_STATUS_FailedToParseXml;
#endif //defined (_WIN32) // WIP_NONPORT
    }

#if defined (_WIN32) // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (WString& ecSchemaXml) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;

    ecSchemaXml = xmlDocPtr->xml.GetBSTR();
    
    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlFile
(
WCharCP ecSchemaXmlFile
)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlFile), SCHEMA_WRITE_STATUS_FailedToSaveXml);

    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlStream
(
IStreamP ecSchemaXmlStream
)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlStream), SCHEMA_WRITE_STATUS_FailedToSaveXml);

    return status;
    }
#endif //defined (_WIN32) // WIP_NONPORT

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaDeserializationContext::ECSchemaDeserializationContext(IECSchemaOwnerR owner, IStandaloneEnablerLocaterR enablerLocater)
    :
    m_schemaOwner (owner), m_standaloneEnablerLocater(enablerLocater), m_hideSchemasFromLeakDetection (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaDeserializationContextPtr  ECSchemaDeserializationContext::CreateContext (IECSchemaOwnerR owner, IStandaloneEnablerLocaterR enablerLocater)   
                                                                                        { return new ECSchemaDeserializationContext(owner, enablerLocater); }
ECSchemaDeserializationContextPtr  ECSchemaDeserializationContext::CreateContext (ECSchemaCacheR owner) { return CreateContext (owner, owner); }
void  ECSchemaDeserializationContext::AddSchemaLocaters (bvector<EC::IECSchemaLocaterP>& locators) { m_locators.insert (m_locators.begin(), locators.begin(), locators.end());  }
void  ECSchemaDeserializationContext::AddSchemaLocater (IECSchemaLocaterR locater)      { m_locators.push_back (&locater);  }
void  ECSchemaDeserializationContext::AddSchemaPath (WCharCP path)               { m_searchPaths.push_back (WString(path));   }
void  ECSchemaDeserializationContext::HideSchemasFromLeakDetection ()                   { m_hideSchemasFromLeakDetection = true; }
bvector<IECSchemaLocaterP>& ECSchemaDeserializationContext::GetSchemaLocaters ()                { return m_locators;    }
T_WStringVectorR            ECSchemaDeserializationContext::GetSchemaPaths ()                   { return m_searchPaths; }
void                        ECSchemaDeserializationContext::ClearSchemaPaths ()                 { m_searchPaths.clear();    }
IECSchemaOwnerR             ECSchemaDeserializationContext::GetSchemaOwner()                    { return m_schemaOwner;  }
IStandaloneEnablerLocaterR  ECSchemaDeserializationContext::GetStandaloneEnablerLocater()       { return m_standaloneEnablerLocater;  }
bool                        ECSchemaDeserializationContext::GetHideSchemasFromLeakDetection()   { return m_hideSchemasFromLeakDetection;  }


/////////////////////////////////////////////////////////////////////////////////////////
// IECSchemaOwner
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECSchemaOwner::AddSchema  (ECSchemaR schema) { return _AddSchema (schema); }
ECObjectsStatus IECSchemaOwner::DropSchema (ECSchemaR schema) { return _DropSchema (schema); }
ECSchemaP       IECSchemaOwner::GetSchema  (WCharCP name, UInt32 major, UInt32 minor) { return _GetSchema (name, major, minor); }
ECSchemaP       IECSchemaOwner::LocateSchema (WCharCP name, UInt32 major, UInt32 minor, SchemaMatchType matchType) { return _LocateSchema (name, major, minor, matchType); }

/////////////////////////////////////////////////////////////////////////////////////////
// IStandaloneEnablerLocater
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr  IStandaloneEnablerLocater::LocateStandaloneEnabler (WCharCP schemaName, WCharCP className)
    {
    return  _LocateStandaloneEnabler (schemaName, className);
    }

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCache::~ECSchemaCache ()
    {
    if (m_ecEnablerMap.size())
        m_ecEnablerMap.clear ();

    if (0 == m_schemas.size())
        return;

    FOR_EACH (ECSchemaP ecSchema, m_schemas)
        ECSchema::DestroySchema (ecSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int                             ECSchemaCache::GetCount ()
    {
    return (int)m_schemas.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::_AddSchema   (ECSchemaR ecSchema)
    {
    m_schemas.push_back (&ecSchema);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::_DropSchema  (ECSchemaR ecSchema)
    {
    bvector<ECSchemaP>::iterator iter = std::find(m_schemas.begin(), m_schemas.end(), &ecSchema);
    if (iter == m_schemas.end())
        return ECOBJECTS_STATUS_SchemaNotFound;

    ECSchema::DestroySchema (*iter);

    m_schemas.erase(iter);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                             ECSchemaCache::Clear ()
    {
    for (bvector<ECSchemaP>::iterator it = m_schemas.begin(); it != m_schemas.end(); ++it)
        ECSchema::DestroySchema (*it);
    m_schemas.clear();
    m_ecEnablerMap.clear();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::_GetSchema   (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor)
    {
    return _LocateSchema (schemaName, versionMajor, versionMinor, SCHEMAMATCHTYPE_Exact);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::_LocateSchema (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType)
    {
    FOR_EACH (ECSchemaP ecSchema, m_schemas)
        {
        if (ECSchema::SchemasMatch (matchType, schemaName, versionMajor, versionMinor, ecSchema->GetName().c_str(), ecSchema->GetVersionMajor(), ecSchema->GetVersionMinor()))
            return ecSchema;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCachePtr    ECSchemaCache::Create ()
    {
    return new ECSchemaCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr          ECSchemaCache::_LocateStandaloneEnabler (WCharCP schemaName, WCharCP className)
    {
    SchemaNameClassNamePair keyPair(schemaName, className);

    bmap<SchemaNameClassNamePair, StandaloneECEnablerPtr>::const_iterator  mapIterator;
    mapIterator = m_ecEnablerMap.find (keyPair);

    if (mapIterator != m_ecEnablerMap.end())
        return mapIterator->second;

    // no existing enabler, try to find schema and build one now
    FOR_EACH (ECSchemaP ecSchema, m_schemas)
        {
        if (ecSchema->GetName().EqualsI (schemaName))
            {
            ECClassP structClass = ecSchema->GetClassP (className);
            if (structClass)
                {
                ClassLayoutP classLayout = ClassLayout::BuildFromClass (*structClass, 0, 0);
                StandaloneECEnablerPtr structEnabler = StandaloneECEnabler::CreateEnabler (*structClass, *classLayout, *this, true);
                if (structEnabler.IsValid())
                    m_ecEnablerMap[keyPair] = structEnabler;

                return structEnabler;
                }
            }
        }

    return NULL;
    }
    

/////////////////////////////////////////////////////////////////////////////////////////
// ECClassContainer
/////////////////////////////////////////////////////////////////////////////////////////
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
bool    ECClassContainer::const_iterator::operator== (const_iterator const& rhs) const
    {
    return (m_state->m_mapIterator == rhs.m_state->m_mapIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP const& ECClassContainer::const_iterator::operator*() const
    {
    // Get rid of ECClassContainer or make it return a pointer directly
#ifdef CREATES_A_TEMP
    bpair<WCharCP , ECClassP> const& mapPair = *(m_state->m_mapIterator);
    return mapPair.second;
#else
    return m_state->m_mapIterator->second;
#endif
    };

/*---------------------------------------------------------------------------------**//**
* Validates a name and ensures a name contains only alphanumeric and underscore characters and does not start with a digit.
*
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool NameValidator::Validate
(
const WString &name
) 
    {
    if (name.empty())
        return false;
    if (   L'0' <= name[0]
        && L'9' >= name[0])
        return false; 
    for (WString::size_type index = 0; index != name.length(); ++index)
        {
        if(    (L'a' > name[index] || L'z' < name[index]) 
            && (L'A' > name[index] || L'Z' < name[index])
            && (L'0' > name[index] || L'9' < name[index])
            && '_'  != name[index])
            return false;
        } 
    return true;
    }

END_BENTLEY_EC_NAMESPACE


#if defined (__unix__)
BEGIN_BENTLEY_EC_NAMESPACE
    #define MSXML2_IXMLDOMNode      void *
    #define MSXML2_IXMLDOMNodePtr   void *
    #define MSXML2_IXMLDOMDocument2 void *
    #define MSXML2_IXMLDOMElement   void *
SchemaReadStatus ECSchema::ReadClassStubsFromXml(MSXML2_IXMLDOMNode&,ClassDeserializationVector&,ECSchemaDeserializationContextR){return SCHEMA_READ_STATUS_FailedToParseXml;}
SchemaReadStatus ECSchema::ReadClassContentsFromXml(ClassDeserializationVector&,ECSchemaDeserializationContextR){return SCHEMA_READ_STATUS_FailedToParseXml;}
SchemaReadStatus ECSchema::ReadSchemaReferencesFromXml(MSXML2_IXMLDOMNode&,ECSchemaDeserializationContextR){return SCHEMA_READ_STATUS_FailedToParseXml;}
SchemaReadStatus ECSchema::ReadXml(ECSchemaP&,MSXML2_IXMLDOMDocument2&,ECSchemaDeserializationContextR){return SCHEMA_READ_STATUS_FailedToParseXml;}
SchemaReadStatus ECSchema::ReadFromXmlFile(ECSchemaP&,WCharCP,ECSchemaDeserializationContextR){return SCHEMA_READ_STATUS_FailedToParseXml;}
SchemaWriteStatus ECSchema::WriteSchemaReferences(MSXML2_IXMLDOMElement&)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteCustomAttributeDependencies(MSXML2_IXMLDOMElement&,IECCustomAttributeContainerCR,ECSchemaSerializationContext&)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteClass(MSXML2_IXMLDOMElement&,ECClassCR,ECSchemaSerializationContext&)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WritePropertyDependencies(MSXML2_IXMLDOMElement&,ECClassCR,ECSchemaSerializationContext&)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteXml(MSXML2_IXMLDOMDocument2*)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteToXmlString(WString&)const{return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteToXmlFile(WCharCP){return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}
SchemaWriteStatus ECSchema::WriteToXmlStream(IStreamP){return SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl;}

END_BENTLEY_EC_NAMESPACE
#endif // defined (__unix__)