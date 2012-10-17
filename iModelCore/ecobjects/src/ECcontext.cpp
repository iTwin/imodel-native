/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECcontext.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects\ECContext.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchemaReadContext::GetStandardPaths (bvector<WString>& searchPaths)
    {
    BeFileName dllPath (ECFileUtilities::GetDllPath().c_str());
    if (0 == *dllPath.GetName())
        return false;
    
    dllPath.AppendSeparator();
    searchPaths.push_back (dllPath.GetName());
    
    dllPath.AppendToPath (L"ECSchemas");

    BeFileName standardPath = dllPath;
    standardPath.AppendToPath (L"Standard");
    standardPath.AppendSeparator();
    searchPaths.push_back (standardPath.GetName());

    BeFileName generalPath = standardPath;
    generalPath.AppendToPath (L"General");
    generalPath.AppendSeparator();
    searchPaths.push_back (generalPath.GetName());

    BeFileName libraryPath = standardPath;
    libraryPath.AppendToPath (L"LibraryUnits");
    libraryPath.AppendSeparator();
    searchPaths.push_back (libraryPath.GetName());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::ECSchemaReadContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    :
    m_standaloneEnablerLocater(enablerLocater),
    m_acceptLegacyImperfectLatestCompatibleMatch(acceptLegacyImperfectLatestCompatibleMatch)
    {
    m_locators.insert(SchemaLocatorKey (&m_knownSchemas, ReaderContext));
    
    bvector<WString> searchPaths;
    if (GetStandardPaths (searchPaths))
        {
        SchemaLocatorSet::iterator insertionPoint = m_locators.begin();
        int priority = StandardPaths;
        for (bvector<WString>::const_iterator iter = searchPaths.begin(); iter != searchPaths.end(); ++iter)
            m_searchPaths.insert(*iter);
        
        SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
        m_locators.insert(insertionPoint, SchemaLocatorKey (locator.get(), ++priority));
        m_ownedLocators.push_back(locator);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)   
                                                                                        { return new ECSchemaReadContext(enablerLocater, acceptLegacyImperfectLatestCompatibleMatch); }
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (bool acceptLegacyImperfectLatestCompatibleMatch) 
    { 
    return CreateContext (NULL, acceptLegacyImperfectLatestCompatibleMatch); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::SchemaLocatorSet::iterator  ECSchemaReadContext::GetHighestLocatorInRange (UInt32& priority)
    {
    UInt32 category = (priority - priority % CATEGORY_PARTITION_SIZE);
    SchemaLocatorKey dummy(NULL, category + CATEGORY_PARTITION_SIZE);
    
    //Find the last insertion point of external schema
    SchemaLocatorSet::iterator iter = std::upper_bound (m_locators.begin(), m_locators.end(), dummy);
    if (iter != m_locators.end() && iter->m_priority < dummy.m_priority)
        priority = iter->m_priority + 1;
    else
        {
        iter = m_locators.begin();
        priority = category + 1;
        }
    
    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECSchemaReadContext::AddExternalSchemaLocaters (bvector<ECObject::IECSchemaLocaterP> const& locators) 
    {
    UInt32 priority = External;
    SchemaLocatorSet::iterator iter =  GetHighestLocatorInRange (priority);
    
    for (bvector<ECObject::IECSchemaLocaterP>::const_iterator locatorIter = locators.begin(); locatorIter != locators.end(); ++locatorIter)
        iter = m_locators.insert(iter, SchemaLocatorKey(*locatorIter, ++priority));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ECSchemaReadContext::AddSchemaLocater (IECSchemaLocaterR locater)
    {
    UInt32 priority = UserSpace;
    SchemaLocatorSet::iterator iter = GetHighestLocatorInRange (priority);
    m_locators.insert (iter, SchemaLocatorKey(&locater, priority));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECSchemaReadContext::RemoveSchemaLocater (IECSchemaLocaterR locator)
    {
    for (SchemaLocatorSet::iterator iter = m_locators.begin(); iter != m_locators.end();)
        {
        if (iter->m_locator == &locator)
            iter = m_locators.erase(iter);
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

    UInt32 priority = UserSpace;
    SchemaLocatorSet::iterator iter = GetHighestLocatorInRange (priority);

    bvector<WString> pathVector;
    pathVector.push_back(pathStr.GetName());
    SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (pathVector);

    m_searchPaths.insert(pathStr.GetName());
    m_locators.insert (iter, SchemaLocatorKey (locator.get(), priority));
    m_ownedLocators.push_back(locator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void                        ECSchemaReadContext::SetFinalSchemaLocater (IECSchemaLocaterR locater) 
    {
    UInt32 priority = Final;
    SchemaLocatorSet::iterator iter = GetHighestLocatorInRange (priority);
    m_locators.insert (iter, SchemaLocatorKey(&locater, priority));
    }

IStandaloneEnablerLocaterP  ECSchemaReadContext::GetStandaloneEnablerLocater()       { return m_standaloneEnablerLocater;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::LocateSchema (SchemaKeyR key, SchemaMatchType matchType)
    {
    m_knownSchemaDirtyStack.push_back(false);
    
    ECSchemaPtr schema;
    for (SchemaLocatorSet::const_iterator iter = m_locators.begin(); iter != m_locators.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != iter->m_locator))
            continue;

        schema = iter->m_locator->LocateSchema(key, matchType, *this);
        if (schema.IsValid())
            break;

        if (m_knownSchemaDirtyStack.back())
            {
            schema = m_knownSchemas.LocateSchema (key, matchType, *this);
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
    for (SchemaLocatorSet::const_iterator iter = m_locators.begin(); iter != m_locators.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != iter->m_locator))
            continue;

        for (bset<SchemaMatchType>::const_iterator matchIter = matches.begin(); matchIter != matches.end(); ++matchIter)
            {
            schema = iter->m_locator->LocateSchema(key, *matchIter, *this); //Doing this will change m_knownSchemas
            if (schema.IsValid())
                break;

            if (m_knownSchemaDirtyStack.back())
                {
                for (bset<SchemaMatchType>::const_iterator kmatchIter = matches.begin(); matchIter != matches.end(); ++matchIter)
                    {
                    schema = m_knownSchemas.LocateSchema (key, *kmatchIter, *this);
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
struct ECSchemaBackedInstanceReadContext: public ECInstanceReadContext
    {
    private:
    ECSchemaCR                          m_schema;
    SchemaKey                           m_key;
    public:
    ECSchemaBackedInstanceReadContext(ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater)
        :m_schema(schema), ECInstanceReadContext(standaloneEnablerLocater, schema), m_key(schema.GetSchemaKey())
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
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext (ECSchemaCR schema, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {
    return new ECSchemaBackedInstanceReadContext (schema, standaloneEnablerLocater);
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
        :m_schemaReadContext(schemaReadContext), m_foundSchema (foundSchema), ECInstanceReadContext(schemaReadContext.GetStandaloneEnablerLocater(), fallBackSchema)
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
ECSchemaPtr     ECSchemaReadContext::GetFoundSchema (SchemaKeyR key, SchemaMatchType matchType)
    {
    return m_knownSchemas.GetSchema(key, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::AddSchema(ECSchemaR schema) 
    {
    if (NULL != m_knownSchemas.GetSchema(schema.GetSchemaKey()))
        return;

    return _AddSchema(schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::_AddSchema(ECSchemaR schema)
    {
    if (ECOBJECTS_STATUS_Success != m_knownSchemas.AddSchema(schema))
        return;

    for (bvector<bool>::iterator iter = m_knownSchemaDirtyStack.begin(); iter != m_knownSchemaDirtyStack.end(); ++iter)
        (*iter) = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::RemoveSchema(ECSchemaR schema)
    {
    m_knownSchemas.DropSchema(schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCacheR  ECSchemaReadContext::GetKnownSchemas ()
    {
    return m_knownSchemas;
    }
