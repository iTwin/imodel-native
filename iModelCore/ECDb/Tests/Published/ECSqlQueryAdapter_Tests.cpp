/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlQueryAdapter_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Adeel.Shoukat    09/13
//=======================================================================================    
struct ECSqlQueryAdapter : public ::testing::Test
    {
    public:
        Db m_TestResultDb;
        ECDb m_QueryDb;
        ECSqlStatement stmt;
        DbResult dbOpenStat ;
        BeFileName dir;

        ECSqlQueryAdapter()
            {
            }
//=======================================================================================    
// @bsiclass                                                 Adeel.Shoukat    09/13
//=======================================================================================   
void openTestDb (WString dbName)
    {
    if (m_TestResultDb.IsDbOpen())
        {
        stmt.Finalize();
        m_TestResultDb.CloseDb();
        }                        

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    BeSQLiteLib::Initialize (temporaryDir);

    //BeTest::GetHost().GetOutputRoot(dir);
    BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(L"DgnDb");
    dir.AppendToPath (dbName.c_str());
    if(dir.DoesPathExist())
        {
        dbOpenStat = m_TestResultDb.OpenBeSQLiteDb (dir.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, dbOpenStat);
        }
    }
//=======================================================================================    
// @bsimethod                                                 Adeel.Shoukat    09/13
//=======================================================================================   
void OpenInputDb (Utf8CP dbName)
    {
	if (m_QueryDb.IsDbOpen())
        {
        stmt.Finalize();
        m_QueryDb.CloseDb();
        }
        BeTest::GetHost().GetDocumentsRoot(dir);
        dir.AppendToPath(L"DgnDb\\");
        dir.AppendUtf8(dbName);
        dbOpenStat = m_QueryDb.OpenBeSQLiteDb (dir.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, dbOpenStat)<<"Db Name: "<<dbName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Adeel.Shoukat     7/2013
//---------------------------------------------------------------------------------------
Utf8String PrimitiveToString(IECSqlValue const& ecsqlValue, ECN::PrimitiveType type)
    {
    Utf8String out;
    if (ecsqlValue.IsNull ())
        {
        out = "NULL";
        return out;
        }

    switch (type)
        {
        case ECN::PRIMITIVETYPE_Binary:
            {
             int blobSize = -1;
                int byteCount=0;
                ecsqlValue.GetBinary (&blobSize);
                Byte storedBlob[]= {0x0c, 0x0b, 0x0c, 0x0b, 0x0c, 0x0b, 65, 66, 67, 68, 0x0c};
                const Byte* data = (const Byte *)ecsqlValue.GetBinary (&blobSize);
                //WString fileName=dir.GetFileNameAndExtension(dir.GetName()); 
               // if (fileName.Equals(L"ecsqlcrudtests.ecdb"))                                 //Binary Comparison is working for ecsqlcrudtest.ecdb only 
               // {
                for(int i=0;i<11;i++)
                    {
                    if(storedBlob[i]==data[i])
                        byteCount++;
                    }
                if(byteCount==11)//<<"Binary Data is not Equal";
                break;
              // }
               else
                {
                 out.Sprintf("BINARY[%d bytes]", blobSize);
                 break;
                }
               /* ECValue ecValue;
                size_t size=   (size_t) blobSize   ;
                ecValue.SetBinary (data, size, false);
                out.Sprintf("%s",(Utf8String)ecValue.ToString());  */
                //out.Sprintf("BINARY[%d bytes]", blobSize);       
            }
        case ECN::PRIMITIVETYPE_Boolean:
            {
            out.Sprintf ("%s", ecsqlValue.GetBoolean () ? "True" : "False");
            break;
            }
        case ECN::PRIMITIVETYPE_DateTime:
            {
            out.Sprintf ("%s", ecsqlValue.GetDateTime ().ToUtf8String ().c_str ());
            break;
            }
        case ECN::PRIMITIVETYPE_Double:
            {
            out.Sprintf ("%f\t", ecsqlValue.GetDouble ());
            break;
            }
        case ECN::PRIMITIVETYPE_Integer:
            {
            out.Sprintf ("%d", ecsqlValue.GetInt ());
            break;
            }
        case ECN::PRIMITIVETYPE_Long:
            {
            out.Sprintf ("%lld", ecsqlValue.GetInt64 ());
            break;
            }
        case ECN::PRIMITIVETYPE_Point2D:
            {
            auto point2d = ecsqlValue.GetPoint2D ();
            out.Sprintf("(%2.1f, %2.1f)", point2d.x, point2d.y);
            break;
            }
        case ECN::PRIMITIVETYPE_Point3D:
            {
            auto point3d = ecsqlValue.GetPoint3D ();
            out.Sprintf("(%2.1f, %2.1f, %2.1f)", point3d.x, point3d.y, point3d.z);
            break;
            }
        case ECN::PRIMITIVETYPE_String:
            {
            out.Sprintf ("%s", ecsqlValue.GetText ());
            break;
            }
        case ECN::PRIMITIVETYPE_IGeometry:
            break;
        }
    return out;
    }
//=======================================================================================    
// @bsimethod                                                Adeel.Shoukat    09/13
//=======================================================================================   
Utf8String PrimitiveToString(IECSqlValue const& ecsqlValue)
    {
    Utf8String out;
    auto primitive = ecsqlValue.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty();
    if (ecsqlValue.IsNull ())
        {
        out = "NULL";
        return out;
        }

        switch (primitive->GetType())
            {
            case ECN::PRIMITIVETYPE_Binary:
                {
                int blobSize = -1;
                int byteCount=0;
                ecsqlValue.GetBinary (&blobSize);
                Byte storedBlob[]= {0x0c, 0x0b, 0x0c, 0x0b, 0x0c, 0x0b, 65, 66, 67, 68, 0x0c};
                const Byte* data = (const Byte *)ecsqlValue.GetBinary (&blobSize);
               // WString fileName=dir.GetFileNameAndExtension(dir.GetName());
                //if (fileName.Equals(L"ecsqlcrudtests.ecdb"))
              //  {
                for(int i=0;i<11;i++)
                    {
                    if(storedBlob[i]==data[i])
                        byteCount++;
                    }
                if(byteCount==11)//<<"Binary Data is not Equal";
                break;
               //}
               else
                {
                 out.Sprintf("BINARY[%d bytes]", blobSize);
                 break;
                }
               /* ECValue ecValue;
                size_t size=   (size_t) blobSize   ;
                ecValue.SetBinary (data, size, false);
                out.Sprintf("%s",(Utf8String)ecValue.ToString());  */
                //out.Sprintf("BINARY[%d bytes]", blobSize);       
                }
            case ECN::PRIMITIVETYPE_Boolean:
                {
                out.Sprintf ("%s", ecsqlValue.GetBoolean () ? "True" : "False");
                break;
                }
            case ECN::PRIMITIVETYPE_DateTime:
                {
                out.Sprintf ("%s", ecsqlValue.GetDateTime ().ToUtf8String ().c_str ());
                break;
                }
            case ECN::PRIMITIVETYPE_Double:
                {
                out.Sprintf ("%f\t", ecsqlValue.GetDouble ());
                break;
                }
            case ECN::PRIMITIVETYPE_Integer:
                {
                out.Sprintf ("%d", ecsqlValue.GetInt ());
                break;
                }
            case ECN::PRIMITIVETYPE_Long:
                {
                out.Sprintf ("%lld", ecsqlValue.GetInt64 ());
                break;
                }
            case ECN::PRIMITIVETYPE_Point2D:
                {
                auto point2d = ecsqlValue.GetPoint2D ();
                out.Sprintf("(%2.1f, %2.1f)", point2d.x, point2d.y);
                break;
                }
            case ECN::PRIMITIVETYPE_Point3D:
                {
                auto point3d = ecsqlValue.GetPoint3D ();
                out.Sprintf("(%2.1f, %2.1f, %2.1f)", point3d.x, point3d.y, point3d.z);
                break;
                }
            case ECN::PRIMITIVETYPE_String:
                {
                out.Sprintf ("%s", ecsqlValue.GetText ());
                break;
                }
            case ECN::PRIMITIVETYPE_IGeometry:
                break;
            }
    return out;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Adeel.Shoukat     7/2013
//---------------------------------------------------------------------------------------
Utf8String ArrayToString(IECSqlArrayValue const& array, ECN::ECPropertyCP property)
    {
    Utf8String out = "(";
    bool bFirst = true;
    for (IECSqlValue const* arrayElement : array)
        {
        if (!bFirst)
            out.append(", ");

        auto arrayProperty = property->GetAsArrayProperty();
        if (arrayProperty->GetKind() == ECN::ArrayKind::ARRAYKIND_Primitive)
            out.append (PrimitiveToString (*arrayElement, arrayProperty->GetPrimitiveElementType ()));
        else
            out.append (StructToString (arrayElement->GetStruct ()) + ", ");

        bFirst = false;  
        }
    out.append(")");   
    return out;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Adeel.Shoukat     7/2013
//---------------------------------------------------------------------------------------
Utf8String StructToString(IECSqlStructValue const& ecsqlStructValue)
    {
    Utf8String out;
    for (int i = 0; i < ecsqlStructValue.GetMemberCount (); i++)
        {
        IECSqlValue const& structMemberValue = ecsqlStructValue.GetValue (i);
        auto property = structMemberValue.GetColumnInfo ().GetProperty ()->GetAsPrimitiveProperty ();
        if (property->GetIsPrimitive())
            {
            out.append (PrimitiveToString (structMemberValue));
            }
        else if (property->GetIsStruct())
            {
            out.append (StructToString (structMemberValue.GetStruct ()));
            }
        else
            {
            out.append (ArrayToString (structMemberValue.GetArray (), property));
            }                                                                                                                  
        out.append(",");
        }
    return out;
    }
//=======================================================================================    
// @bsimethod                                              Adeel.Shoukat    09/13
//=======================================================================================   
void verifyResults(Utf8CP expectedResult,Utf8CP InputQuery,int expectedRowCount,Utf8CP expectedLastError,/*int ExpectedPrepareStatus*/int comparisonPassFail)
    {
    Utf8String out="";
    Utf8String Result(expectedResult);
    ECSqlStatement ecsqlStatement;
    int rowCount=0;
           
    if(comparisonPassFail)
        {
            auto ecsqlPrepareStat = ecsqlStatement.Prepare (m_QueryDb,InputQuery);
        if(ecsqlPrepareStat==ECSqlStatus::Success)
            {
            ASSERT_TRUE (ecsqlStatement.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to Prepare is expected to return empty string.";

            while(ecsqlStatement.Step()==ECSqlStepStatus::HasRow)
            {
            ASSERT_TRUE (ecsqlStatement.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to Step is expected to return empty string.";
            for(int i=0; i < ecsqlStatement.GetColumnCount(); i++)
                {
                IECSqlValue const& ecsqlValue = ecsqlStatement.GetValue (i);
                if (ecsqlValue.GetColumnInfo ().GetProperty ()->GetIsPrimitive ())
                    out += PrimitiveToString (ecsqlValue) + "\t";
                else if (ecsqlValue.GetColumnInfo ().GetProperty ()->GetIsStruct ())
                    out += StructToString (ecsqlValue.GetStruct ()) + "\t";
                else if (ecsqlValue.GetColumnInfo ().GetProperty ()->GetIsArray ())
                    out += ArrayToString (ecsqlValue.GetArray (), ecsqlValue.GetColumnInfo ().GetProperty ()) + "\t";
                }
            rowCount++;                                               
                }
            EXPECT_EQ(expectedRowCount,rowCount)<<"Row count in expected and Result not equal";
            EXPECT_STREQ(Result.c_str(),out.c_str())<<"Expected result and Query result not equal";
            ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to Step is expected to return empty string.";
            }
        ecsqlPrepareStat = ecsqlStatement.ClearBindings ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (ecsqlPrepareStat)) << "ECSqlStatement::Reset failed unexpectedly.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to ClearBindings is expected to return empty string.";

        ecsqlPrepareStat = ecsqlStatement.Reset ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (ecsqlPrepareStat)) << "ECSqlStatement::Reset failed unexpectedly.";
        ASSERT_TRUE (ecsqlStatement.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to Reset is expected to return empty string.";

        ecsqlStatement.Finalize ();
        ASSERT_TRUE (ecsqlStatement.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after call to Finalize is expected to return empty string.";
        } 
    else
        {    
        ECSqlStatement stmt;
        auto stat = stmt.Prepare (m_QueryDb,InputQuery);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::InvalidECSql), static_cast<int> (stat)) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

        auto actualLastStatusMessage = stmt.GetLastStatusMessage ();
        ASSERT_STREQ (actualLastStatusMessage.c_str (), expectedLastError);

        stmt.Finalize ();
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after call to Finalize is expected to return empty string.";
        }
    }
};
//=======================================================================================                                                                                                                        k
// @bsiMthod                                                 Adeel.Shoukat    09/13
//=======================================================================================   
TEST_F(ECSqlQueryAdapter,VerifyECSqlQueries)
    {
    WString DbName=L"ECSqlTestDb.db";
    openTestDb(DbName);
    Utf8CP InputDbName,InputQuery,expectedResultString,expectedLastError;
    Statement TestStatement;
    int expectedRowCount=0;
    int comparisonPassFail;
    DbResult statementResult = TestStatement.Prepare (m_TestResultDb, "SELECT * FROM ECSQlQueryTest");
    if(statementResult==DbResult::BE_SQLITE_OK)
        {
        while(TestStatement.Step()==DbResult::BE_SQLITE_ROW)
            {
            InputQuery = TestStatement.GetValueText(0);
            InputDbName = TestStatement.GetValueText(1);
            expectedResultString = TestStatement.GetValueText(2);
            expectedRowCount = TestStatement.GetValueInt(3);
            comparisonPassFail = TestStatement.GetValueInt(4);
            expectedLastError=TestStatement.GetValueText(5);
            OpenInputDb(InputDbName);
            verifyResults(expectedResultString,InputQuery,expectedRowCount,expectedLastError,/*expectedPrepareStatus*/comparisonPassFail);
            }
        } 
    }

END_ECDBUNITTESTS_NAMESPACE
