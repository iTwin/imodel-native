/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECContext.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static BeFileName s_rootDirectory;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECSchemaReadContext::Initialize(BeFileNameCR rootDirectory)
    {
    s_rootDirectory = rootDirectory;
    s_rootDirectory.AppendSeparator();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BeFileNameCR ECSchemaReadContext::GetHostAssetsDirectory()
    {
    return s_rootDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaReadContext::GetStandardPaths(bvector<WString>& searchPaths)
    {
    BeFileName rootDir (s_rootDirectory);
    if (0 == *rootDir.GetName())
        return false;

    rootDir.AppendSeparator();
    searchPaths.push_back(rootDir.GetName());

    rootDir.AppendToPath(EC_SCHEMAS_DIRECTORY);

    BeFileName standardPath = rootDir;
    standardPath.AppendToPath(EC_STANDARD_DIRECTORY);
    standardPath.AppendSeparator();
    searchPaths.push_back(standardPath.GetName());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::ECSchemaReadContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch, bool createConversionContext, bool includeFilesWithNoVerExt)
    : m_remapper(nullptr), m_standaloneEnablerLocater(enablerLocater), m_acceptLegacyImperfectLatestCompatibleMatch(acceptLegacyImperfectLatestCompatibleMatch), m_includeFilesWithNoVerExt(includeFilesWithNoVerExt)
    {
    m_knownSchemas = ECSchemaCache::Create();
    m_locaters.push_back(m_knownSchemas.get());

    m_userAddedLocatersCount = 0;
    m_searchPathLocatersCount = 0;

    bvector<WString> searchPaths;
    if (GetStandardPaths (searchPaths))
        {
        for (bvector<WString>::const_iterator iter = searchPaths.begin(); iter != searchPaths.end(); ++iter)
            m_searchPaths.insert(*iter);

        SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths, m_includeFilesWithNoVerExt);
        m_locaters.push_back(locator.get());
        m_searchPathLocatersCount++;
        m_ownedLocators.push_back(locator);
        }
    else
        LOG.warning("ECSchemaReadContext - Failed to get standard schema paths.  Use ECSchemaReadContext::Initialize with the path to the Assets directory so standard schemas can be successfully located.");

    if (createConversionContext)
        {
        BeFileName conversionSchemasDirectory(s_rootDirectory);
        conversionSchemasDirectory.AppendToPath(EC_SCHEMAS_DIRECTORY);
        conversionSchemasDirectory.AppendToPath(EC_V3CONVERSION_DIRECTORY);
        m_conversionSchemas = CreateContext(enablerLocater, acceptLegacyImperfectLatestCompatibleMatch, false, m_includeFilesWithNoVerExt);
        m_conversionSchemas->AddSchemaPath(conversionSchemasDirectory.c_str());
        m_conversionSchemas->AddSchemaLocater(*m_knownSchemas);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::ResolveClassName(Utf8StringR className, ECSchemaCR schema) const
    {
    if (nullptr != m_remapper)
        m_remapper->ResolveClassName(className, schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr ECSchemaReadContext::CreateContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch, bool createConversionContext, bool includeFilesWithNoVerExt)
    {
    return new ECSchemaReadContext(enablerLocater, acceptLegacyImperfectLatestCompatibleMatch, createConversionContext, includeFilesWithNoVerExt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr ECSchemaReadContext::CreateContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    {
    return CreateContext(enablerLocater, acceptLegacyImperfectLatestCompatibleMatch, true, false /*=includeFilesWithNoVerExt*/);
    }
ECSchemaReadContextPtr ECSchemaReadContext::CreateContext(bool acceptLegacyImperfectLatestCompatibleMatch, bool includeFilesWithNoVerExt)
    {
    return CreateContext(nullptr, acceptLegacyImperfectLatestCompatibleMatch, true, includeFilesWithNoVerExt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::AddSchemaLocaters(bvector<ECN::IECSchemaLocaterP> const& locators)
    {
    for (bvector<ECN::IECSchemaLocaterP>::const_iterator locatorIter = locators.begin(); locatorIter != locators.end(); ++locatorIter)
        m_locaters.insert(m_locaters.begin() + ++m_userAddedLocatersCount, *locatorIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::RemoveSchemaLocater(IECSchemaLocaterR locator)
    {
    for (bvector<IECSchemaLocaterP>::iterator iter = m_locaters.begin(); iter != m_locaters.end();)
        {
        if (*iter == &locator)
            {
            iter = m_locaters.erase(iter);
            m_userAddedLocatersCount--;
            }
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::AddSchemaPath(WCharCP path)
    {
    BeFileName pathStr(path);
    pathStr.AppendSeparator();

    if (m_searchPaths.end() != m_searchPaths.find (pathStr.GetName()))
        return;

    bvector<WString> pathVector;
    pathVector.push_back(pathStr.GetName());
    SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(pathVector, m_includeFilesWithNoVerExt);

    m_searchPaths.insert(pathStr.GetName());
    m_locaters.insert(m_locaters.begin() + m_userAddedLocatersCount + ++m_searchPathLocatersCount, locator.get());
    m_ownedLocators.push_back(locator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECSchemaReadContext::LocateSchema(SchemaKeyR key, SchemaMatchType matchType)
    {
    m_knownSchemaDirtyStack.push_back(false);

    ECSchemaPtr schema;
    for (auto const& locater : m_locaters)
        {
        if (!EXPECTED_CONDITION (nullptr != locater))
            continue;

        schema = locater->LocateSchema(key, matchType, *this);
        if (schema.IsValid())
            break;

        if (m_knownSchemaDirtyStack.back())
            {
            schema = m_knownSchemas->LocateSchema (key, matchType, *this);
            m_knownSchemaDirtyStack.back() = false;
            }

        if (schema.IsValid())
            break;
        }

    m_knownSchemaDirtyStack.pop_back();
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaReadContext::AddSchema(ECSchemaR schema)
    {
    if (nullptr != m_knownSchemas->GetSchema(schema.GetSchemaKey()))
        return ECObjectsStatus::DuplicateSchema;

    _AddSchema(schema);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::_AddSchema(ECSchemaR schema)
    {
    if (ECObjectsStatus::Success != m_knownSchemas->AddSchema(schema))
        return;

    for (bvector<bool>::iterator iter = m_knownSchemaDirtyStack.begin(); iter != m_knownSchemaDirtyStack.end(); ++iter)
        (*iter) = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchemaReadContext::AddConversionSchema(ECSchemaR schema)
    {
    if (m_conversionSchemas.IsValid())
        return m_conversionSchemas->AddSchema(schema);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaPtr ECSchemaReadContext::LocateConversionSchemaFor(Utf8CP schemaName, int versionRead, int versionMinor)
    {
    ECSchemaPtr conversionSchema;
    if (m_conversionSchemas.IsValid())
        {
        m_conversionSchemas->SetSkipValidation(GetSkipValidation());
        m_conversionSchemas->SetCalculateChecksum(GetCalculateChecksum());
        Utf8String conversionSchemaName(schemaName);
        conversionSchemaName += "_V3Conversion";
        SchemaKey key(conversionSchemaName.c_str(), versionRead, versionMinor);
        conversionSchema = m_conversionSchemas->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        }
    
    return conversionSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECSchemaReadContext::AddAliasIfSchemaToPrune(Utf8StringCR schemaName, Utf8StringCR schemaToPrune, Utf8StringCR alias)
    {
    if (GetSchemasToPrune().end() != std::find(GetSchemasToPrune().begin(), GetSchemasToPrune().end(), schemaToPrune))
        {
        auto schemaAliases = m_schemasToPruneAliases.find(schemaName);
        if (schemaAliases == m_schemasToPruneAliases.end())
            m_schemasToPruneAliases[schemaName] = bvector<Utf8String> {alias};
        else
            schemaAliases->second.push_back(alias);
        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECSchemaReadContext::IsAliasToPrune(Utf8StringCR schemaName, Utf8StringCR alias) const
    {
    if (alias.empty())
        return false;
    auto schemaAliases = m_schemasToPruneAliases.find(schemaName);
    if (schemaAliases == m_schemasToPruneAliases.end())
        return false;
    auto toPruneMatch = std::find_if(schemaAliases->second.begin(), schemaAliases->second.end(), [&](const auto& toPrune){return toPrune.EqualsIAscii(alias);});
    return toPruneMatch != schemaAliases->second.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECSchemaReadContext::ClearAliasesToPruneForSchema(Utf8StringCR schemaName)
    {
    auto it = m_schemasToPruneAliases.find(schemaName);
    if (it != m_schemasToPruneAliases.end())
        m_schemasToPruneAliases.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaBackedInstanceReadContext: public ECInstanceReadContext
{
private:
    ECSchemaCR m_schema;
    SchemaKey m_key;
public:
    ECSchemaBackedInstanceReadContext(ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater, IPrimitiveTypeResolver const* typeResolver)
        : m_schema(schema), ECInstanceReadContext(standaloneEnablerLocater, schema, typeResolver), m_key(schema.GetSchemaKey())
        { }

    virtual ECSchemaCP _FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const override
        {
        return key.Matches(m_key, matchType) ? &m_schema : nullptr;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext(ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater, IPrimitiveTypeResolver const* typeResolver)
    {
    return new ECSchemaBackedInstanceReadContext (schema, standaloneEnablerLocater, typeResolver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaReadContextBackedInstanceReadContext: public ECInstanceReadContext
{
private:
    ECSchemaReadContextR    m_schemaReadContext;
    ECSchemaPtr*            m_foundSchema;
public:
    ECSchemaReadContextBackedInstanceReadContext(ECSchemaReadContextR schemaReadContext, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema)
        :m_schemaReadContext(schemaReadContext), m_foundSchema (foundSchema), ECInstanceReadContext(schemaReadContext.GetStandaloneEnablerLocater(), fallBackSchema, nullptr)
        { }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ECSchemaCP _FindSchemaCP(SchemaKeyCR keyIn, SchemaMatchType matchType) const
        {
        SchemaKey key(keyIn);
        ECSchemaPtr schema = m_schemaReadContext.LocateSchema (key, matchType);
        if (schema.IsNull())
            return nullptr;

        if (nullptr != m_foundSchema)
            (*m_foundSchema) = schema;

        return schema.get();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext(ECSchemaReadContextR context, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema)
    {
    return new ECSchemaReadContextBackedInstanceReadContext (context, fallBackSchema, foundSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceReadContext::_CreateStandaloneInstance(ECClassCR ecClass)
    {
    if (nullptr != ecClass.GetRelationshipClassCP())
        {
        StandaloneECRelationshipEnablerPtr enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*(ecClass.GetRelationshipClassCP()));
        return enabler->CreateRelationshipInstance();
        }
    StandaloneECEnablerPtr standaloneEnabler = ecClass.GetDefaultStandaloneEnabler();
    return standaloneEnabler->CreateInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceReadContext::CreateStandaloneInstance(ECClassCR ecClass)
    {
    return _CreateStandaloneInstance (ecClass);
    }


END_BENTLEY_ECOBJECT_NAMESPACE
