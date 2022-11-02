/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <map>
#include <windows.h>
#include <gtest\gtest.h>
#include <string>

/*================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StackInfo 
{
    DWORD64 m_size;
    DWORD64 m_peak;
    std::string m_testName;
    StackInfo (DWORD64 sz, DWORD64 peak, std::string const& name);
    std::string ToString ();
};

/*================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class StackExaminer : public ::testing::EmptyTestEventListener
{
protected:
    typedef  std::multimap <DWORD64,StackInfo> StackMap;
    StackMap m_stacktable;

public:
    StackExaminer();
    virtual void OnTestStart (const ::testing::TestInfo& test_info);
    virtual void OnTestEnd (const ::testing::TestInfo& test_info);
    void DumpStackInfo ();
};
