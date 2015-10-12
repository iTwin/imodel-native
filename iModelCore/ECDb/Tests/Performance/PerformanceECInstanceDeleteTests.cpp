/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECInstanceDeleteTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceECInstanceDeleteTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

void PerformanceECInstanceDeleteTestsFixture::CopyDgnDb ()
    {
    BeFileName DgnDbFileCopy;
    BeFileName DgnDbFile;

    //Actual DgnDb File.
    BeTest::GetHost ().GetDocumentsRoot (DgnDbFile);
    DgnDbFile.AppendToPath (L"DgnDb");
    DgnDbFile.AppendToPath (L"Main.idgndb");

    //Copy of DgnDb File.
    BeTest::GetHost ().GetDocumentsRoot (DgnDbFileCopy);
    DgnDbFileCopy.AppendToPath (L"DgnDb");
    DgnDbFileCopy.AppendToPath (L"Main_Copy.idgndb");

    ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile (DgnDbFile, DgnDbFileCopy, false));

    if (DgnDbFile.DoesPathExist ())
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, m_testproject.Open (DgnDbFileCopy.GetNameUtf8 ().c_str (), Db::OpenParams (Db::OpenMode::ReadWrite)));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesFromDerivedClassesOfElementItem)
    {
    ECSqlStatement stmt;
    StopWatch timer;
    double ElapsedTime = 0.0;
    CopyDgnDb ();

    ECDbR ecdb = m_testproject.GetECDb ();
    ASSERT_TRUE (ecdb.IsDbOpen ());

    ECN::ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (schema != nullptr);

    ECN::ECClassCP ParentClass = ecdb.Schemas ().GetECClass ("dgn", "ElementItem");
    ASSERT_TRUE (ParentClass != nullptr);

    ASSERT_EQ (17, ecdb.Schemas ().GetDerivedECClasses (*ParentClass).size ());

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (879, stmt.GetValueInt (0));
    stmt.Finalize ();

    for (auto Class : ecdb.Schemas ().GetDerivedECClasses (*ParentClass))
        {
        Utf8StringCR SchemaPrefix = Class->GetSchema ().GetNamespacePrefix ();
        Utf8String ClassName = Class->GetName ();
        LOG.infov ("\n Class Name = %s \n", ClassName.c_str());

        Utf8String stat = "Delete FROM ";
        stat.append (SchemaPrefix);
        stat.append (".");
        stat.append (ClassName);
        timer.Start ();
        ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, stat.c_str ())) << "Prepare failed for " << stat.c_str ();;
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
        timer.Stop ();
        ElapsedTime += timer.GetElapsedSeconds ();
        stmt.Finalize ();
        }

    LOGTODB (TEST_DETAILS, ElapsedTime, "Sql Delete time");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesFromDerivedClassesOfElement)
    {

    ECSqlStatement stmt;
    StopWatch timer;
    double ElapsedTime = 0.0;
    CopyDgnDb ();

    ECDbR ecdb = m_testproject.GetECDb ();
    ASSERT_TRUE (ecdb.IsDbOpen ());

    ECN::ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (schema != nullptr);

    ECN::ECClassCP Class_Element = ecdb.Schemas ().GetECClass ("dgn", "Element");
    ASSERT_TRUE (Class_Element != nullptr);

    ASSERT_EQ (93, ecdb.Schemas ().GetDerivedECClasses (*Class_Element).size ());

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (1000, stmt.GetValueInt (0));
    stmt.Finalize ();

    for (auto DerivedClass : ecdb.Schemas ().GetDerivedECClasses (*Class_Element))
        {
        Utf8StringCR SchemaPrefix = DerivedClass->GetSchema ().GetNamespacePrefix ();
        Utf8String ClassName = DerivedClass->GetName ();
        LOG.infov ("\n Derived Class Name = %s \n", ClassName.c_str());

        Utf8String stat = "Delete FROM ";
        stat.append (SchemaPrefix);
        stat.append (".");
        stat.append (ClassName);
        timer.Start ();
        ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, stat.c_str ())) << "Prepare failed for " << stat.c_str ();;
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
        timer.Stop ();
        ElapsedTime += timer.GetElapsedSeconds ();
        stmt.Finalize ();
        }

    LOGTODB (TEST_DETAILS, ElapsedTime, "Sql Delete time");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesDirectlyFromElementItem)
    {

    ECSqlStatement stmt;
    StopWatch timer;
    CopyDgnDb ();

    ECDbR ecdb = m_testproject.GetECDb ();
    ASSERT_TRUE (ecdb.IsDbOpen ());

    ECN::ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (schema != nullptr);

    ECN::ECClassCP ParentClass = ecdb.Schemas ().GetECClass ("dgn", "ElementItem");
    ASSERT_TRUE (ParentClass != nullptr);

    ASSERT_EQ (17, ecdb.Schemas ().GetDerivedECClasses (*ParentClass).size ());

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (879, stmt.GetValueInt (0));
    stmt.Finalize ();

    Utf8String stat = "Delete FROM dgn.ElementItem";

    timer.Start ();
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, stat.c_str ())) << "Prepare failed for " << stat.c_str ();;
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
    timer.Stop ();
    stmt.Finalize ();

    double ElapsedTime = timer.GetElapsedSeconds ();
    LOGTODB (TEST_DETAILS, ElapsedTime, "Sql Delete time");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.ElementItem"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceDeleteTestsFixture, DeleteInstancesDirectlyFromElement)
    {
    ECSqlStatement stmt;
    StopWatch timer;
    CopyDgnDb ();

    ECDbR ecdb = m_testproject.GetECDb ();
    ASSERT_TRUE (ecdb.IsDbOpen ());

    ECN::ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (schema != nullptr);

    ECN::ECClassCP ParentClass = ecdb.Schemas ().GetECClass ("dgn", "Element");
    ASSERT_TRUE (ParentClass != nullptr);

    ASSERT_EQ (93, ecdb.Schemas ().GetDerivedECClasses (*ParentClass).size ());

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (1000, stmt.GetValueInt (0));
    stmt.Finalize ();

    Utf8String stat = "Delete FROM dgn.Element";

    timer.Start ();
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, stat.c_str ())) << "Prepare failed for " << stat.c_str ();;
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "Step failed for " << stat.c_str ();
    timer.Stop ();
    stmt.Finalize ();

    double ElapsedTime = timer.GetElapsedSeconds ();
    LOGTODB (TEST_DETAILS, ElapsedTime, "Sql Delete time");

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select COUNT(*) FROM dgn.Element"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }


END_ECDBUNITTESTS_NAMESPACE