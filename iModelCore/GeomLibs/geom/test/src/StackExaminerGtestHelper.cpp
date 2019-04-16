/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "StackExaminer.h"
#include "StackExaminerGtestHelper.h"

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
StackInfo :: StackInfo (DWORD64 sz, DWORD64 peak, std::string const& name)
    {
    m_size = sz;
    m_peak = peak;
    m_testName = name;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
std::string StackInfo :: ToString ()
    {
    std::stringstream s;
    s << "peak: " <<m_peak << ", " << m_testName.c_str(); 
    return std::string (s.str().c_str());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
StackExaminer :: StackExaminer()
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer :: OnTestStart (const ::testing::TestInfo& /* test_info */) 
    {
    reclaimUnusedStackPages ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer :: OnTestEnd (const ::testing::TestInfo& test_info) 
    {
    DWORD_PTR   pBase = NULL;
    DWORD64       size;
    DWORD64       peak;

    getCurrentThreadStackInfo (&pBase, &size, &peak);

    std::string name = std::string (test_info.test_case_name()) + "." + std::string (test_info.name());


    StackInfo info (size, peak, name);
    m_stacktable.insert(std::make_pair(peak, info));

    reclaimUnusedStackPages ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer :: DumpStackInfo ()
    {
    StackMap::iterator iter;
    for (iter = m_stacktable.begin(); iter != m_stacktable.end(); ++iter)
        {
        if (iter->second.m_peak > 35000)
            std::cout << "[ STACK RESULT ] " << iter->second.ToString().c_str() << "\n";
        }
    }

