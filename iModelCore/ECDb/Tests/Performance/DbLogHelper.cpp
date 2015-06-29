/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/DbLogHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DbLogHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

DbLogHelper::DbLogHelper()
    {
    BeFileName dbName;
    BeTest::GetHost().GetOutputRoot(dbName);
    BeSQLiteLib::Initialize(dbName);
    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    dbName.AppendToPath(processorArchitecture);
    dbName.AppendToPath(L"TestResults");
    if (!dbName.DoesPathExist())
        BeFileName::CreateNewDirectory (dbName.c_str());

    dbName.AppendToPath(L"PerformanceResults.ecdb");

    Initialize(dbName);
    }

DbLogHelper::DbLogHelper(BeFileNameR dbName)
    {
    Initialize(dbName);
    }

void DbLogHelper::Initialize(BeFileNameR dbName)
    {
    if (dbName.DoesPathExist())
        {
        auto dbOpenStat = m_db.OpenBeSQLiteDb(dbName.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ(BE_SQLITE_OK, dbOpenStat);
        }
    else
        {
        DbResult stat = m_db.CreateNewDb (dbName.GetNameUtf8().c_str ());
        ASSERT_EQ(BE_SQLITE_OK, stat);
        CreateTables();
        }
    }

void DbLogHelper::CreateTables()
    {
    AString ddl = "ResultId INTEGER PRIMARY KEY";
    ddl.append (", TestId INTEGER");
    ddl.append (", Date TEXT");
    ddl.append (", Runtime DOUBLE");

    m_db.CreateTable("Results", ddl.c_str());

    AString ddl2 = "TestId INTEGER PRIMARY KEY";
    ddl2.append(", Description TEXT");
    m_db.CreateTable("Tests", ddl2.c_str());
    }

int DbLogHelper::GetTestId(Utf8String testName)
    {
    Statement sqlStatement;
    Utf8PrintfString selectQuery("SELECT TestId FROM Tests WHERE Description=\"%s\"", testName.c_str());
    DbResult result = sqlStatement.Prepare(m_db, selectQuery);
    if (BE_SQLITE_OK != result)
        return -1;

    //sqlStatement.BindText(1, testName.c_str(), Statement::MakeCopy::No);
    int id = -1;
    result = sqlStatement.Step();
    if (BE_SQLITE_ROW == result)
        id = sqlStatement.GetValueInt(0);
    else
        {
        Statement insertStatement;
        Utf8CP insertQuery="INSERT INTO Tests(Description) VALUES(?)";
        result = insertStatement.Prepare(m_db, insertQuery);
        if (BE_SQLITE_OK != result)
            return -1;

        insertStatement.BindText(1, testName, Statement::MakeCopy::No);
        insertStatement.Step();
        m_db.SaveChanges();

        sqlStatement.Reset();
        result = sqlStatement.Step();
        if (BE_SQLITE_ROW == result)
            {
            id = sqlStatement.GetValueInt(0);
            }
        }
    return id;
    }

typedef bpair<Utf8String, double> T_TimerResultPair;
void DbLogHelper::LogResults(bmap<Utf8String, double> results)
    {
    Statement sqlStatement;
    Utf8CP insertQuery = "INSERT INTO Results(TestId, Date, Runtime) VALUES(?,?,?)";

    DbResult result = sqlStatement.Prepare(m_db, insertQuery);
    ASSERT_EQ(BE_SQLITE_OK, result);
    Utf8String dateTime = DateTime::GetCurrentTime().ToUtf8String();

    for (T_TimerResultPair const& pair : results)
        {
        sqlStatement.ClearBindings();
        int id = GetTestId(pair.first);
        if (-1 == id)
            continue;

        sqlStatement.BindInt(1, id);
        sqlStatement.BindText(2, dateTime.c_str(), Statement::MakeCopy::No);
        sqlStatement.BindDouble(3, pair.second);
        sqlStatement.Step();
        sqlStatement.Reset();
        }
    m_db.SaveChanges();
    }

END_ECDBUNITTESTS_NAMESPACE
