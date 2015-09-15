/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECcontext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECContext.h>

USING_NAMESPACE_BENTLEY_EC

static BeFileName s_rootDirectory;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     06/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void            ECSchemaReadContext::Initialize (BeFileNameCR rootDirectory)
    {
    s_rootDirectory = rootDirectory;
    s_rootDirectory.AppendSeparator();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     10/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BeFileNameCR ECSchemaReadContext::GetHostAssetsDirectory ()
    {
    return s_rootDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchemaReadContext::GetStandardPaths (bvector<WString>& searchPaths)
    {
    BeFileName rootDir (s_rootDirectory);
    if (0 == *rootDir.GetName())
        return false;

    rootDir.AppendSeparator();
    searchPaths.push_back (rootDir.GetName());

    rootDir.AppendToPath (L"ECSchemas");

    BeFileName standardPath = rootDir;
    standardPath.AppendToPath (L"Standard");
    standardPath.AppendSeparator();
    searchPaths.push_back (standardPath.GetName());

#if defined (LEGACY_ECSCHEMAS_FOLDER_STRUCTURE)
    // For Graphite, there are no subfolders under "Standard"
    BeFileName generalPath = standardPath;
    generalPath.AppendToPath (L"General");
    generalPath.AppendSeparator();
    searchPaths.push_back (generalPath.GetName());

    BeFileName libraryPath = standardPath;
    libraryPath.AppendToPath (L"LibraryUnits");
    libraryPath.AppendSeparator();
    searchPaths.push_back (libraryPath.GetName());
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::ECSchemaReadContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    :
    m_standaloneEnablerLocater(enablerLocater),
    m_acceptLegacyImperfectLatestCompatibleMatch(acceptLegacyImperfectLatestCompatibleMatch),
    m_remapper (nullptr)
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

        SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
        m_locaters.push_back(locator.get());
        m_searchPathLocatersCount++;
        m_ownedLocators.push_back(locator);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::ResolveClassName (Utf8StringR className, ECSchemaCR schema) const
    {
    if (nullptr != m_remapper)
        m_remapper->ResolveClassName (className, schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    {
    return new ECSchemaReadContext(enablerLocater, acceptLegacyImperfectLatestCompatibleMatch);
    }
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (bool acceptLegacyImperfectLatestCompatibleMatch)
    {
    return CreateContext (NULL, acceptLegacyImperfectLatestCompatibleMatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECSchemaReadContext::AddSchemaLocaters (bvector<ECN::IECSchemaLocaterP> const& locators)
    {
    for (bvector<ECN::IECSchemaLocaterP>::const_iterator locatorIter = locators.begin(); locatorIter != locators.end(); ++locatorIter)
        m_locaters.insert(m_locaters.begin() + ++m_userAddedLocatersCount, *locatorIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECSchemaReadContext::AddSchemaLocater (IECSchemaLocaterR locater)
    {
    m_locaters.insert (m_locaters.begin() + ++m_userAddedLocatersCount, &locater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECSchemaReadContext::RemoveSchemaLocater (IECSchemaLocaterR locator)
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECSchemaReadContext::AddSchemaPath (WCharCP path)
    {
    BeFileName pathStr(path);
    pathStr.AppendSeparator();

    if (m_searchPaths.end() != m_searchPaths.find (pathStr.GetName()))
        return;

    bvector<WString> pathVector;
    pathVector.push_back(pathStr.GetName());
    SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (pathVector);

    m_searchPaths.insert(pathStr.GetName());
    m_locaters.insert (m_locaters.begin() + m_userAddedLocatersCount + ++m_searchPathLocatersCount, locator.get());
    m_ownedLocators.push_back(locator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaReadContext::AddCulture(WCharCP culture)
    {
    m_cultureStrings.push_back(WString(culture));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<WString>* ECSchemaReadContext::GetCultures()
    {
    return &m_cultureStrings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void                        ECSchemaReadContext::SetFinalSchemaLocater (IECSchemaLocaterR locater)
    {
    m_locaters.push_back (&locater);
    }

IStandaloneEnablerLocaterP  ECSchemaReadContext::GetStandaloneEnablerLocater()       { return m_standaloneEnablerLocater;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::LocateSchema (SchemaKeyR key, SchemaMatchType matchType)
    {
    m_knownSchemaDirtyStack.push_back(false);

    ECSchemaPtr schema;
    for (bvector<IECSchemaLocaterP>::const_iterator iter = m_locaters.begin(); iter != m_locaters.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != *iter))
            continue;

        schema = (*iter)->LocateSchema(key, matchType, *this);
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::LocateSchema (SchemaKeyR key, bset<SchemaMatchType> const& matches)
    {
    m_knownSchemaDirtyStack.push_back(false);

    ECSchemaPtr schema;
    for (bvector<IECSchemaLocaterP>::const_iterator iter = m_locaters.begin(); iter != m_locaters.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != *iter))
            continue;

        for (bset<SchemaMatchType>::const_iterator matchIter = matches.begin(); matchIter != matches.end(); ++matchIter)
            {
            schema = (*iter)->LocateSchema(key, *matchIter, *this); //Doing this will change m_knownSchemas
            if (schema.IsValid())
                break;

            if (m_knownSchemaDirtyStack.back())
                {
                for (bset<SchemaMatchType>::const_iterator kmatchIter = matches.begin(); matchIter != matches.end(); ++matchIter)
                    {
                    schema = m_knownSchemas->LocateSchema (key, *kmatchIter, *this);
                    if (schema.IsValid())
                        break;
                    }
                m_knownSchemaDirtyStack.back() = false;
                }

            if (schema.IsValid())
                break;
            }

        if (schema.IsValid())
            break;
        }

    m_knownSchemaDirtyStack.pop_back();
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCacheR ECSchemaReadContext::GetCache ()
    {
    return *m_knownSchemas;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaBackedInstanceReadContext: public ECInstanceReadContext
    {
    private:
    ECSchemaCR                          m_schema;
    SchemaKey                           m_key;
    public:
    ECSchemaBackedInstanceReadContext(ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater, IPrimitiveTypeResolver const* typeResolver)
        :m_schema(schema), ECInstanceReadContext(standaloneEnablerLocater, schema, typeResolver), m_key(schema.GetSchemaKey())
        {
        }

    virtual ECSchemaCP               _FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const override
        {
        return key.Matches(m_key, matchType) ? &m_schema : NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext (ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater, IPrimitiveTypeResolver const* typeResolver)
    {
    return new ECSchemaBackedInstanceReadContext (schema, standaloneEnablerLocater, typeResolver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaReadContextBackedInstanceReadContext: public ECInstanceReadContext
    {
    private:
    ECSchemaReadContextR    m_schemaReadContext;
    ECSchemaPtr*            m_foundSchema;
    public:
    ECSchemaReadContextBackedInstanceReadContext(ECSchemaReadContextR schemaReadContext,  ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema)
        :m_schemaReadContext(schemaReadContext), m_foundSchema (foundSchema), ECInstanceReadContext(schemaReadContext.GetStandaloneEnablerLocater(), fallBackSchema, NULL)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    10/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ECSchemaCP               _FindSchemaCP(SchemaKeyCR keyIn, SchemaMatchType matchType) const
        {
        SchemaKey key(keyIn);
        ECSchemaPtr schema = m_schemaReadContext.LocateSchema (key, matchType);
        if (schema.IsNull())
            return NULL;

        if (NULL != m_foundSchema)
            (*m_foundSchema) = schema;

        return schema.get();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext (ECSchemaReadContextR context, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema)
    {
    return new ECSchemaReadContextBackedInstanceReadContext (context, fallBackSchema, foundSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceReadContext::_CreateStandaloneInstance (ECClassCR ecClass)
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
* @bsimethod                                                    CaseyMullen     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceReadContext::CreateStandaloneInstance (ECClassCR ecClass)
    {
    return _CreateStandaloneInstance (ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ColinKerr     08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR      ECInstanceReadContext::GetFallBackSchema ()
    {
    return m_fallBackSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::GetFoundSchema (SchemaKeyCR key, SchemaMatchType matchType)
    {
    return m_knownSchemas->GetSchema(key, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus   ECSchemaReadContext::AddSchema(ECSchemaR schema)
    {
    if (NULL != m_knownSchemas->GetSchema(schema.GetSchemaKey()))
        return ECOBJECTS_STATUS_DuplicateSchema;

    _AddSchema(schema);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::_AddSchema(ECSchemaR schema)
    {
    if (ECOBJECTS_STATUS_Success != m_knownSchemas->AddSchema(schema))
        return;

    for (bvector<bool>::iterator iter = m_knownSchemaDirtyStack.begin(); iter != m_knownSchemaDirtyStack.end(); ++iter)
        (*iter) = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::RemoveSchema(ECSchemaR schema)
    {
    m_knownSchemas->DropAllReferencesOfSchema(schema);
    }
