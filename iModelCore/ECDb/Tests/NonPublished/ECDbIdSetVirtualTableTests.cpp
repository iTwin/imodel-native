/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbIdSetVirtualTableTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbIdSetVirtualTableTestFixture, IdSetModuleTest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("vtab.ecdb"));
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]')"));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]') where id = 1"));

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 1);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,2,3,4,5]') where id > 1"));

        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0x1\",\"0x2\",3,4,5]')"));
    
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[0x1,0x2,\"3\",\"4\",\"5\"]')"));

        // Should fail while converting to json array because hex values with quotes are required so should be empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,\"2\",3, 4.0, 5.0]')"));
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,\"2\",3, 4.5, 5.6]')"));
        
        // Will not take into account 4.5 and 5.6 because they are decimal values so should be empty table and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        ASSERT_EQ(ECSqlStatus::Error,stmt.BindText(1, "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

        // no binding as we use array ecsql binder so need to call AddArrayElement first
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));

        // no binding so no data
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText( "[1,\"2\",3, \"abc\"]", IECSqlBinder::MakeCopy::No));

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindInt(i));
        }

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 10);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
        }

        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i), stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 10);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
        for(int i = 1;i<=10;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(i));
        }

        // having null as an element so should be empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
        DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(pArrayOfST1_P2D[i]));
        }
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(pArrayOfST1_P3D[i]));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
        DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
        double pST1P_ST2P_D2 = 431231.3432;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[i]));
        }
        for(int i = 0;i<=2;i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["D1"].BindDouble(pST1P_ST2P_D2));
            ASSERT_EQ(ECSqlStatus::Error, elementBinder["P3D"].BindPoint3d(pArrayOfST1_P3D[i]));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        const std::vector<std::vector<uint8_t>> bi_array = {
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6}
        };
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(auto& m : bi_array)
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));

        // Binary is Binded so should be empty table and error should be thrown
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        const auto dt = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
        const auto dtUtc = DateTime(DateTime::Kind::Utc, 2018, 2, 17, 0, 0);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dt));
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dtUtc));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        auto geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

        // Binary is Binded because BindGeometry internally calls so should be empty table and error should be thrown
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("ABC",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[abc]",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i = 0;i<=1;i++)
        {
            ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("[\"abc\"]",IECSqlBinder::MakeCopy::No));
        }

        // EmptyArray is Binded so should be empty table and should log error because the ultimate json text which will be binded will be an empty json array and that is not allowed
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT x FROM ECVLib.IdSet('[1,2,3,4,5]'), (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte) where id = x group by x"));
        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 6);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT x FROM  (with cte(x) as(select ECInstanceId from meta.ECClassDef) select x from cte), ECVLib.IdSet('[1,2,3,4,5]') where id = x group by x"));
        int i = 1;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 6);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet('[1,2,3,4,5]'), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN IdSet VIRTUAL TABLE INDEX 1"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SEARCH main.ec_Class USING INTEGER PRIMARY KEY (rowid=?)"));
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId FROM ECVLib.IdSet('[1,2,3,4,5]'), meta.ECClassDef where ECClassId = id group by ECClassId"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN IdSet VIRTUAL TABLE INDEX 1"));
        ASSERT_EQ(true, m_ecdb.ExplainQuery(stmt.GetNativeSql(), true).Contains("SCAN main.ec_Class USING COVERING INDEX ix_ec_Class_Name"));
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Reset());
        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        IECSqlBinder& arrayBinder_two = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder_two.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, hexIds.size());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECClassId FROM ECVLib.IdSet(?,?), meta.ECClassDef where ECClassId = id group by ECClassId"));
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[1,1,1,1]')"));
        
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ(i+1, stmt.GetValueInt64(0));
            i++;
        }
        ASSERT_EQ(i, 1);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[-1,-2,3,-4,5]')"));
        // negative values are not allowed so empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<int> ids = std::vector<int>{0,1,1,2};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& binder = stmt.GetBinder(1);

        for(auto& i : ids)
        {
            ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindInt(i));
        }
        // 0 is not allowed so empty table and dhould log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"-1\",\"-2\",\"3\",\"-4\",\"5\"]')"));
        // negative values are not allowed so empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<Utf8String> stringIds = std::vector<Utf8String>{"-1", "-2", "3", "-4", "5"};
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet(?)"));
        IECSqlBinder& binder = stmt.GetBinder(1);

        for(auto& i : stringIds)
        {
            if(i.EqualsIAscii("3") || i.EqualsIAscii("5"))
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindText(i.c_str(), IECSqlBinder::MakeCopy::No));
            else
                ASSERT_EQ(ECSqlStatus::Error, binder.AddArrayElement().BindText(i.c_str(), IECSqlBinder::MakeCopy::No));
        }
        // Binding negative values will fail so for the negative values binder.AddArrayElement().BindText() will be empty array element which are not allowed so empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0xFFFFFFFF\",3,4,5]')"));
        int i = 3;
        while (i<=5)
        {
            ASSERT_EQ(BE_SQLITE_ROW,  stmt.Step());
            ASSERT_EQ(i, stmt.GetValueInt64(0));
            i++;
        }
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT id FROM ECVLib.IdSet('[\"0x0\",3,4,5]')"));
        // 0 values are not allowed so empty table and should log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<Utf8String> hexIds = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4", "5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<hexIds.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(hexIds[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        int i = 0;
        while (i<=2)
        {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));
        }

        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(BeStringUtilities::ParseHex(hexIds[i++].c_str()), stmt.GetValueInt64(0));

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
        std::vector<Utf8String> ids = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4.5", "5.5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            if(i == 3 || i == 4)
                ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText(ids[i].c_str(), IECSqlBinder::MakeCopy::No));
            else
                ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(ids[i].c_str(), IECSqlBinder::MakeCopy::No));
        }

        // "4.5" and "5.5" are not allowed to bind so empty array element so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());

        ECSqlStatus stat = stmt.Reset();
        ASSERT_EQ(stat.IsSQLiteError(), true ); // As we are not stepping successfully in the statement so reset fails from sqlite side
        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        std::vector<double> dec_ids = std::vector<double>{1, 2, 4.5, 3, 5.5};
        IECSqlBinder& arrayBinder_two = stmt.GetBinder(1);
        for(int i =0;i<dec_ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder_two.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(dec_ids[i]));
        }
        
        // 4.5 and 5.5 are not allowed so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
    }
    {
        std::vector<Utf8String> ids = std::vector<Utf8String>{"0x1", "0x2", "0x3", "4.5", "5.5"};

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            if(i == 3 || i == 4)
                ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText(ids[i].c_str(), IECSqlBinder::MakeCopy::No));
            else
                ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(ids[i].c_str(), IECSqlBinder::MakeCopy::No));
        }

        // "4.5" and "5.5" are not allowed to bind so empty array element so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());

        ECSqlStatus stat = stmt.Reset();
        ASSERT_EQ(stat.IsSQLiteError(), true ); // As we are not stepping successfully in the statement so reset fails from sqlite side
        ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());

        std::vector<double> dec_ids = std::vector<double>{1, 2, 4.0, 3, 5.0};
        IECSqlBinder& arrayBinder_two = stmt.GetBinder(1);
        for(int i =0;i<dec_ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder_two.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(dec_ids[i]));
        }
        
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));

        std::vector<double> dec_ids = std::vector<double>{1, 2, 4.0, 3, 5.0};
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<dec_ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(dec_ids[i]));
        }
        
        int i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);

        ASSERT_EQ(ECSqlStatus::Success, stmt.Reset());

        i = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((1+i++), stmt.GetValueInt64(0));
        }
        ASSERT_EQ(i, 5);        
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));

        std::vector<double> dec_ids = std::vector<double>{0,1,2,3.0};
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<dec_ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(dec_ids[i]));
        }
        
        // 0 is not allowed in IdSet VT so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());      
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet(?), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));

        std::vector<Utf8String> dec_ids = std::vector<Utf8String>{"0","1","2","3.0"};
        IECSqlBinder& arrayBinder = stmt.GetBinder(1);
        for(int i =0;i<dec_ids.size();i++)
        {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            if(i == dec_ids.size() - 1)
                ASSERT_EQ(ECSqlStatus::Error, elementBinder.BindText(dec_ids[i].c_str(), IECSqlBinder::MakeCopy::No));
            else
                ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(dec_ids[i].c_str(), IECSqlBinder::MakeCopy::No));
        }
        
        // "0" are not allowed to bind so empty array element so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());      
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet('[\"0\",\"1\",\"2\",\"3.0\"]'), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        
        // "0" is not allowed in IdSet VT so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());      
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECVLib.IdSet('[\"1\",\"2\",\"3.0\"]'), meta.ECClassDef where ECInstanceId = id group by ECInstanceId"));
        
        // "3.0" is not allowed in IdSet VT so should fail and log error
        ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());      
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbIdSetVirtualTableTestFixture, IdSetWithJOINS) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IdSet_with_JOINS.ecdb", SchemaItem(R"xml(<?xml version='1.0' encoding='utf-8'?>
    <ECSchema schemaName='TestSchema' alias='ts' version='10.10.10' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        <ECEntityClass typeName='A' >
            <ECProperty propertyName="str_prop" typeName="string" />
            <ECProperty propertyName="int_prop" typeName="int" />
        </ECEntityClass>
        </ECSchema>)xml")));
    std::vector<BeInt64Id> listOfIds;
    std::vector<Utf8CP> listOfStringVal = {"str1", "str2", "str3", "str4","str5", "str6", "str7", "str8", "str9", "str10"};
    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.A(str_prop, int_prop) VALUES(?,?)"));
    for(int i =1;i<=10;i++)
    {
        insertStmt.BindText(1, listOfStringVal[i-1], IECSqlBinder::MakeCopy::No);
        insertStmt.BindInt(2, i);
        ECInstanceKey key;
        if(insertStmt.Step(key) != BE_SQLITE_DONE)
            break;
        listOfIds.push_back((BeInt64Id)key.GetInstanceId());
        insertStmt.ClearBindings();
        insertStmt.Reset();
    }
    ASSERT_EQ(10, listOfIds.size());
    if("testing normal joins with IdSet")
    {
        ECSqlStatement selectStmt;
        ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "Select test.str_prop, test.int_prop, v.id from ts.A test JOIN ECVLib.IdSet(?) v on test.ECInstanceId = v.id"));
        IECSqlBinder& binder = selectStmt.GetBinder(1);
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[3]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[6]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[7]));
        ASSERT_STREQ("str_prop", selectStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("int_prop", selectStmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("id", selectStmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str4", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(4, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[3], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str7", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(7, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[6], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str8", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(8, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[7], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_DONE, selectStmt.Step());

        ASSERT_STREQ("SELECT [test].[str_prop],[test].[int_prop],v.id FROM (SELECT [Id] ECInstanceId,88 ECClassId,[str_prop],[int_prop] FROM [main].[ts_A]) [test] INNER JOIN IdSet(:_ecdb_sqlparam_ix1_col1) v ON [test].[ECInstanceId]=v.id ", selectStmt.GetNativeSql());
    }
    if("testing inner join with IdSet")
    {
        ECSqlStatement selectStmt;
        ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "Select test.str_prop, test.int_prop, v.id from ts.A test INNER JOIN ECVLib.IdSet(?) v on test.ECInstanceId = v.id"));
        IECSqlBinder& binder = selectStmt.GetBinder(1);
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[3]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[6]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[7]));
        ASSERT_STREQ("str_prop", selectStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("int_prop", selectStmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("id", selectStmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str4", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(4, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[3], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str7", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(7, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[6], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str8", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(8, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[7], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_DONE, selectStmt.Step());
        ASSERT_STREQ("SELECT [test].[str_prop],[test].[int_prop],v.id FROM (SELECT [Id] ECInstanceId,88 ECClassId,[str_prop],[int_prop] FROM [main].[ts_A]) [test] INNER JOIN IdSet(:_ecdb_sqlparam_ix1_col1) v ON [test].[ECInstanceId]=v.id ", selectStmt.GetNativeSql());
    }
    if("testing right outer join with IdSet")
    {
        ECSqlStatement selectStmt;
        ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "Select test.str_prop, test.int_prop, v.id from ts.A test RIGHT OUTER JOIN ECVLib.IdSet(?) v on test.ECInstanceId = v.id"));
        IECSqlBinder& binder = selectStmt.GetBinder(1);
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[3]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[6]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[7]));
        ASSERT_STREQ("str_prop", selectStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("int_prop", selectStmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("id", selectStmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str4", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(4, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[3], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str7", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(7, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[6], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str8", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(8, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[7], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_DONE, selectStmt.Step());
        ASSERT_STREQ("SELECT [test].[str_prop],[test].[int_prop],v.id FROM (SELECT [Id] ECInstanceId,88 ECClassId,[str_prop],[int_prop] FROM [main].[ts_A]) [test] RIGHT OUTER JOIN IdSet(:_ecdb_sqlparam_ix1_col1) v ON [test].[ECInstanceId]=v.id ", selectStmt.GetNativeSql());
    }
    if("testing left outer join with IdSet")
    {
        ECSqlStatement selectStmt;
        ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "Select test.str_prop, test.int_prop, v.id from ts.A test LEFT OUTER JOIN ECVLib.IdSet(?) v on test.ECInstanceId = v.id"));
        IECSqlBinder& binder = selectStmt.GetBinder(1);
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[3]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[6]));
        ASSERT_EQ(ECSqlStatus::Success,binder.AddArrayElement().BindId(listOfIds[7]));
        ASSERT_STREQ("str_prop", selectStmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("int_prop", selectStmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        ASSERT_STREQ("id", selectStmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str1", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(1, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str2", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(2, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str3", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(3, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str4", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(4, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[3], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str5", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(5, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str6", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(6, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str7", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(7, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[6], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str8", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(8, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(listOfIds[7], selectStmt.GetValueId<BeInt64Id>(2)) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str9", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(9, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
        ASSERT_STREQ("str10", selectStmt.GetValueText(0)) << "str_prop";
        ASSERT_EQ(10, selectStmt.GetValueInt(1)) << "int_prop";
        ASSERT_EQ(selectStmt.IsValueNull(2), true) << "id";

        ASSERT_EQ(BE_SQLITE_DONE, selectStmt.Step());
        ASSERT_STREQ("SELECT [test].[str_prop],[test].[int_prop],v.id FROM (SELECT [Id] ECInstanceId,88 ECClassId,[str_prop],[int_prop] FROM [main].[ts_A]) [test] LEFT OUTER JOIN IdSet(:_ecdb_sqlparam_ix1_col1) v ON [test].[ECInstanceId]=v.id ", selectStmt.GetNativeSql());
    }
    if("testing left outer join with IdSet")
    {
        ECSqlStatement selectStmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, selectStmt.Prepare(m_ecdb, "Select test.str_prop, test.int_prop, v.id from ts.A test OUTER JOIN ECVLib.IdSet(?) v on test.ECInstanceId = v.id"));
    }
}


END_ECDBUNITTESTS_NAMESPACE
