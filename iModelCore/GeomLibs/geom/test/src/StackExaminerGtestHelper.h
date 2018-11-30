/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/test/src/StackExaminerGtestHelper.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <map>
#include <windows.h>
#include <gtest\gtest.h>
#include <string>

/*================================================================================**//**
* @bsiclass                                                     KevinNyman      04/10
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
* @bsiclass                                                     KevinNyman      03/10
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
