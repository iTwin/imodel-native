/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/TestFixture.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_EC_NAMESPACE
  
#define MAX_INTERNAL_INSTANCES  0
#define MAX_INTERNAL_SCHEMAS    0

#define DEBUG_ECSCHEMA_LEAKS
#define DEBUG_IECINSTANCE_LEAKS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::SetUp ()
    {
    IECInstance::Debug_ResetAllocationStats();
    ECSchema::Debug_ResetAllocationStats();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::TearDown ()
    {
#if defined (DEBUG_ECSCHEMA_LEAKS)
    ECSchema::Debug_DumpAllocationStats(L"PostTest");
#endif

#if defined (DEBUG_IECINSTANCE_LEAKS)
    IECInstance::Debug_DumpAllocationStats(L"PostTest");
#endif

    if (_WantSchemaLeakDetection())
        TestForECSchemaLeaks(); 

    if (_WantInstanceLeakDetection())
        TestForIECInstanceLeaks(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECTestFixture::TestForECSchemaLeaks ()
    {
    int numLiveSchemas = 0;

    ECSchema::Debug_GetAllocationStats (&numLiveSchemas, NULL, NULL);
    
    if (numLiveSchemas > MAX_INTERNAL_SCHEMAS)
        {
        char message[1024];
        sprintf (message, "TestForECSchemaLeaks found that there are %d Schemas still alive. Anything more than %d is flagged as an error!\n", 
            numLiveSchemas, MAX_INTERNAL_SCHEMAS);

        std::vector<bwstring> schemaNamesToExclude;
        ECSchema::Debug_ReportLeaks (schemaNamesToExclude);

        EXPECT_TRUE (numLiveSchemas <= MAX_INTERNAL_SCHEMAS) << message;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECTestFixture::TestForIECInstanceLeaks ()
    {
    int numLiveInstances = 0;

    IECInstance::Debug_GetAllocationStats (&numLiveInstances, NULL, NULL);
    
    if (numLiveInstances > MAX_INTERNAL_INSTANCES)
        {
        char message[1024];
        sprintf (message, "TestForIECInstanceLeaks found that there are %d IECInstances still alive. Anything more than %d is flagged as an error!\n", 
            numLiveInstances, MAX_INTERNAL_INSTANCES);

        std::vector<bwstring> classNamesToExclude;
        IECInstance::Debug_ReportLeaks (classNamesToExclude);

        EXPECT_TRUE (numLiveInstances <= MAX_INTERNAL_INSTANCES) << message;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void*    getDLLInstance ()
    {
    MEMORY_BASIC_INFORMATION    mbi;
    if (VirtualQuery ((void*)&getDLLInstance, &mbi, sizeof mbi))
        return mbi.AllocationBase;

    return 0;
    }
 
std::wstring ECTestFixture::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ECTestFixture::GetTestDataPath(const wchar_t *dataFile)
    {
    if (s_dllPath.empty())
        {
        HINSTANCE ecobjectsHInstance = (HINSTANCE) getDLLInstance();
        wchar_t strExePath [MAX_PATH];
        if (0 == (GetModuleFileNameW (ecobjectsHInstance, strExePath, MAX_PATH)))
            return L"";
            
        wchar_t executingDirectory[_MAX_DIR];
        wchar_t executingDrive[_MAX_DRIVE];
        _wsplitpath(strExePath, executingDrive, executingDirectory, NULL, NULL);
        wchar_t filepath[_MAX_PATH];
        _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
        s_dllPath = filepath;
        }
        
    std::wstring testData(s_dllPath);
    testData.append(dataFile);
    return testData;
    
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
TestSchemaOwner::~TestSchemaOwner ()
    {
    if (0 == m_schemas.size())
        return;

    for each (ECSchemaP ecSchema in m_schemas)
        ECSchema::DestroySchema (ecSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus TestSchemaOwner::AddSchema (ECSchemaR ecSchema)
    {
    m_schemas.push_back (&ecSchema);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus TestSchemaOwner::DropSchema (ECSchemaR ecSchema)
    {
    bvector<ECSchemaP>::iterator iter = std::find(m_schemas.begin(), m_schemas.end(), &ecSchema);
    if (iter == m_schemas.end())
        return ECOBJECTS_STATUS_SchemaNotFound;

    ECSchema::DestroySchema (*iter);

    m_schemas.erase(iter);

    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE

