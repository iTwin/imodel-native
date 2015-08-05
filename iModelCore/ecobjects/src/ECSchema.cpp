/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchema.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <list>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>

#include <ECObjects/StronglyConnectedGraph.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

extern ECObjectsStatus GetSchemaFileName (WStringR fullFileName, uint32_t& foundVersionMinor, WCharCP schemaPath, bool useLatestCompatibleMatch);


// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidName (WCharCP name)
    {
    return RESULT_Valid == Validate (name);
    }

/*---------------------------------------------------------------------------------**//**
* Currently this is only used by ECValidatedName and ECSchema.
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECNameValidation::ValidationResult ECNameValidation::Validate (WCharCP name)
    {
    if (NULL == name || 0 == *name)
        return RESULT_NullOrEmpty;
    else if ('0' <= name[0] && '9' >= name[0])
        return RESULT_BeginsWithDigit;
    else
        {
        for (WCharCP cur = name; *cur; ++cur)
            if (!IsValidAlphaNumericCharacter (*cur))
                return RESULT_IncludesInvalidCharacters;
        }

    return RESULT_Valid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECNameValidation::AppendEncodedCharacter (WStringR encoded, WChar c)
    {
    WChar buf[5];
    HexFormatOptions opts = (HexFormatOptions)(static_cast<int>(HexFormatOptions::LeadingZeros) | static_cast<int>(HexFormatOptions::Uppercase));
    BeStringUtilities::FormatUInt64 (buf, _countof(buf), (uint64_t)c, opts, 4);
    encoded.append (L"__x");
    encoded.append (buf);
    encoded.append (L"__");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidAlphaNumericCharacter (WChar c)
    {
    return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidAlphaNumericCharacter (Utf8Char c)
    {
    return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::DecodeFromValidName (WStringR decoded, WStringCR name)
    {
    // "__x####__"
    //  012345678

    decoded = name;
    size_t pos = 0;
    bool wasDecoded = false;
    while (pos + 8 < decoded.length() && WString::npos != (pos = decoded.find (L"__x", pos)))
        {
        if ('_' == decoded[pos+7] && '_' == decoded[pos+8])
            {
            uint32_t charCode;
            if (1 == BE_STRING_UTILITIES_SWSCANF (decoded.c_str() + pos + 3, L"%x", &charCode))
                {
                decoded[pos] = (WChar)charCode;
                decoded.erase (pos+1, 8);
                wasDecoded = true;
                pos++;
                continue;
                }
            }

        // could not decode this escape code, leave it intact
        pos += 3;
        }

    return wasDecoded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::EncodeToValidName (WStringR encoded, WStringCR name)
    {
    encoded.clear();
    if (name.empty())
        return false;

    encoded.reserve (name.length());
    bool wasEncoded = false;

    // First character cannot be a digit
    size_t startIndex = 0;
    if ('0' <= name[0] && '9' >= name[0])
        {
        AppendEncodedCharacter (encoded, name[0]);
        startIndex = 1;
        wasEncoded = true;
        }

    for (size_t i = startIndex; i < name.length(); i++)
        {
        if (!IsValidAlphaNumericCharacter (name[i]))
            {
            AppendEncodedCharacter (encoded, name[i]);
            wasEncoded = true;
            }
        else
            encoded.append (1, name[i]);
        }

    return wasEncoded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECValidatedName::GetDisplayLabel() const
    {
    return m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValidatedName::SetName (WCharCP name)
    {
    // Note that this method can be called with an un-encoded name (e.g. called by users creating schemas dynamically),
    // or with an encoded name (e.g. when deserializing a schema from xml)
    // Hence we both encode and decode here

    // Encode the name
    m_name.clear();
    ECNameValidation::EncodeToValidName (m_name, name);

    // If the display label has not been explicitly set, use the (decoded) property name
    if (!m_hasExplicitDisplayLabel)
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValidatedName::SetDisplayLabel (WCharCP label)
    {
    if (NULL == label || '\0' == *label)
        {
        m_hasExplicitDisplayLabel = false;
        m_displayLabel.clear();
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_name);
        }
    else
        {
        m_hasExplicitDisplayLabel = true;
        m_displayLabel = label;
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::ECSchema ()
    :m_classContainer(m_classMap), m_isSupplemented(false), m_hasExplicitDisplayLabel(false), m_immutable(false), m_ecSchemaId(0)
    {
    //
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema ()
    {
    for (auto entry : m_classMap)
        {
        ECClassP ecClass = entry.second;
        //==========================================================
        //Bug #23511: Publisher crash related to a NULL ECClass name
        //We need to cleanup any derived class link in other schema.
        //If schema fail later during loading it is possiable that is
        //had created dervied class links in reference ECSchemas. Since
        //This schema would be deleted we need to remove those dead links.
        for(auto baseClass : ecClass->GetBaseClasses())
            {
            if (&baseClass->GetSchema() != this)
                baseClass->RemoveDerivedClass(*ecClass);
            }
        //==========================================================
        }

    ClassMap::iterator  classIterator = m_classMap.begin();
    while (classIterator != m_classMap.end())
        {
        ECClassP ecClass = classIterator->second;
        classIterator = m_classMap.erase(classIterator);
        delete ecClass;
        }
    BeAssert (m_classMap.empty());

    m_refSchemaList.clear();

    memset ((void*)this, 0xececdead, 4);// Replaced sizeof(this) with 4. There is value 
                                        // in trying to make this logic "correct" for a 64-bit build. 
                                        // This memset is clearly just intended to aid in debugging. 
                                        // Attempting to use this object with the high 4 bytes of the vtable 
                                        // pointer set to 0xececdead will crash just as reliably.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchema::IsSystemSchema () const
    {
    return StandardCustomAttributeHelper::IsSystemSchema (*this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsDynamicSchema () const
    {
    return StandardCustomAttributeHelper::IsDynamicSchema (*this);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetIsDynamicSchema (bool isDynamic)
    {
    return StandardCustomAttributeHelper::SetIsDynamicSchema (*this, isDynamic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::SetErrorHandling (bool showMessages, bool doAssert)
    {
    s_noAssert = !doAssert;
    ECClass::SetErrorHandling(doAssert);
    }

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
WStringCR ECSchema::GetName () const
    {
    return m_key.m_schemaName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName (WStringCR name)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    ECNameValidation::EncodeToValidName (m_key.m_schemaName, name);
    if (!m_hasExplicitDisplayLabel)
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_key.m_schemaName);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetNamespacePrefix () const
    {
    return m_namespacePrefix;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetNamespacePrefix (WStringCR namespacePrefix)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;
    m_namespacePrefix = namespacePrefix;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDescription (WStringCR description)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDisplayLabel () const
    {
    return m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel (WStringCR displayLabel)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    m_displayLabel = displayLabel;
    m_hasExplicitDisplayLabel = true;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::GetIsDisplayLabelDefined () const
    {
    return m_hasExplicitDisplayLabel;
    }

static WCharCP s_standardSchemaNames[] =
    {
    L"Bentley_Standard_CustomAttributes",
    L"Bentley_Standard_Classes",
    L"Bentley_ECSchemaMap",
    L"EditorCustomAttributes",
    L"Bentley_Common_Classes",
    L"Dimension_Schema",
    L"iip_mdb_customAttributes",
    L"KindOfQuantity_Schema",
    L"rdl_customAttributes",
    L"SIUnitSystemDefaults",
    L"Unit_Attributes",
    L"Units_Schema",
    L"USCustomaryUnitSystemDefaults"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsStandardSchema(WStringCR schemaName)
    {
    for (WCharCP* cur = s_standardSchemaNames, *end = cur + _countof(s_standardSchemaNames); cur < end; ++cur)
        if (schemaName.Equals (*cur))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsStandardSchema () const
    {
    return IsStandardSchema(m_key.m_schemaName);
    }

static WCharCP s_originalStandardSchemaFullNames[] =
    {
    L"Bentley_Standard_CustomAttributes.01.00",
    L"Bentley_Standard_Classes.01.00",
    L"EditorCustomAttributes.01.00",
    L"Bentley_Common_Classes.01.00",
    L"Dimension_Schema.01.00",
    L"iip_mdb_customAttributes.01.00",
    L"KindOfQuantity_Schema.01.00",
    L"rdl_customAttributes.01.00",
    L"SIUnitSystemDefaults.01.00",
    L"Unit_Attributes.01.00",
    L"Units_Schema.01.00",
    L"USCustomaryUnitSystemDefaults.01.00"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSamePrimarySchema
(
ECSchemaR primarySchema
) const
    {
    if (0 != wcscmp(this->GetNamespacePrefix().c_str(), primarySchema.GetNamespacePrefix().c_str()))
        return false;

    if (0 != wcscmp(this->GetFullSchemaName().c_str(), primarySchema.GetFullSchemaName().c_str()))
        return false;

    return (GetClassCount() == primarySchema.GetClassCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSupplemented
(
) const
    {
    return m_isSupplemented;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored () const
    {
    return ShouldNotBeStored (GetSchemaKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored (SchemaKeyCR key)
    {
    WString schemaName = key.GetFullSchemaName();
    for (WCharCP* cur = s_originalStandardSchemaFullNames, *end = cur + _countof(s_originalStandardSchemaFullNames); cur < end; ++cur)
        if (schemaName.Equals (*cur))
            return true;

    // We don't want to import any version of the Units_Schema
    if (BeStringUtilities::Wcsicmp(L"Units_Schema", key.m_schemaName.c_str()) == 0)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetVersionMajor () const
    {
    return m_key.m_versionMajor;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMajor (const uint32_t versionMajor)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    m_key.m_versionMajor = versionMajor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetVersionMinor
(
) const
    {
    return m_key.m_versionMinor;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMinor (const uint32_t versionMinor)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    m_key.m_versionMinor = versionMinor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchema::GetClassCP (WCharCP name) const
    {
    return const_cast<ECSchemaP> (this)->GetClassP(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECSchema::GetClassP (WCharCP name)
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name);

    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::DebugDump()const
    {
    wprintf(L"ECSchema: this=0x%llx  %ls.%02d.%02d, nClasses=%" PRIu64 "\n", (uint64_t)this, m_key.m_schemaName.c_str(), m_key.m_versionMajor, m_key.m_versionMinor, (uint64_t)m_classMap.size());
    for (ClassMap::const_iterator it = m_classMap.begin(); it != m_classMap.end(); ++it)
        {
        bpair<WCharCP, ECClassP>const& entry = *it;
        ECClassCP ecClass = entry.second;
        wprintf(L"    ECClass: 0x%llx, %ls\n", (uint64_t)ecClass, ecClass->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::DeleteClass (ECClassR ecClass)
    {
    ClassMap::iterator iter = m_classMap.find (ecClass.GetName().c_str());
    if (iter == m_classMap.end() || iter->second != &ecClass)
        return ECOBJECTS_STATUS_ClassNotFound;

    m_classMap.erase (iter);
    delete &ecClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RenameClass (ECClassR ecClass, WCharCP newName)
    {
    ClassMap::iterator iter = m_classMap.find (ecClass.GetName().c_str());
    if (iter == m_classMap.end() || iter->second != &ecClass)
        return ECOBJECTS_STATUS_ClassNotFound;

    ECClassP pClass = &ecClass;
    m_classMap.erase (iter);
    ECObjectsStatus renameStatus = ecClass.SetName (newName);
    ECObjectsStatus addStatus = AddClass (pClass, false);
    return ECOBJECTS_STATUS_Success != renameStatus ? renameStatus : addStatus;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass (ECClassP& pClass, bool deleteClassIfDuplicate)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    bpair <ClassMap::iterator, bool> resultPair;
    resultPair = m_classMap.insert (bpair<WCharCP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        LOG.warningv (L"Can not create class '%ls' because it already exists in the schema", pClass->GetName().c_str());
        if (deleteClassIfDuplicate)
            {
            // preserving weird existing behavior, added option to not do this...
            delete pClass;
            pClass = NULL;
            }

        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
    //DebugDump(); wprintf(L"\n");
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateClass (ECClassP& pClass, WStringCR name)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    pClass = new ECClass(*this);
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
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopyClass
(
ECClassP& targetClass,
ECClassCR sourceClass
)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    // first make sure the class doesn't already exist in the schema
    if (NULL != this->GetClassCP(sourceClass.GetName().c_str()))
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;

    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    ECRelationshipClassCP sourceAsRelationshipClass = sourceClass.GetRelationshipClassCP();
    if (NULL != sourceAsRelationshipClass)
        {
        ECRelationshipClassP newRelationshipClass;
        status = this->CreateRelationshipClass(newRelationshipClass, sourceClass.GetName());
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        newRelationshipClass->SetStrength(sourceAsRelationshipClass->GetStrength());
        newRelationshipClass->SetStrengthDirection(sourceAsRelationshipClass->GetStrengthDirection());

        sourceAsRelationshipClass->GetSource().CopyTo(newRelationshipClass->GetSource());
        sourceAsRelationshipClass->GetTarget().CopyTo(newRelationshipClass->GetTarget());
        targetClass = newRelationshipClass;
        }
    else
        {
        status = CreateClass(targetClass, sourceClass.GetName());
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }

    targetClass->SetIsCustomAttributeClass(sourceClass.GetIsCustomAttributeClass());
    targetClass->SetIsDomainClass(sourceClass.GetIsDomainClass());
    targetClass->SetIsStruct(sourceClass.GetIsStruct());
    if (sourceClass.GetIsDisplayLabelDefined())
        targetClass->SetDisplayLabel(sourceClass.GetDisplayLabel());
    targetClass->SetDescription(sourceClass.GetDescription());

    // Set the base classes on the target class from the source class
    // This is inconsistent with the Managed implementation of CopyClass which does not copy base classes
    for (ECClassP baseClass: sourceClass.GetBaseClasses())
        {
        targetClass->AddBaseClass(*baseClass);
        }

    if (!sourceClass.GetSchema().IsSupplemented())
        {
        for(ECPropertyP sourceProperty: sourceClass.GetProperties(false))
            {
            ECPropertyP destProperty;
            status = targetClass->CopyProperty(destProperty, sourceProperty, true);
            if (ECOBJECTS_STATUS_Success != status)
                return status;
            }
        }

    return sourceClass.CopyCustomAttributesTo(*targetClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass (ECRelationshipClassP& pClass, WStringCR name)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

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
        LOG.warningv (L"Can not create relationship class '%ls' because it already exists in the schema", name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECSchema::GetFullSchemaName () const
    {
    wchar_t fullName[1024]; // we decided to use a large buffer instead of caculating the length and using _alloc to boost performance

    BeStringUtilities::Snwprintf (fullName, L"%ls.%02d.%02d", GetName().c_str(), GetVersionMajor(), GetVersionMinor());
    return fullName;
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (WStringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, WStringCR  fullName)
    {
    if (fullName.empty())
        return ECOBJECTS_STATUS_ParseError;

    WCharCP fullNameCP = fullName.c_str();
    WCharCP firstDot = wcschr (fullNameCP, L'.');
    if (NULL == firstDot)
        {
        LOG.errorv (L"Invalid ECSchema FullName String: '%ls' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullNameCP;
    if (nameLen < 1)
        {
        LOG.errorv (L"Invalid ECSchema FullName String: '%ls' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullNameCP, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (WStringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, WCharCP fullName)
    {
    if (NULL == fullName || '\0' == *fullName)
        return ECOBJECTS_STATUS_ParseError;

    WCharCP firstDot = wcschr (fullName, L'.');
    if (NULL == firstDot)
        {
        LOG.errorv (L"Invalid ECSchema FullName String: '%ls' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullName;
    if (nameLen < 1)
        {
        LOG.errorv (L"Invalid ECSchema FullName String: '%ls' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullName, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECSchema::FormatSchemaVersion (uint32_t& versionMajor, uint32_t& versionMinor)
    {
    wchar_t versionString[80];
    BeStringUtilities::Snwprintf (versionString, _countof(versionString), L"%02d.%02d", versionMajor, versionMinor);
    return WString (versionString);
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLANATION L" Format must be MM.mm where MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString (uint32_t& versionMajor, uint32_t& versionMinor, WCharCP versionString)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (NULL == versionString || '\0' == *versionString)
        return ECOBJECTS_STATUS_Success;

    WCharCP theDot = wcschr (versionString, L'.');
    if (NULL == theDot)
        {
        LOG.errorv (L"Invalid ECSchema Version String: '%ls' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t majorLen = theDot - versionString;
    if (majorLen < 1 || majorLen > 3)
        {
        LOG.errorv (L"Invalid ECSchema Version String: '%ls' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharCP endDot = wcschr (theDot+1, L'.');
    size_t minorLen = (NULL != endDot) ? (endDot - theDot) - 1 : wcslen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        LOG.errorv (L"Invalid ECSchema Version String: '%ls' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharP end = NULL;
    uint32_t  localMajor = BeStringUtilities::Wcstoul (versionString, &end, 10);
    if (versionString == end)
        {
        LOG.errorv (L"Invalid ECSchema Version String: '%ls' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        {
        versionMajor = localMajor;
        }

    uint32_t localMinor = BeStringUtilities::Wcstoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        LOG.errorv (L"Invalid ECSchema Version String: '%ls' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
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
ECObjectsStatus ECSchema::SetVersionFromString (WCharCP versionString)
    {
    if (m_immutable) return ECOBJECTS_STATUS_SchemaIsImmutable;

    uint32_t versionMajor;
    uint32_t versionMinor;
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
ECObjectsStatus ECSchema::CreateSchema (ECSchemaPtr& schemaOut, WStringCR schemaName, uint32_t versionMajor, uint32_t versionMinor)
    {
    schemaOut = new ECSchema();

    ECObjectsStatus status;

    if (ECOBJECTS_STATUS_Success != (status = schemaOut->SetName (schemaName)) ||
        ECOBJECTS_STATUS_Success != (status = schemaOut->SetVersionMajor (versionMajor)) ||
        ECOBJECTS_STATUS_Success != (status = schemaOut->SetVersionMinor (versionMinor)))
        {
        schemaOut = NULL;
        return status;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopySchema
(
ECSchemaPtr& schemaOut
) const
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    status = CreateSchema(schemaOut,  GetName(), GetVersionMajor(), GetVersionMinor());
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    schemaOut->SetDescription(m_description);
    if (GetIsDisplayLabelDefined())
        schemaOut->SetDisplayLabel(GetDisplayLabel());

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        schemaOut->AddReferencedSchema(*iter->second.get());
        
    for(ECClassP ecClass: m_classContainer)
        {
        ECClassP copyClass;
        status = schemaOut->CopyClass(copyClass, *ecClass);
        if (ECOBJECTS_STATUS_Success != status && ECOBJECTS_STATUS_NamedItemAlreadyExists != status)
            return status;
        }

    return CopyCustomAttributesTo(*schemaOut);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::GetSchemaByNamespacePrefixP (WStringCR namespacePrefix) const
    {
    if (namespacePrefix.length() == 0)
        return this;

    // lookup referenced schema by prefix
    bmap<ECSchemaP, WString>::const_iterator schemaIterator;
    for (schemaIterator = m_referencedSchemaNamespaceMap.begin(); schemaIterator != m_referencedSchemaNamespaceMap.end(); schemaIterator++)
        {
        if (0 == namespacePrefix.compare (schemaIterator->second))
            return schemaIterator->first;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ResolveNamespacePrefix (ECSchemaCR schema, WStringR namespacePrefix) const
    {
    namespacePrefix = EMPTY_STRING;
    if (&schema == this)
        return ECOBJECTS_STATUS_Success;

    bmap<ECSchemaP, WString >::const_iterator schemaIterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &schema);
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
ECClassContainerCR ECSchema::GetClasses () const
    {
    return m_classContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetClassCount
(
) const
    {
    return (uint32_t) m_classMap.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReferenceListCR ECSchema::GetReferencedSchemas () const
    {
    return m_refSchemaList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema)
    {
    return AddReferencedSchema (refSchema, refSchema.GetNamespacePrefix());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema, WStringCR namespacePrefix)
    {
    ECSchemaReadContext context (NULL, false);
    return AddReferencedSchema(refSchema, namespacePrefix, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema, WStringCR namespacePrefix, ECSchemaReadContextR readContext)
    {
    SchemaKeyCR refSchemaKey = refSchema.GetSchemaKey();
    if (m_refSchemaList.end () != m_refSchemaList.find (refSchemaKey))
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;

    WString prefix(namespacePrefix);
    if (prefix.length() == 0)
        prefix = L"s";

    // Make sure prefix is unique within this schema
    bmap<ECSchemaP, WString >::const_iterator namespaceIterator;
    for (namespaceIterator = m_referencedSchemaNamespaceMap.begin(); namespaceIterator != m_referencedSchemaNamespaceMap.end(); namespaceIterator++)
        {
        if (0 == prefix.compare (namespaceIterator->second))
            {
            break;
            }
        }

    // We found a matching prefix already being referenced
    if (namespaceIterator != m_referencedSchemaNamespaceMap.end())
        {
        int subScript;
        for (subScript = 1; subScript < 500; subScript++)
            {
            wchar_t temp[256];
            BeStringUtilities::Snwprintf(temp, 256, L"%ls%d", prefix.c_str(), subScript);
            WString tryPrefix(temp);
            for (namespaceIterator = m_referencedSchemaNamespaceMap.begin(); namespaceIterator != m_referencedSchemaNamespaceMap.end(); namespaceIterator++)
                {
                if (0 == tryPrefix.compare (namespaceIterator->second))
                    {
                    break;
                    }
                }
            // we didn't find the prefix in the map
            if (namespaceIterator == m_referencedSchemaNamespaceMap.end())
                {
                prefix = tryPrefix;
                break;
                }
            }
        }

    m_refSchemaList[refSchemaKey] = &refSchema;
    // Check for recursion
    if (AddingSchemaCausedCycles ())
        {
        m_refSchemaList.erase (refSchemaKey);
        return ECOBJECTS_STATUS_SchemaHasReferenceCycle;
        }

    m_referencedSchemaNamespaceMap.insert(bpair<ECSchemaP, const WString> (&refSchema, prefix));
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema (ECSchemaR refSchema)
    {
    ECSchemaReferenceList::iterator schemaIterator = m_refSchemaList.find (refSchema.GetSchemaKey());
    if (schemaIterator == m_refSchemaList.end())
        return ECOBJECTS_STATUS_SchemaNotFound;

    // Can only remove the reference if nothing actually references it.

    ECSchemaPtr foundSchema = schemaIterator->second;
    for (ECClassP ecClass: GetClasses())
        {
        // First, check each base class to see if the base class uses that schema
        for (ECClassP baseClass: ecClass->GetBaseClasses())
            {
            if ((ECSchemaP) &(baseClass->GetSchema()) == foundSchema.get())
                {
                return ECOBJECTS_STATUS_SchemaInUse;
                }
            }

        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
        if (NULL != relClass)
            {
            for (auto target : relClass->GetTarget().GetConstraintClasses())
                {
                if ((ECSchemaP) &(target->GetClass().GetSchema()) == foundSchema.get())
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            for (auto source : relClass->GetSource().GetConstraintClasses())
                {
                if ((ECSchemaP) &(source->GetClass().GetSchema()) == foundSchema.get())
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            }

        // And make sure that there are no struct types from another schema
        for (ECPropertyP prop: ecClass->GetProperties(false))
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
    bmap<ECSchemaP, WString>::iterator iterator = m_referencedSchemaNamespaceMap.find(&refSchema);
    if (iterator != m_referencedSchemaNamespaceMap.end())
        m_referencedSchemaNamespaceMap.erase(iterator);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadClassStubsFromXml (BeXmlNodeR schemaNode, ClassDeserializationVector& classes, ECSchemaReadContextR schemaContext)
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
            ecClass = new ECClass (*this);
            ecRelationshipClass = NULL;
            }
        else if (0 == strcmp (nodeName, EC_RELATIONSHIP_CLASS_ELEMENT))
            {
            ecRelationshipClass = new ECRelationshipClass (*this);
            ecClass = ecRelationshipClass;
            }
        else
            continue;

        if (SCHEMA_READ_STATUS_Success != (status = ecClass->_ReadXmlAttributes (*classNode)))
            {
            delete ecClass;
            return status;
            }

        ECClassP existingClass = this->GetClassP (ecClass->GetName().c_str());

        if (NULL != existingClass)
            {
            existingClass->_ReadXmlAttributes (*classNode);
            delete ecClass;
            ecClass = existingClass;
            }
        else if (ECOBJECTS_STATUS_Success != this->AddClass (ecClass))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

        if (NULL == ecRelationshipClass)
            LOG.tracev (L"    Created ECClass Stub: %ls", ecClass->GetName().c_str());
        else
            LOG.tracev (L"    Created Relationship ECClass Stub: %ls", ecClass->GetName().c_str());

        classes.push_back (make_bpair (ecClass, classNode));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 - Expects class stubs have already been read and created.  They are stored in the vector passed into this method.
 - Expects referenced schemas have been resolved and read so that base classes & structs in other schemas can be located.
 - Reads the contents of each XML node cached in the classes vector and populates the in-memory EC:ECClass with
   base classes, properties & relationship endpoints.
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadClassContentsFromXml (ClassDeserializationVector& classes, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP    ecClass;
    BeXmlNodeP  classNode;
    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass     = classesIterator->first;
        classNode   = classesIterator->second;
        status = ecClass->_ReadXmlContents (*classNode, schemaContext);
        if (SCHEMA_READ_STATUS_Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetIsSupplemented
(
bool isSupplemented
)
    {
    m_isSupplemented = isSupplemented;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetSupplementalSchemaInfo(SupplementalSchemaInfo* info)
    {
    m_supplementalSchemaInfo = info;
    if (NULL == info)
        this->RemoveCustomAttribute(SupplementalSchemaInfo::GetCustomAttributeAccessor());
    else
        {
        IECInstancePtr attribute = info->CreateCustomAttribute();
        if (attribute.IsValid())
            {
            this->SetConsolidatedCustomAttribute(*attribute);
            auto& bsca = attribute->GetClass().GetSchema();
            if (!ECSchema::IsSchemaReferenced(*this, bsca ))
                {
                this->AddReferencedSchema(const_cast<ECSchemaR>(bsca));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfoPtr const ECSchema::GetSupplementalInfo() const
    {
    return m_supplementalSchemaInfo;
    }

/*---------------------------------------------------------------------------------**//**
* - OpenPlant shipped a malformed schema that has a circular reference through supplementation.
* - Therefore a special case had to be created so that we do not try to de-serialize this
* - schema
* @bsimethod                                    Carole.MacDonald                01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECSchema::IsOpenPlantPidCircularReferenceSpecialCase
(
WString& referencedECSchemaName
)
    {
    if (0 != referencedECSchemaName.CompareTo(L"OpenPlant_PID"))
        return false;

    WString fullName = GetFullSchemaName();
    return (0 == fullName.CompareTo(L"OpenPlant_Supplemental_Mapping_OPPID.01.01") || 0 == fullName.CompareTo(L"OpenPlant_Supplemental_Mapping_OPPID.01.02"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadSchemaReferencesFromXml (BeXmlNodeR schemaNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    m_referencedSchemaNamespaceMap.clear();

    BeXmlDom::IterableNodeSet schemaReferenceNodes;
    schemaNode.SelectChildNodes (schemaReferenceNodes, EC_NAMESPACE_PREFIX ":" EC_SCHEMAREFERENCE_ELEMENT);
    for (BeXmlNodeP& schemaReferenceNode: schemaReferenceNodes)
        {
        SchemaKey key;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (key.m_schemaName, SCHEMAREF_NAME_ATTRIBUTE))
            {
            LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_NAME_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        WString prefix;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (prefix, SCHEMAREF_PREFIX_ATTRIBUTE))
            {
            LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_PREFIX_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }


        WString versionString;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (versionString, SCHEMAREF_VERSION_ATTRIBUTE))
            {
            LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_VERSION_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        if (ECOBJECTS_STATUS_Success != ParseVersionString (key.m_versionMajor, key.m_versionMinor, versionString.c_str()))
            {
            LOG.errorv (L"Invalid ECSchemaXML: unable to parse version string for referenced schema %ls.", key.m_schemaName.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        // If the schema (uselessly) references itself, just skip it
        if (m_key.m_schemaName.compare(key.m_schemaName) == 0)
            continue;

        if (IsOpenPlantPidCircularReferenceSpecialCase(key.m_schemaName))
            continue;

        LOG.debugv (L"About to locate referenced ECSchema %ls", key.GetFullSchemaName().c_str());

        ECSchemaPtr referencedSchema = LocateSchema (key, schemaContext);

        if (referencedSchema.IsValid())
            {
            //We can encounter some time same schema referenced twice with different namespacePrefix.
            //We will not treat it as error.
            SchemaKeyCR refSchemaKey = referencedSchema->GetSchemaKey ();
            auto const& references = GetReferencedSchemas ();
            if (references.end () != references.find (refSchemaKey))
                {
                continue;
                }

            ECObjectsStatus status = AddReferencedSchema (*referencedSchema, prefix, schemaContext);
            if (ECOBJECTS_STATUS_Success != status)
                return ECOBJECTS_STATUS_SchemaHasReferenceCycle == status ? SCHEMA_READ_STATUS_HasReferenceCycle : static_cast<SchemaReadStatus> (status);
            }
        else
            {
            LOG.errorv(L"Unable to locate referenced schema %ls", key.GetFullSchemaName().c_str());
            return SCHEMA_READ_STATUS_ReferencedSchemaNotFound;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr IECSchemaLocater::LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return _LocateSchema (key, matchType, schemaContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchema::LocateSchema (SchemaKeyR key, ECSchemaReadContextR schemaContext)
    {
    return schemaContext.LocateSchema(key, SCHEMAMATCHTYPE_LatestCompatible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetMinorVersionFromSchemaFileName (uint32_t& versionMinor, WCharCP filePath)
    {
    BeFileName  fileName (filePath);

    WString     name;
    fileName.ParseName (NULL, NULL, &name, NULL);

    // after fileName.parse, name contains "SchemaName.XX.XX.eschema".
    WString::size_type firstDot;
    if (WString::npos == (firstDot = name.find ('.')))
        {
        BeAssert (s_noAssert);
        LOG.errorv (L"Invalid ECSchema FileName String: '%ls' does not contain the suffix '.ecschema.xml'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, filePath);
        return ECOBJECTS_STATUS_ParseError;
        }

    WString     versionString = name.substr (firstDot+1);
    uint32_t    versionMajor;
    return ECSchema::ParseVersionString (versionMajor, versionMinor, versionString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetSchemaFileName (WString& fullFileName, uint32_t& foundMinorVersion, WCharCP schemaPath, bool useLatestCompatibleMatch)
    {
    WString     schemaPathWithWildcard = schemaPath;
    schemaPathWithWildcard += L"*";

    BeFileListIterator  fileList (schemaPathWithWildcard.c_str(), false);
    BeFileName          filePath;
    uint32_t currentMinorVersion=0;

    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        WCharCP     fileName = filePath.GetName();

        if (!useLatestCompatibleMatch)
            {
            fullFileName = fileName;
            return ECOBJECTS_STATUS_Success;
            }

        if (fullFileName.empty())
            {
            fullFileName = fileName;
            GetMinorVersionFromSchemaFileName (foundMinorVersion, fileName);
            continue;
            }

        if (ECOBJECTS_STATUS_Success != GetMinorVersionFromSchemaFileName (currentMinorVersion, fileName))
            continue;

        if (currentMinorVersion > foundMinorVersion)
            {
            foundMinorVersion = currentMinorVersion;
            fullFileName = fileName;
            }
        }

    if (fullFileName.empty())
        return ECOBJECTS_STATUS_Error;

    return ECOBJECTS_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr       SearchPathSchemaFileLocater::FindMatchingSchema
(
WStringCR                       schemaMatchExpression,
SchemaKeyR                      key,
ECSchemaReadContextR            schemaContext,
SchemaMatchType                 matchType,
bvector<WString>&               searchPaths
)
    {
    WString fullFileName;
    for (WString schemaPathStr: searchPaths)
        {
        BeFileName schemaPath (schemaPathStr.c_str());
        schemaPath.AppendToPath (schemaMatchExpression.c_str());
        LOG.debugv (L"Checking for existence of %ls...", schemaPath.GetName());

        //Finds latest
        SchemaKey foundKey(key);
        if (ECOBJECTS_STATUS_Success != GetSchemaFileName (fullFileName, foundKey.m_versionMinor, schemaPath,  matchType == SCHEMAMATCHTYPE_LatestCompatible))
            continue;

        ECSchemaPtr schemaOut = NULL;
        //Check if schema is compatible before reading, as reading it would add the schema to the cache.
        if (!foundKey.Matches(key, matchType))
            {
            if (schemaContext.m_acceptLegacyImperfectLatestCompatibleMatch && matchType == SCHEMAMATCHTYPE_LatestCompatible &&
                0 == foundKey.m_schemaName.CompareTo(key.m_schemaName) && foundKey.m_versionMajor == key.m_versionMajor)
                {
                // See if this imperfect match ECSchema has is already cached (so we can avoid loading it, below)

                //We found a different key;
                if (matchType != SCHEMAMATCHTYPE_Exact)
                    schemaOut = schemaContext.GetFoundSchema(foundKey, SCHEMAMATCHTYPE_Exact);

                if (schemaOut.IsValid())
                    {
                    key.m_versionMinor = foundKey.m_versionMinor;
                    return schemaOut;
                    }
                LOG.warningv (L"Located %ls, which does not meet 'latest compatible' criteria to match %ls, but is being accepted because some legacy schemas are known to require this",
                                                  fullFileName.c_str(), key.GetFullSchemaName().c_str());
                }
            else
                {
                LOG.warningv (L"Located %ls, but it does not meet 'latest compatible' criteria to match %ls.%02d.%02d. Caller can use acceptImperfectLegacyMatch to cause it to be accepted.",
                                                  fullFileName.c_str(), key.m_schemaName.c_str(),   key.m_versionMajor,   key.m_versionMinor);
                continue;
                }
            }

        if (SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlFile (schemaOut, fullFileName.c_str(), schemaContext))
            continue;

        LOG.debugv (L"Located %ls...", fullFileName.c_str());
        // Now check this same path for supplemental schemas
        bvector<ECSchemaP> supplementalSchemas;
        TryLoadingSupplementalSchemas(key.m_schemaName.c_str(), schemaPathStr, schemaContext, supplementalSchemas);
        if (supplementalSchemas.size() > 0)
            {
            ECN::SupplementedSchemaBuilder builder;
            builder.UpdateSchema(*schemaOut, supplementalSchemas);
            }

        return schemaOut;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchPathSchemaFileLocater::TryLoadingSupplementalSchemas
(
WStringCR schemaName, 
WStringCR schemaFilePath, 
ECSchemaReadContextR schemaContext,
bvector<ECSchemaP>& supplementalSchemas
)
    {
    BeFileName schemaPath (schemaFilePath.c_str());
    WString filter = schemaName + L"_Supplemental_*.*.*.ecschema.xml";
    schemaPath.AppendToPath(filter.c_str());
    BeFileListIterator fileList(schemaPath.GetName(), false);
    BeFileName filePath;
    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        WCharCP     fileName = filePath.GetName();
        ECSchemaPtr schemaOut = NULL;

        if (SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlFile (schemaOut, fileName, schemaContext))
            continue;
        supplementalSchemas.push_back(schemaOut.get());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     SearchPathSchemaFileLocater::LocateSchemaByPath (SchemaKeyR key, ECSchemaReadContextR schemaContext, SchemaMatchType matchType, bvector<WString>& searchPaths)
    {
    wchar_t versionString[24];

    if (matchType == SCHEMAMATCHTYPE_Latest)
        BeStringUtilities::Snwprintf(versionString, 24, L".*.*.ecschema.xml");
    else if (matchType == SCHEMAMATCHTYPE_LatestCompatible)
        BeStringUtilities::Snwprintf(versionString, 24, L".%02d.*.ecschema.xml", key.m_versionMajor);
    else
        BeStringUtilities::Snwprintf(versionString, 24, L".%02d.%02d.ecschema.xml", key.m_versionMajor, key.m_versionMinor);

    WString schemaMatchExpression(key.m_schemaName);
    schemaMatchExpression += versionString;
    WString fullFileName;

    ECSchemaPtr   schemaOut = FindMatchingSchema (schemaMatchExpression, key, schemaContext, matchType, searchPaths);
    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::SearchPathSchemaFileLocater (bvector<WString> const& searchPaths) : m_searchPaths(searchPaths) {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::~SearchPathSchemaFileLocater () {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocaterPtr SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths)
    {
    return new SearchPathSchemaFileLocater(searchPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr SearchPathSchemaFileLocater::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return LocateSchemaByPath (key, schemaContext, matchType, m_searchPaths);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadXml (ECSchemaPtr& schemaOut, BeXmlDomR xmlDom, uint32_t checkSum, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
    StopWatch overallTimer(L"Overall schema de-serialization timer", true);

    xmlDom.RegisterNamespace (EC_NAMESPACE_PREFIX, ECXML_URI_2_0);

    BeXmlNodeP      schemaNode;
    if ( (BEXML_Success != xmlDom.SelectNode (schemaNode, "/" EC_NAMESPACE_PREFIX ":" EC_SCHEMA_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == schemaNode) )
        {
        BeAssert (s_noAssert);
        LOG.errorv(L"Invalid ECSchemaXML: Missing a top-level %ls node in the %ls namespace", WString(EC_SCHEMA_ELEMENT, BentleyCharEncoding::Utf8).c_str(), WString(ECXML_URI_2_0, BentleyCharEncoding::Utf8).c_str());
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    // schemaName is a REQUIRED attribute in order to create the schema
    WString schemaName;
    if (BEXML_Success != schemaNode->GetAttributeStringValue (schemaName, SCHEMA_NAME_ATTRIBUTE))
        {
        BeAssert (s_noAssert);
        LOG.errorv(L"Invalid ECSchemaXML: %ls element must contain a schemaName attribute", WString(EC_SCHEMA_ELEMENT, BentleyCharEncoding::Utf8).c_str());
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    uint32_t versionMajor = DEFAULT_VERSION_MAJOR;
    uint32_t versionMinor = DEFAULT_VERSION_MINOR;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    WString     versionString;
    if ( (BEXML_Success != schemaNode->GetAttributeStringValue (versionString, SCHEMA_VERSION_ATTRIBUTE)) ||
         (ECOBJECTS_STATUS_Success != ParseVersionString (versionMajor, versionMinor, versionString.c_str())) )
        {
        LOG.warningv (L"Invalid version attribute has been ignored while reading ECSchema '%ls'.  The default version number %02d.%02d has been applied.",
            schemaName.c_str(), versionMajor, versionMinor);
        }

    LOG.debugv (L"Reading ECSchema %ls.%02d.%02d", (WCharCP)schemaName.c_str(), versionMajor, versionMinor);

    ECObjectsStatus createStatus = CreateSchema (schemaOut, schemaName, versionMajor, versionMinor);
    if (ECOBJECTS_STATUS_Success != createStatus)
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

    schemaOut->m_key.m_checkSum = checkSum;

    if (ECOBJECTS_STATUS_DuplicateSchema == schemaContext.AddSchema (*schemaOut))
        {
        return SCHEMA_READ_STATUS_DuplicateSchema;
        }

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    WString value;  // used by macro.
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    StopWatch readingSchemaReferences(L"Reading Schema References", true);
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadSchemaReferencesFromXml (*schemaNode, schemaContext)))
        {
        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }

    readingSchemaReferences.Stop();
    LOG.tracev(L"Reading schema references for %ls took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingSchemaReferences.GetElapsedSeconds());

    ClassDeserializationVector classes;
    StopWatch readingClassStubs(L"Reading class stubs", true);
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassStubsFromXml (*schemaNode, classes, schemaContext)))
        {
        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }
    readingClassStubs.Stop();
    LOG.tracev(L"Reading class stubs for %ls took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassStubs.GetElapsedSeconds());

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    StopWatch readingClassContents(L"Reading class contents", true);
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassContentsFromXml (classes, schemaContext)))
        {
        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = NULL;
        return status;
        }
    readingClassContents.Stop();
    LOG.tracev(L"Reading class contents for %ls took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingClassContents.GetElapsedSeconds());

    StopWatch readingCustomAttributes(L"Reading custom attributes", true);
    schemaOut->ReadCustomAttributes(*schemaNode, schemaContext, *schemaOut);
    readingCustomAttributes.Stop();
    LOG.tracev(L"Reading custom attributes for %ls took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), readingCustomAttributes.GetElapsedSeconds());


    //Compute the schema checkSum
    overallTimer.Stop();
    LOG.debugv(L"Overall schema de-serialization for %ls took %.4lf seconds\n", schemaOut->GetFullSchemaName().c_str(), overallTimer.GetElapsedSeconds());

    return SCHEMA_READ_STATUS_Success;
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteSchemaReferences (BeXmlNodeR parentNode) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    bmap<ECSchemaP, WString>::const_iterator iterator;
    for (iterator = m_referencedSchemaNamespaceMap.begin(); iterator != m_referencedSchemaNamespaceMap.end(); iterator++)
        {
        bpair<ECSchemaP, const WString> mapPair = *(iterator);
        ECSchemaP   refSchema           = mapPair.first;
        BeXmlNodeP  schemaReferenceNode = parentNode.AddEmptyElement (EC_SCHEMAREFERENCE_ELEMENT);

        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str());

        wchar_t versionString[8];
        BeStringUtilities::Snwprintf(versionString, 8, L"%02d.%02d", refSchema->GetVersionMajor(), refSchema->GetVersionMinor());
        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_VERSION_ATTRIBUTE, versionString);

        const WString prefix = mapPair.second;
        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_PREFIX_ATTRIBUTE, prefix.c_str());
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteCustomAttributeDependencies (BeXmlNodeR parentNode, IECCustomAttributeContainerCR container, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    for (IECInstancePtr instance: container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass (parentNode, currentClass, context);
        if (SCHEMA_WRITE_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteClass (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    // don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != this)
        return status;

    bset<WCharCP>::const_iterator setIterator;
    setIterator = context.m_alreadyWrittenClasses.find(ecClass.GetName().c_str());
    // Make sure we don't write any class twice
    if (setIterator != context.m_alreadyWrittenClasses.end())
        return status;
    else
        context.m_alreadyWrittenClasses.insert(ecClass.GetName().c_str());

    // write the base classes first.
    for (ECClassP baseClass: ecClass.GetBaseClasses())
        {
        WriteClass(parentNode, *baseClass, context);
        }

    // Serialize relationship constraint dependencies
    ECRelationshipClassP relClass = const_cast<ECRelationshipClassP>(ecClass.GetRelationshipClassCP());
    if (NULL != relClass)
        {
        for (auto source : relClass->GetSource().GetConstraintClasses())
            WriteClass(parentNode, source->GetClass(), context);

        for (auto target : relClass->GetTarget().GetConstraintClasses())
            WriteClass(parentNode, target->GetClass(), context);
        }
    WritePropertyDependencies(parentNode, ecClass, context);
    WriteCustomAttributeDependencies(parentNode, ecClass, context);

    BeXmlNodeP  classNode;
    ecClass._WriteXml (classNode, parentNode);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WritePropertyDependencies (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    for (ECPropertyP prop: ecClass.GetProperties(false))
        {
        if (prop->GetIsStruct())
            {
            StructECPropertyP structProperty = prop->GetAsStructPropertyP();
            WriteClass(parentNode, structProperty->GetType(), context);
            }
        else if (prop->GetIsArray())
            {
            ArrayECPropertyP arrayProperty = prop->GetAsArrayPropertyP();
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
SchemaWriteStatus ECSchema::WriteXml (BeXmlDomR xmlDom) const
    {
    BeXmlNodeP  schemaNode = xmlDom.AddNewElement (EC_SCHEMA_ELEMENT, NULL, NULL);

    wchar_t versionString[8];
    BeStringUtilities::Snwprintf (versionString, _countof(versionString), L"%02d.%02d", m_key.m_versionMajor, m_key.m_versionMinor);

    schemaNode->AddAttributeStringValue (SCHEMA_NAME_ATTRIBUTE, this->GetName().c_str());
    schemaNode->AddAttributeStringValue (SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, this->GetNamespacePrefix().c_str());
    schemaNode->AddAttributeStringValue (SCHEMA_VERSION_ATTRIBUTE, versionString);
    schemaNode->AddAttributeStringValue (DESCRIPTION_ATTRIBUTE, this->GetDescription().c_str());
    if (GetIsDisplayLabelDefined())
        schemaNode->AddAttributeStringValue (DISPLAY_LABEL_ATTRIBUTE, this->GetDisplayLabel().c_str());

    // make sure the schema node has a namespace, and if it doesn't set its default namespace.
    if (NULL == schemaNode->GetNamespace())
        schemaNode->SetNamespace (NULL, ECXML_URI_2_0);

    WString namespaceSpec (ECXML_URI_2_0, true);
    schemaNode->AddAttributeStringValue ("xmlns:" EC_NAMESPACE_PREFIX, namespaceSpec.c_str());

    WriteSchemaReferences (*schemaNode);

    ECSchemaWriteContext context;
    WriteCustomAttributeDependencies (*schemaNode, *this, context);
    WriteCustomAttributes (*schemaNode);

    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are written is predictable.
    for (ECClassP pClass: GetClasses())
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
        WriteClass (*schemaNode, *pClass, context);
        }

    ECXml::FormatXml (xmlDom);
    return SCHEMA_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LogXmlLoadError (BeXmlDomP xmlDom)
    {
    WString     errorString;
    int         line = 0, linePos = 0;
    if (NULL == xmlDom)
        {
        BeXmlDom::GetLastErrorString (errorString);
        }
    else
        {
        xmlDom->GetErrorMessage (errorString);
        xmlDom->GetErrorLocation (line, linePos);
        }

    LOG.errorv (errorString.c_str());
    LOG.errorv (L"line %d, position %d", line, linePos);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddFilePathToSchemaPaths  (ECSchemaReadContextR schemaContext, WCharCP ecSchemaXmlFile)
    {
    BeFileName pathToThisSchema (BeFileName::DevAndDir, ecSchemaXmlFile);
    schemaContext.AddSchemaPath(pathToThisSchema);
    }

/*---------------------------------------------------------------------------------**//**
I initially considered adler-32 for its speed but for small schemas it will not work well.
So we are using crc-32
http://tools.ietf.org/html/rfc3309

The crc 32 function should be moved to use the boost version once the run time check failure #1
has been addressed by boost. This is the version used by the zip library
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static const uint32_t crc_table[256] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
    };
struct          CheckSumHelper
    {
    #define CRC32(c, b) (crc_table[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
    #define DO1(buf)  crc = CRC32(crc, *buf++)
    #define DO2(buf)  DO1(buf); DO1(buf)
    #define DO4(buf)  DO2(buf); DO2(buf)
    #define DO8(buf)  DO4(buf); DO4(buf)

    static uint32_t crc32(uint32_t crc, const Byte* buf, size_t len)
    { if (buf==NULL) return 0L;
      crc = crc ^ 0xffffffffL;
      while (len >= 8) {DO8(buf); len -= 8;}
      if (len) do {DO1(buf);} while (--len);
      return crc ^ 0xffffffffL;  // (instead of ~c for 64-bit machines)
    }


    static const int BUFFER_SIZE = 1024;
    public:
        static uint32_t ComputeCheckSumForString (Utf8CP string, size_t bufferSize);
        static uint32_t ComputeCheckSumForString (WCharCP string, size_t bufferSize);
        static uint32_t ComputeCheckSumForFile (WCharCP schemaFile);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Krischan.Eberle                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CheckSumHelper::ComputeCheckSumForString (Utf8CP string, size_t bufferSize)
    {
    return crc32 (0, (Byte*) string, bufferSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CheckSumHelper::ComputeCheckSumForString (WCharCP string, size_t bufferSize)
    {
    return crc32 (0, (Byte*) string, bufferSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        CheckSumHelper::ComputeCheckSumForFile (WCharCP schemaFile)
    {
    uint32_t checkSum = 0;
    BeFile file;
    if (BeFileStatus::Success != file.Open (schemaFile, BeFileAccess::Read))
        {
        BeAssert(false);
        return checkSum;
        }

    Byte buffer [BUFFER_SIZE];
    do
        {
        memset(buffer, 0, BUFFER_SIZE * sizeof(Byte));
        uint32_t bytesRead = 0;
        file.Read(buffer, &bytesRead, BUFFER_SIZE);
        if (bytesRead == 0)
            break;

        checkSum = crc32 (checkSum, buffer, bytesRead);
        } while(true);

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlFile (ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema from file: fileName='%ls'", ecSchemaXmlFile);
    schemaOut = NULL;

    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, ecSchemaXmlFile);
    if ((xmlStatus != BEXML_Success) || !xmlDom.IsValid())
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    AddFilePathToSchemaPaths(schemaContext, ecSchemaXmlFile);
    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForFile(ecSchemaXmlFile);

    status = ReadXml (schemaOut, *xmlDom.get(), checkSum, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (SCHEMA_READ_STATUS_Success != status)
        LOG.errorv (L"Failed to read XML file: %ls", ecSchemaXmlFile);
    else
        {
        //We have serialized a schema and its valid. Add its checksum
        timer.Stop();
        LOG.infov (L"Read (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(), schemaOut->m_classMap.size(), ecSchemaXmlFile);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaPtr&         schemaOut,
Utf8CP               ecSchemaXml,
ECSchemaReadContextR schemaContext
)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema read from string."); // mainly included for timing
    schemaOut = NULL;
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    size_t stringByteCount = strlen (ecSchemaXml) * sizeof(Utf8Char);

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecSchemaXml, stringByteCount);

    if (BEXML_Success != xmlStatus)
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForString (ecSchemaXml, stringByteCount);
    status = ReadXml (schemaOut, *xmlDom.get(), checkSum, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (SCHEMA_READ_STATUS_Success != status)
        {
        Utf8Char first200Bytes[201];

        BeStringUtilities::Strncpy (first200Bytes, ecSchemaXml, 200);
        first200Bytes[200] = '\0';
        LOG.errorv (L"Failed to read XML from string (1st 200 characters approx.): %hs", first200Bytes);
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(),
            schemaOut->m_classMap.size(), schemaOut->GetFullSchemaName().c_str());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaPtr&         schemaOut,
WCharCP              ecSchemaXml,
ECSchemaReadContextR schemaContext
)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema read from string."); // mainly included for timing
    schemaOut = NULL;
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    BeXmlStatus xmlStatus;
    size_t stringSize = wcslen (ecSchemaXml) * sizeof(WChar);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecSchemaXml, stringSize / sizeof (WChar));

    if (BEXML_Success != xmlStatus)
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForString(ecSchemaXml, stringSize);
    status = ReadXml (schemaOut, *xmlDom.get(), checkSum, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (SCHEMA_READ_STATUS_Success != status)
        {
        WChar first200Characters[201];
        wcsncpy (first200Characters, ecSchemaXml, 200);
        first200Characters[200] = L'\0';
        LOG.errorv (L"Failed to read XML from string (1st 200 characters): %ls", first200Characters);
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(),
            schemaOut->m_classMap.size(), schemaOut->GetFullSchemaName().c_str());
        }

    return status;
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
    ECSchemaReferenceListCR referencedSchemas = thisSchema.GetReferencedSchemas();
    return referencedSchemas.end() != referencedSchemas.find (potentiallyReferencedSchema.GetSchemaKey());
    }


#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlStream
(
ECSchemaP&                      schemaOut,
IStreamP                        ecSchemaXmlStream,
ECSchemaReadContextR schemaContext
)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);

    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (ECOBJECTS_STATUS_Success != status)
        LOG.errorv (L"Failed to read XML from stream");
    return status;
    return SCHEMA_READ_STATUS_FailedToParseXml;
    }
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (WStringR ecSchemaXml) const
    {
    ecSchemaXml.clear();

    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();

    SchemaWriteStatus status;
    if (SCHEMA_WRITE_STATUS_Success != (status = WriteXml (*xmlDom.get())))
        return status;

    xmlDom->ToString (ecSchemaXml, BeXmlDom::TO_STRING_OPTION_Default);

    return SCHEMA_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (Utf8StringR ecSchemaXml) const
    {
    ecSchemaXml.clear();

    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();

    SchemaWriteStatus status;
    if (SCHEMA_WRITE_STATUS_Success != (status = WriteXml (*xmlDom.get())))
        return status;

    xmlDom->ToString (ecSchemaXml, BeXmlDom::TO_STRING_OPTION_Default);

    return SCHEMA_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlFile
(
WCharCP ecSchemaXmlFile,
bool    utf16
) const
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();

    SchemaWriteStatus status;
    if (SCHEMA_WRITE_STATUS_Success != (status = WriteXml (*xmlDom.get())))
        return status;

    return (BEXML_Success == xmlDom->ToFile (ecSchemaXmlFile, (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted),
        utf16 ? BeXmlDom::FILE_ENCODING_Utf16 : BeXmlDom::FILE_ENCODING_Utf8)) ? SCHEMA_WRITE_STATUS_Success : SCHEMA_WRITE_STATUS_FailedToWriteFile;
    }

#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlStream
(
IStreamP ecSchemaXmlStream,
bool     utf16
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
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::FindAllSchemasInGraph (bvector<ECN::ECSchemaP>& allSchemas, bool includeRootSchema)
    {
    FindAllSchemasInGraph ((bvector<ECN::ECSchemaCP>&)allSchemas, includeRootSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::CollectAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
    {
    if (includeRootSchema)
        allSchemas.push_back (this);

    ECSchemaReferenceListCR referencedSchemas = this->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        if (!includeRootSchema)
            {
            if (this == iter->second.get())
                continue;
            }

        bvector<ECN::ECSchemaCP>::iterator it = std::find (allSchemas.begin(), allSchemas.end(), iter->second.get());

        if (it != allSchemas.end())
            {
            allSchemas.erase(it);
            allSchemas.push_back(iter->second.get());
            continue;
            }

        iter->second->CollectAllSchemasInGraph (allSchemas, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::FindAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
    {
    CollectAllSchemasInGraph (allSchemas, includeRootSchema);
    std::reverse(allSchemas.begin(), allSchemas.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaId ECSchema::GetId() const
    {
    BeAssert (0 != m_ecSchemaId);
    return m_ecSchemaId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::FindSchema (SchemaKeyCR schemaKey, SchemaMatchType matchType) const
    {
    if (this->GetSchemaKey().Matches (schemaKey, matchType))
        return this;

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP schema = iter->second->FindSchema (schemaKey, matchType);
        if (NULL != schema)
            return schema;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::FindSchemaP (SchemaKeyCR schemaKey, SchemaMatchType matchType)
    {
    return const_cast<ECSchemaP> (FindSchema(schemaKey, matchType));
    }

/////////////////////////////////////////////////////////////////////////////////////////
// IStandaloneEnablerLocater
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr  IStandaloneEnablerLocater::LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className)
    {
    return  _LocateStandaloneEnabler (schemaKey, className);
    }

DEFINE_KEY_METHOD(IStandaloneEnablerLocater)

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCache::~ECSchemaCache ()
    {
    m_schemas.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int                             ECSchemaCache::GetCount ()
    {
    return (int)m_schemas.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECSchemaLocater& ECSchemaCache::GetSchemaLocater()
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::AddSchema   (ECSchemaR ecSchema)
    {
    if (m_schemas.end() != m_schemas.find (ecSchema.GetSchemaKey()))
        return ECOBJECTS_STATUS_DuplicateSchema;

    bvector<ECSchemaP> schemas;
    ecSchema.FindAllSchemasInGraph(schemas, true);
    bool inserted = false;

    for (bvector<ECSchemaP>::const_iterator iter = schemas.begin(); iter != schemas.end(); ++iter)
        {
        bpair<SchemaMap::iterator, bool> result = m_schemas.insert(SchemaMap::value_type((*iter)->GetSchemaKey(), *iter));
        inserted |= result.second;
        }

    return inserted ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_DuplicateSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::DropSchema  (SchemaKeyCR ecSchemaKey)
    {
    SchemaMap::iterator iter = m_schemas.find (ecSchemaKey);
    if (iter == m_schemas.end())
        return ECOBJECTS_STATUS_SchemaNotFound;

    m_schemas.erase(iter);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::DropAllReferencesOfSchema(ECSchemaR schema)
    {
    for (SchemaMap::iterator iter = m_schemas.begin(); iter != m_schemas.end();)
        {
        bool removeSchema = false;
        bvector<ECSchemaP> schemas;
        iter->second->FindAllSchemasInGraph(schemas, true);
        for (bvector<ECSchemaP>::const_iterator refIter = schemas.begin(); refIter != schemas.end(); ++refIter)
            {
            if ((*refIter) == &schema)
                {
                removeSchema = true;
                break;
                }
            }

        if (removeSchema)
            iter = m_schemas.erase(iter);
        else
            ++iter;

        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                             ECSchemaCache::Clear ()
    {
    m_schemas.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::GetSchema   (SchemaKeyCR key)
    {
    return GetSchema(key, SCHEMAMATCHTYPE_Identical);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::GetSchema(SchemaKeyCR key, SchemaMatchType matchType)
    {
    SchemaMap::iterator iter;
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            iter = m_schemas.find (key);
            break;
            }
        default:
            {
            //Other cases the container is not sorted by the match type.
            iter = std::find_if(m_schemas.begin(), m_schemas.end(), SchemaKeyMatchPredicate(key, matchType));
            break;
            }
        }

    if (iter == m_schemas.end())
        return NULL;

    return iter->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaCache::_LocateSchema (SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return GetSchema(key, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCachePtr    ECSchemaCache::Create ()
    {
    return new ECSchemaCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECSchemaCache::GetSchemas (bvector<ECSchemaP>& schemas) const
    {
    schemas.clear();
    for (SchemaMap::const_iterator it = m_schemas.begin(); it != m_schemas.end(); it++)
        schemas.push_back (it->second.get());
    return schemas.size();
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaKeyCR ECSchema::GetSchemaKey ()const
    {
    return m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer& ECSchema::GetCustomAttributeContainer()
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaKey::ParseSchemaFullName (SchemaKeyR key, WCharCP schemaFullName)
    {
    return ECSchema::ParseSchemaFullName (key.m_schemaName, key.m_versionMajor, key.m_versionMinor, schemaFullName);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECGetChildFunctor
    {
    typedef bvector<ECSchemaCP> ChildCollection;
    ChildCollection operator () (ECSchemaCP schema) const
        {
        bvector<ECSchemaCP> schemas;
        ECSchemaReferenceListCR referencedSchemas = schema->GetReferencedSchemas();
        for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
            schemas.push_back(iter->second.get());

        return schemas;
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchema::AddingSchemaCausedCycles () const
    {
    ECGetChildFunctor fncTor;

    typedef SCCGraph <ECSchemaCP, ECGetChildFunctor> SchemaGraph;
    SchemaGraph graph (fncTor);

    SchemaGraph::SCCContext context;
    graph.StronglyConnect(this, context);

    bool hasCycles = false;
    for (SchemaGraph::SccNodes::const_iterator iter = context.m_components.begin(); iter != context.m_components.end(); ++iter)
        {
        if (1 != iter->size())
            {
            hasCycles = true;
            WString cycleString;
            for (SchemaGraph::NodeVector::const_iterator cycleIter = iter->begin(); cycleIter != iter->end(); ++cycleIter)
                {
                cycleString.append((*cycleIter)->m_node->m_key.GetFullSchemaName());
                cycleString.append(L"-->");
                }
            cycleString.append( (*iter->begin())->m_node->m_key.GetFullSchemaName());
            LOG.errorv (L"ECSchema '%ls' contains cycles %ls", m_key.GetFullSchemaName().c_str(), cycleString.c_str());

            break;
            }
        }

    return hasCycles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString         SchemaKey::GetFullSchemaName () const
    {
    WChar schemaName[512] = {0};
    BeStringUtilities::Snwprintf(schemaName, L"%ls.%02d.%02d", m_schemaName.c_str(), m_versionMajor, m_versionMinor);
    return schemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClassFinder
    {
    ECN::SchemaNameClassNamePair const& m_key;
    ECClassP&                          m_class;
    ECClassFinder (ECN::SchemaNameClassNamePair const& key, ECClassP& foundClass)
        :m_key(key), m_class(foundClass)
        {}

    bool operator () (ECSchemaR val)
        {
        if (0 != val.GetName().CompareTo(m_key.m_schemaName))
            return false;

        m_class = val.GetClassP(m_key.m_className.c_str());
        return NULL != m_class;
        }

    bool operator () (ECSchemaPtr const& val)
        {
        if (val.IsNull())
            return false;

        return (*this)(*val);
        }

    bool operator () (bpair<SchemaKey,ECSchemaPtr> const& val)
        {
        return (*this)(val.second);
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP        SchemaMapExact::FindClassP (ECN::SchemaNameClassNamePair const& classNamePair) const
    {
    ECClassP classInstance = NULL;
    ECClassFinder classFinder(classNamePair, classInstance);

    SchemaMapExact::const_iterator iter = std::find_if (begin(), end(), classFinder);
    return iter == end() ? NULL : classInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        ECSchema::ComputeSchemaXmlStringCheckSum(WCharCP str, size_t len)
    {
    return CheckSumHelper::ComputeCheckSumForString (str, len);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::ReComputeCheckSum ()
    {
    if (m_immutable)
        return;

    WString xmlStr;
    if (SCHEMA_WRITE_STATUS_Success != WriteToXmlString (xmlStr))
        return;

    m_key.m_checkSum = CheckSumHelper::ComputeCheckSumForString (xmlStr.c_str(), sizeof(WChar)* xmlStr.length());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::SetImmutable()
    {
    BeAssert(!m_immutable);
    ReComputeCheckSum();
    m_immutable = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int             SchemaKey::CompareByName (WString schemaName) const
    {
    int caseSensitive = wcscmp (m_schemaName.c_str(), schemaName.c_str());
    if (0 != caseSensitive && 0 == BeStringUtilities::Wcsicmp (m_schemaName.c_str(), schemaName.c_str()))
        {
        LOG.warningv (L"Schema name %ls and schema name %ls are different in case only.", m_schemaName.c_str(), schemaName.c_str());
        return 0;
        }

    return caseSensitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaKey::LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            if (0 != m_checkSum || 0 != rhs.m_checkSum)
                return m_checkSum < rhs.m_checkSum;
            //Fall through
            }
        case SCHEMAMATCHTYPE_Exact:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            if (m_versionMajor != rhs.m_versionMajor)
                return m_versionMajor < rhs.m_versionMajor;

            return m_versionMinor < rhs.m_versionMinor;
            break;
            }
        case SCHEMAMATCHTYPE_LatestCompatible:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            return m_versionMajor < rhs.m_versionMajor;
            }
        case SCHEMAMATCHTYPE_Latest: //Only compare by name
            {
            return CompareByName (rhs.m_schemaName) < 0;
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaKey::Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            if (0 != m_checkSum && 0 != rhs.m_checkSum)
                return m_checkSum == rhs.m_checkSum;
            //fall through
            }
        case SCHEMAMATCHTYPE_Exact:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionMajor == rhs.m_versionMajor && m_versionMinor == rhs.m_versionMinor;
        case SCHEMAMATCHTYPE_LatestCompatible:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionMajor == rhs.m_versionMajor && m_versionMinor >= rhs.m_versionMinor;
        case SCHEMAMATCHTYPE_Latest:
            return 0 == CompareByName (rhs.m_schemaName);
        default:
            return false;
        }
    }

static IECTypeAdapterContext::FactoryFn s_typeAdapterContextFactory;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void IECTypeAdapterContext::RegisterFactory (FactoryFn fn)  { s_typeAdapterContextFactory = fn; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECTypeAdapterContextPtr IECTypeAdapterContext::Create (ECPropertyCR prop, IECInstanceCR instance, uint32_t componentIndex)
    {
    return NULL != s_typeAdapterContextFactory ? s_typeAdapterContextFactory (prop, instance, componentIndex) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString QualifiedECAccessor::ToString() const
    {
    WString str;
    str.Sprintf (L"%ls:%ls:%ls", m_schemaName.c_str(), m_className.c_str(), m_accessString.c_str());
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QualifiedECAccessor::FromString (WCharCP str)
    {
    bvector<WString> tokens;
    BeStringUtilities::Split (str, L":", tokens);
    if (3 != tokens.size())
        return false;

    m_schemaName = tokens[0];
    m_className = tokens[1];
    m_accessString = tokens[2];

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QualifiedECAccessor::FromAccessString (ECEnablerCR enabler, WCharCP accessString)
    {
    ECValueAccessor va;
    if (ECValueAccessor::PopulateValueAccessor (va, enabler, accessString) && 0 < va.GetDepth() && nullptr != va[0].GetECProperty())
        {
        ECClassCR rootClass = va[0].GetECProperty()->GetClass();
        m_schemaName = rootClass.GetSchema().GetName();
        m_className = rootClass.GetName();
        m_accessString = accessString;
        return true;
        }
    else
        return false;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Ramanujam.Raman                 12/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//void IECClassLocater::RegisterClassLocater (IECClassLocaterR classLocater) 
//    {
//    s_registeredClassLocater = IECClassLocaterPtr (&classLocater);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      03/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//void IECClassLocater::UnRegisterClassLocater ()
//    {
//    s_registeredClassLocater = nullptr;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Ramanujam.Raman                 12/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//IECClassLocaterP IECClassLocater::GetRegisteredClassLocater() 
//    {
//    return s_registeredClassLocater.get();
//    }
END_BENTLEY_ECOBJECT_NAMESPACE



