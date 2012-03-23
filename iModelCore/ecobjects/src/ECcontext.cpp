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
    
    searchPaths.push_back (dllPath.GetName());
    
    dllPath.AppendToPath (L"ECSchemas");

    BeFileName standardPath = dllPath;
    standardPath.AppendToPath (L"Standard");
    searchPaths.push_back (standardPath.GetName());

    BeFileName generalPath = standardPath;
    generalPath.AppendToPath (L"General");
    searchPaths.push_back (generalPath.GetName());

    BeFileName libraryPath = standardPath;
    libraryPath.AppendToPath (L"LibraryUnits");
    searchPaths.push_back (libraryPath.GetName());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::ECSchemaReadContext(IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    :
    m_standaloneEnablerLocater(enablerLocater), m_hideSchemasFromLeakDetection (false), 
    m_acceptLegacyImperfectLatestCompatibleMatch(acceptLegacyImperfectLatestCompatibleMatch)
    {
    m_locators.insert(SchemaLocatorKey (&m_knownSchemas, ReaderContext));
    
    bvector<WString> searchPaths;
    if (GetStandardPaths (searchPaths))
        {
        SchemaLocatorSet::iterator insertionPoint = m_locators.begin();
        int priority = StandardPaths;
        for (bvector<WString>::const_iterator iter = searchPaths.begin(); iter != searchPaths.end(); ++iter)
            {
            bvector<WString> path; 
            path.push_back(*iter);
            SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (path);
            m_searchPathLocators[*iter] = locator;
            insertionPoint= m_locators.insert(insertionPoint, SchemaLocatorKey (locator.get(), ++priority));
            }
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
    if (iter != m_locators.end())
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
void  ECSchemaReadContext::AddExternalSchemaLocaters (bvector<EC::IECSchemaLocaterP> const& locators) 
    {
    UInt32 priority = External;
    SchemaLocatorSet::iterator iter =  GetHighestLocatorInRange (priority);
    
    for (bvector<EC::IECSchemaLocaterP>::const_iterator locatorIter = locators.begin(); locatorIter != locators.end(); ++locatorIter)
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
void  ECSchemaReadContext::AddSchemaPath (WCharCP path) 
    {
    WString pathStr(path);
    if (m_searchPathLocators.end() != m_searchPathLocators.find (pathStr))
        return;

    UInt32 priority = UserSpace;
    SchemaLocatorSet::iterator iter = GetHighestLocatorInRange (priority);

    bvector<WString> pathVector;
    pathVector.push_back(pathStr);
    SearchPathSchemaFileLocaterPtr locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (pathVector);

    m_searchPathLocators[pathStr] = locator;

    m_locators.insert (iter, SchemaLocatorKey (locator.get(), priority));
    }

void  ECSchemaReadContext::HideSchemasFromLeakDetection ()                   { m_hideSchemasFromLeakDetection = true; }

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
bool                        ECSchemaReadContext::GetHideSchemasFromLeakDetection()   { return m_hideSchemasFromLeakDetection;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::LocateSchema (SchemaKeyR key, SchemaMatchType matchType)
    {
    for (SchemaLocatorSet::const_iterator iter = m_locators.begin(); iter != m_locators.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != iter->m_locator))
            continue;

        ECSchemaPtr schema = iter->m_locator->LocateSchema(key, matchType, NULL);
        if (schema.IsValid())
            return schema;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::LocateSchema (SchemaKeyR key, bset<SchemaMatchType> const& matches)
    {
    for (SchemaLocatorSet::const_iterator iter = m_locators.begin(); iter != m_locators.end(); ++iter)
        {
        if ( ! EXPECTED_CONDITION (NULL != iter->m_locator))
            continue;

        for (bset<SchemaMatchType>::const_iterator matchIter = matches.begin(); matchIter != matches.end(); ++matchIter)
            {
            ECSchemaPtr schema = iter->m_locator->LocateSchema(key, *matchIter, NULL);
            if (schema.IsValid())
                return schema;
            }
        }
    
    return NULL;
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
        :m_schema(schema), ECInstanceReadContext(standaloneEnablerLocater), m_key(schema.GetSchemaKey())
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
    ECSchemaPtr&            m_foundSchema;
    public:
    ECSchemaReadContextBackedInstanceReadContext(ECSchemaReadContextR schemaReadContext,  ECSchemaPtr& foundSchema, IStandaloneEnablerLocaterP standaloneEnablerLocater)
        :m_schemaReadContext(schemaReadContext), m_foundSchema (foundSchema), ECInstanceReadContext(standaloneEnablerLocater)
        {
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    10/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ECSchemaCP               _FindSchemaCP(SchemaKeyCR keyIn, SchemaMatchType matchType) const
        {
            //WIP_FUSION: Do we want to check for mismatches between the supplied schema name/version and m_fullSchemaName from the instance
        if (m_foundSchema.IsValid())
            return m_foundSchema.get();
        
        
        SchemaKey key(keyIn);
        ECSchemaPtr schema = m_schemaReadContext.LocateSchema (key, matchType);
        if (schema.IsNull())
            return NULL;

        m_foundSchema = schema;
        return m_foundSchema.get();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceReadContextPtr ECInstanceReadContext::CreateContext (ECSchemaReadContextR context, ECSchemaPtr& foundSchema, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {
    return new ECSchemaReadContextBackedInstanceReadContext (context, foundSchema, standaloneEnablerLocater);
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaReadContext::GetFoundSchema (SchemaKeyR key, SchemaMatchType matchType)
    {
    return m_knownSchemas.LocateSchema(key, matchType, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::AddSchema(ECSchemaR schema)
    {
    m_knownSchemas.AddSchema(schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchemaReadContext::RemoveSchema(ECSchemaR schema)
    {
    m_knownSchemas.DropSchema(schema);
    }