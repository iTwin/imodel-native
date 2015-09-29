/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceAdaptersTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECInstanceAdaptersTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

ECInstanceAdaptersTestFixture::ECInstanceAdaptersTestFixture() : m_testProject(nullptr)
    {
    }

void ECInstanceAdaptersTestFixture::SetTestProject (std::unique_ptr<ECDbTestProject> testProject)
    {
    m_testProject = move (testProject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestProject& ECInstanceAdaptersTestFixture::GetTestProject () const
    {
    return _GetTestProject ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
ECDbTestProject& ECInstanceAdaptersTestFixture::_GetTestProject () const
    {
    return *m_testProject;
    }

std::unique_ptr<ECDbTestProject> ECInstanceAdaptersTestFixture::CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName)
    {
    Utf8String filePath;
    // Create and populate a sample project
        {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create (ecdbFileName, schemaECXmlFileName, false);
        filePath = ecdb.GetDbFileName();
        }

        //re-open the file so that we can determine the open mode
        auto testProject = std::unique_ptr<ECDbTestProject> (new ECDbTestProject ());
        testProject->Open (filePath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
        return move (testProject);
    }

END_ECDBUNITTESTS_NAMESPACE
